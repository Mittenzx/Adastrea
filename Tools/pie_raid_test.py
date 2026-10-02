"""PIE test for pirate needs and raids (run inside the editor while PIE is up in CombatTest).

    py "<project>/Tools/pie_raid_test.py"

Uses the Cinder Wolves gang from Organisations.json, with supplies set short. Stages:

  1. Scoring: a trader carrying fuel scores above one carrying artwork; security and patrols
     lower the score; desperation raises it again.
  2. AI trader complies (adastrea.RaidAIComply 1): raiders hail it, it stops, drones take
     part of its fuel, the gang's supplies rise, and the raiders leave without a shot.
  3. AI trader refuses (adastrea.RaidAIComply 0): raiders disable it (a wreck, never
     destroyed), drones strip its hold, and the gang's supplies and parts rise.
  4. The player complies: the comms panel is up (IsPlayerHailed), RaidComply, drones take
     the cargo from the player's hold.
  5. The player refuses: RaidRefuse, the raiders attack; when their shields fail they
     break off and leave the level without being disabled.

Progress is logged as "RAID ..." lines ending with "RAID SUMMARY". The raid subsystem's own
lines ("Raid 3: ...", "Pirates: ...") are in the same log.
"""
import unreal

GANG = "cinder_wolves"
FUEL = "Helium-3"
ART = "Artwork"
STATION_OFFSET = 600000.0  # cm: 6 km from the station, so turrets stay out of it and traders take minutes to reach a dock
STAGE_TIMEOUT = 200.0  # game seconds
results = []


def log(msg):
    unreal.log("RAID " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def cmd(c):
    unreal.SystemLibrary.execute_console_command(world(), c, unreal.GameplayStatics.get_player_controller(world(), 0))


def check(name, ok, note=""):
    results.append((name, ok))
    log("%s %s %s" % ("PASS" if ok else "FAIL", name, note))


def live_subsystem(cls):
    # This build's Python has no SubsystemBlueprintLibrary: find the live instance (not the CDO) instead.
    for obj in unreal.ObjectIterator(cls):
        if not obj.get_name().startswith("Default__") and obj.get_world() == world():
            return obj
    return None


def raids():
    return live_subsystem(unreal.RaidSubsystem)


def pirates():
    return live_subsystem(unreal.PirateSubsystem)


def need(n):
    return pirates().get_need_level(GANG, n)


def player():
    return unreal.GameplayStatics.get_player_pawn(world(), 0)


def item(name):
    return unreal.load_object(None, "/Game/DataAssets/Trading/Items/DA_TradeItem_%s.DA_TradeItem_%s" % (name, name))


def qty(ship, name):
    return ship.get_editor_property("cargo_component").get_item_quantity(item(name))


def raider_ships():
    return [p.get_controlled_pawn() for p in unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.RaiderController) if p.get_controlled_pawn()]


def ships_tagged_raider():
    return [s for s in unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.Spaceship) if s.actor_has_tag("Raider")]


def drones():
    return list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.LootDrone))


def spawn_trader(item_name, units, dist_m):
    before = set(s.get_name() for s in unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.Spaceship))
    cmd("adastrea.RaidTrader %s %d %d" % (item_name, units, dist_m))
    new = [s for s in unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.Spaceship) if s.get_name() not in before]
    return new[0] if new else None


def phase():
    # The newest raid (each stage clears the previous ones first).
    lines = list(raids().describe_raids())
    return lines[-1].split(" ")[2] if lines else "none"


class Runner:
    def __init__(self):
        self.t = 0.0
        self.mark = 0.0
        self.next_report = 0.0
        self.stage = "setup"
        self.phases = set()
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def go(self, stage):
        self.stage, self.mark, self.next_report = stage, self.t, 0.0
        self.phases = set()
        log("stage %s" % stage)

    def since(self):
        return self.t - self.mark

    def finish(self):
        unreal.unregister_slate_post_tick_callback(self.h)
        for c in ("adastrea.ClearRaids", "adastrea.RaidAIComply -1", "adastrea.Security -1", "adastrea.PirateUpkeepScale 1"):
            cmd(c)
        passed = sum(1 for _, ok in results if ok)
        log("SUMMARY %d/%d passed" % (passed, len(results)))

    def watch(self, label, target=None):
        self.phases.add(phase())
        if self.since() >= self.next_report:
            self.next_report += 5.0
            parts = list(raids().describe_raids())
            if target:
                hc = target.get_editor_property("health_component")
                parts.append("target %s sh%.0f hull%.0f v%.0f wreck=%s" % (target.get_name(), hc.get_shield(),
                    target.get_editor_property("current_hull_integrity"), target.get_velocity().length(), target.is_wrecked()))
            parts.append("drones=%d" % len(drones()))
            log("%s t=%.0f %s" % (label, self.since(), " | ".join(parts)))

    def tick(self, dt):
        try:
            if world() is None:
                log("PIE not running")
                self.finish()
                return
            self.t = unreal.GameplayStatics.get_time_seconds(world())
            getattr(self, "stage_" + self.stage)()
        except Exception as e:  # never leave a throwing callback registered
            log("ERROR %s" % e)
            self.finish()

    # ---- setup ----
    def stage_setup(self):
        p = player()
        stations = list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.SpaceStation))
        if not isinstance(p, unreal.Spaceship) or p.is_wrecked() or not stations or raids() is None or pirates() is None:
            check("setup", False, "need a working player ship, a station and the raid/pirate subsystems")
            self.finish()
            return
        for c in ("HostileWaves 0", "ClearHostiles", "ClearWrecks", "adastrea.ClearRaids", "adastrea.Security 0",
                  "adastrea.PirateUpkeepScale 0", "adastrea.PirateNeed %s supplies 10" % GANG, "adastrea.PirateNeed %s parts 40" % GANG,
                  "adastrea.PirateNeed %s cash 60" % GANG, "adastrea.PirateDesperation %s 0" % GANG):
            cmd(c)
        station = stations[0]
        away = p.get_actor_location() - station.get_actor_location()
        away = away.normal() if away.length() > 1 else unreal.Vector(1, 0, 0)
        p.set_actor_location(station.get_actor_location() + away * STATION_OFFSET, False, True)
        self.go("scoring")

    # ---- 1. scoring ----
    def stage_scoring(self):
        if self.since() < 2.0:
            return
        fuel = spawn_trader(FUEL, 10, 600)
        art = spawn_trader(ART, 10, -600)
        if not fuel or not art:
            check("scoring traders spawned", False)
            self.go("comply_ai_start")
            return
        r = raids()
        f0, a0 = r.get_target_score(GANG, fuel), r.get_target_score(GANG, art)
        check("short of supplies: fuel trader scores above art trader", f0 > a0, "fuel %.1f art %.1f" % (f0, a0))
        cmd("adastrea.Security 3")
        f_high = r.get_target_score(GANG, fuel)
        check("high security lowers the score", f_high < f0, "%.1f -> %.1f" % (f0, f_high))
        cmd("adastrea.PirateDesperation %s 1" % GANG)
        f_desp = r.get_target_score(GANG, fuel)
        check("desperation raises it again", f_desp > f_high, "%.1f -> %.1f" % (f_high, f_desp))
        for c in ("adastrea.PirateDesperation %s 0" % GANG, "adastrea.Security 0"):
            cmd(c)
        for s in (fuel, art):
            s.get_controller().destroy_actor() if s.get_controller() else None
            s.destroy_actor()
        self.go("comply_ai_start")

    # ---- 2. AI trader complies ----
    def stage_comply_ai_start(self):
        if self.since() < 1.0:
            return
        cmd("adastrea.ClearRaids")
        cmd("ClearWrecks")
        cmd("adastrea.RaidAIComply 1")
        self.trader = spawn_trader(FUEL, 10, 500)
        self.fuel0 = qty(self.trader, FUEL)
        self.supplies0 = need(unreal.PirateNeed.SUPPLIES)
        self.go("comply_ai_launch")

    def stage_comply_ai_launch(self):
        # Give the trader a moment to pick its destination: the raiders lie in wait ahead of it.
        if self.since() < 3.0:
            return
        rid = raids().start_raid_on(GANG, self.trader, 120000.0)
        log("comply_ai: trader %s with %d fuel, supplies %.1f, raid %d" % (self.trader.get_name(), self.fuel0, self.supplies0, rid))
        check("raid started on the AI trader", rid >= 0)
        self.min_speed_collect = 1e9
        self.go("comply_ai")

    def stage_comply_ai(self):
        self.watch("comply_ai", self.trader)
        if phase() == "collecting":
            self.min_speed_collect = min(self.min_speed_collect, self.trader.get_velocity().length())
        if raids().get_raid_count() == 0 or self.since() > STAGE_TIMEOUT:
            hc = self.trader.get_editor_property("health_component")
            check("AI: raiders hailed, then collected", {"hailing", "collecting"} <= self.phases, str(sorted(self.phases)))
            check("AI: never attacked after complying", "attacking" not in self.phases and not self.trader.is_wrecked()
                  and hc.get_shield() >= hc.get_max_shield() - 1, "shield %.0f/%.0f" % (hc.get_shield(), hc.get_max_shield()))
            check("AI: trader held still while drones worked", self.min_speed_collect < 300.0, "min speed %.0f cm/s" % self.min_speed_collect)
            fuel1 = qty(self.trader, FUEL)
            check("AI: drones took part of the fuel", 0 < fuel1 < self.fuel0, "%d -> %d" % (self.fuel0, fuel1))
            s1 = need(unreal.PirateNeed.SUPPLIES)
            check("AI: gang's supplies rose", s1 > self.supplies0, "%.1f -> %.1f" % (self.supplies0, s1))
            check("AI: raid over, raiders gone", raids().get_raid_count() == 0 and not raider_ships(), "after %.0f s" % self.since())
            self.go("refuse_ai_start")

    # ---- 3. AI trader refuses ----
    def stage_refuse_ai_start(self):
        if self.since() < 1.0:
            return
        cmd("adastrea.ClearRaids")
        cmd("ClearWrecks")
        cmd("adastrea.RaidAIComply 0")
        cmd("adastrea.PirateNeed %s supplies 10" % GANG)
        self.trader = spawn_trader(FUEL, 10, 500)
        self.fuel0 = qty(self.trader, FUEL)
        self.supplies0 = need(unreal.PirateNeed.SUPPLIES)
        self.parts0 = need(unreal.PirateNeed.PARTS)
        self.go("refuse_ai_launch")

    def stage_refuse_ai_launch(self):
        if self.since() < 3.0:
            return
        rid = raids().start_raid_on(GANG, self.trader, 120000.0)
        check("raid started on the refusing trader", rid >= 0)
        self.go("refuse_ai")

    def stage_refuse_ai(self):
        self.watch("refuse_ai", self.trader)
        if raids().get_raid_count() == 0 or self.since() > STAGE_TIMEOUT:
            check("AI refuse: hail, attack, strip", {"hailing", "attacking", "stripping"} <= self.phases, str(sorted(self.phases)))
            alive = unreal.SystemLibrary.is_valid(self.trader)
            check("AI refuse: trader disabled into a wreck, not destroyed", alive and self.trader.is_wrecked())
            fuel1 = qty(self.trader, FUEL) if alive else -1
            check("AI refuse: hold stripped", fuel1 == 0, "%d -> %d" % (self.fuel0, fuel1))
            s1, p1 = need(unreal.PirateNeed.SUPPLIES), need(unreal.PirateNeed.PARTS)
            check("AI refuse: supplies and salvage reached the gang", s1 > self.supplies0 and p1 > self.parts0,
                  "supplies %.1f -> %.1f, parts %.1f -> %.1f" % (self.supplies0, s1, self.parts0, p1))
            check("AI refuse: raid over", raids().get_raid_count() == 0, "after %.0f s" % self.since())
            cmd("ClearWrecks")
            self.go("player_comply_start")

    # ---- 4. player complies ----
    def stage_player_comply_start(self):
        if self.since() < 2.0:
            return
        cmd("adastrea.ClearRaids")
        cmd("ClearWrecks")
        p = player()
        p.get_editor_property("health_component").restore()
        cargo = p.get_editor_property("cargo_component")
        cargo.add_cargo(item(FUEL), 10)
        self.fuel0 = qty(p, FUEL)
        cmd("adastrea.PirateNeed %s supplies 10" % GANG)
        rid = raids().start_raid_on(GANG, p, 120000.0)
        check("raid started on the player", rid >= 0, "player fuel %d" % self.fuel0)
        self.answered = False
        self.go("player_comply")

    def stage_player_comply(self):
        p = player()
        self.watch("player_comply", p)
        if not self.answered and raids().is_player_hailed() and phase() == "hailing" and self.since() > 2.0:
            check("player: comms panel up (hailed)", True)
            cmd("RaidComply")
            self.answered = True
        if raids().get_raid_count() == 0 or self.since() > STAGE_TIMEOUT:
            check("player: hail answered", self.answered)
            check("player: comply -> collect, no attack", "collecting" in self.phases and "attacking" not in self.phases, str(sorted(self.phases)))
            fuel1 = qty(p, FUEL)
            check("player: drones took fuel from the player's hold", fuel1 < self.fuel0, "%d -> %d" % (self.fuel0, fuel1))
            self.go("player_refuse_start")

    # ---- 5. player refuses; raiders break off when their shields fail ----
    def stage_player_refuse_start(self):
        if self.since() < 2.0:
            return
        cmd("adastrea.ClearRaids")
        cmd("ClearWrecks")
        p = player()
        p.get_editor_property("health_component").restore()
        p.get_editor_property("cargo_component").add_cargo(item(FUEL), 10)
        cmd("adastrea.PirateNeed %s supplies 10" % GANG)
        self.lost0 = None
        rid = raids().start_raid_on(GANG, p, 120000.0)
        check("second raid started on the player", rid >= 0)
        self.answered = False
        self.hit = False
        self.go("player_refuse")

    def stage_player_refuse(self):
        p = player()
        self.watch("player_refuse", p)
        if not self.answered and raids().is_player_hailed() and phase() == "hailing":
            cmd("RaidRefuse")
            self.answered = True
            self.refused_at = self.since()
        if self.answered and not self.hit and phase() == "attacking" and self.since() - self.refused_at > 3.0:
            # Knock each raider's shields out (the player's guns, in effect).
            for s in raider_ships():
                hc = s.get_editor_property("health_component")
                hc.apply_damage(hc.get_shield() + 1.0, p)
            self.hit = True
            self.hit_at = self.since()
            self.raiders_at_hit = len(raider_ships())
        if self.hit and (raids().get_raid_count() == 0 or self.since() > STAGE_TIMEOUT):
            check("player refuse: attack after refusal", "attacking" in self.phases, str(sorted(self.phases)))
            check("player refuse: raiders broke off when shields failed", "retreating" in self.phases)
            wrecks = [s for s in ships_tagged_raider() if s.is_wrecked()]
            check("player refuse: raiders left the level, none disabled", not raider_ships() and not wrecks,
                  "%d raiders hit, %d wrecks, raid over after %.0f s" % (self.raiders_at_hit, len(wrecks), self.since() - self.hit_at))
            p.get_editor_property("health_component").restore()
            self.finish()
        elif not self.hit and self.since() > STAGE_TIMEOUT:
            check("player refuse: reached the attack", False, str(sorted(self.phases)))
            self.finish()


Runner()
