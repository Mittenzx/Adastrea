"""PIE test for sector security and distress calls (run inside the editor while PIE is up in CombatTest).

    py "<project>/Tools/pie_distress_test.py"

Moves the player 1 km from the CombatTest station (out of the patrols' sensor range), spawns
two patrols at the station, then:

  1. High security (adastrea.Security 3): two hostiles attack the player. The patrols should
     answer the distress call at once, fly out, and disable the hostiles; the call then closes.
  2. No security (adastrea.Security 0): one hostile attacks. Nobody should come.

Progress is logged as "DSTT ..." lines ending with "DSTT SUMMARY". The distress subsystem's own
lines ("Distress call", "answering distress call", "closed") are in the same log.
"""
import unreal

STATION_OFFSET = 100000.0  # cm: put the player 1 km out from the station (patrol sensors reach 250 m)
HIGH_TIMEOUT = 180.0  # game seconds
NONE_WATCH = 25.0
results = []


def log(msg):
    unreal.log("DSTT " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def cmd(c):
    unreal.SystemLibrary.execute_console_command(world(), c, unreal.GameplayStatics.get_player_controller(world(), 0))


def check(name, ok, note=""):
    results.append((name, ok))
    log("%s %s %s" % ("PASS" if ok else "FAIL", name, note))


def player():
    return unreal.GameplayStatics.get_player_pawn(world(), 0)


def patrol_ships():
    return [p.get_controlled_pawn() for p in unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.PatrolController) if p.get_controlled_pawn()]


def hostile_ships():
    return [p.get_controlled_pawn() for p in unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.HostileFighterController) if p.get_controlled_pawn()]


def all_ships():
    return list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.Spaceship))


def dist_m(a, b):
    return a.get_actor_location().distance(b.get_actor_location()) / 100.0


class Runner:
    def __init__(self):
        self.t = 0.0
        self.mark = 0.0
        self.next_report = 0.0
        self.stage = "setup"
        self.station = None
        self.hostiles = []
        self.closest_patrol_m = 1e9
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def go(self, stage):
        self.stage, self.mark = stage, self.t

    def since(self):
        return self.t - self.mark

    def finish(self):
        unreal.unregister_slate_post_tick_callback(self.h)
        cmd("adastrea.Security -1")
        passed = sum(1 for _, ok in results if ok)
        log("SUMMARY %d/%d passed" % (passed, len(results)))

    def report(self, label):
        p = player()
        parts = []
        for s in patrol_ships():
            parts.append("patrol %.0fm" % dist_m(s, p))
        for s in self.hostiles:
            if s.is_wrecked():
                parts.append("hostile WRECK")
            else:
                hc = s.get_editor_property("health_component")
                parts.append("hostile %.0fm sh%.0f hull%.0f" % (dist_m(s, p), hc.get_shield(), s.get_editor_property("current_hull_integrity")))
        log("%s t=%.0f %s" % (label, self.since(), ", ".join(parts)))

    def tick(self, dt):
        try:
            if world() is None:
                log("PIE not running")
                self.finish()
                return
            # Game time: a backgrounded editor runs at a few fps and game time runs slower than real time.
            self.t = unreal.GameplayStatics.get_time_seconds(world())
            getattr(self, "stage_" + self.stage)()
        except Exception as e:  # never leave a throwing callback registered
            log("ERROR %s" % e)
            self.finish()

    def stage_setup(self):
        p = player()
        stations = list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.SpaceStation))
        if not isinstance(p, unreal.Spaceship) or p.is_wrecked() or not stations:
            check("setup", False, "player isn't flying a working ship (%s), or no station" % (p.get_name() if p else None))
            self.finish()
            return
        self.station = stations[0]
        for c in ("HostileWaves 0", "ClearHostiles", "ClearWrecks", "adastrea.Security 3"):
            cmd(c)
        self.go("settle_start")

    def stage_settle_start(self):
        # Let any distress call the CombatTest waves opened close (8 s without hostiles).
        if self.since() < 10.0:
            return
        p = player()
        away = p.get_actor_location() - self.station.get_actor_location()
        away = away.normal() if away.length() > 1 else unreal.Vector(1, 0, 0)
        p.set_actor_location(self.station.get_actor_location() + away * STATION_OFFSET, False, True)
        cmd("SpawnPatrols 2")
        log("setup: player %.0f m from %s, %d patrols" % (dist_m(p, self.station), self.station.get_name(), len(patrol_ships())))
        check("patrols spawned", len(patrol_ships()) >= 2)
        self.go("spawn_high")

    def stage_spawn_high(self):
        if self.since() < 3.0:
            return
        before = set(s.get_name() for s in hostile_ships())
        cmd("SpawnHostiles 2")
        self.hostiles = [s for s in hostile_ships() if s.get_name() not in before]
        log("high: spawned %d hostiles" % len(self.hostiles))
        self.next_report = 0.0
        self.go("watch_high")

    def stage_watch_high(self):
        p = player()
        for s in patrol_ships():
            self.closest_patrol_m = min(self.closest_patrol_m, dist_m(s, p))
        if self.since() >= self.next_report:
            self.report("high")
            self.next_report += 5.0
        if self.hostiles and all(s.is_wrecked() for s in self.hostiles):
            check("patrols came to the player", self.closest_patrol_m < 800.0, "closest %.0f m" % self.closest_patrol_m)
            check("hostiles disabled", True, "after %.0f s" % self.since())
            self.go("settle_high")
        elif self.since() > HIGH_TIMEOUT:
            self.report("high timeout")
            check("patrols came to the player", self.closest_patrol_m < 800.0, "closest %.0f m" % self.closest_patrol_m)
            check("hostiles disabled", False, "timed out")
            self.go("settle_high")

    def stage_settle_high(self):
        # Give the call time to close (8 s quiet) before the next stage.
        if self.since() < 12.0:
            return
        for c in ("ClearHostiles", "ClearWrecks", "adastrea.Security 0"):
            cmd(c)
        p = player()
        # Move to the far side of the station, out of sensor range of the patrols at the old scene.
        back = self.station.get_actor_location() - p.get_actor_location()
        back = back.normal() if back.length() > 1 else unreal.Vector(-1, 0, 0)
        p.set_actor_location(self.station.get_actor_location() + back * STATION_OFFSET, False, True)
        self.patrol_start = {s.get_name(): dist_m(s, p) for s in patrol_ships()}
        before = set(s.get_name() for s in hostile_ships())
        cmd("SpawnHostiles 1")
        self.hostiles = [s for s in hostile_ships() if s.get_name() not in before]
        log("none: spawned %d hostile, patrols at %s" % (len(self.hostiles), self.patrol_start))
        self.next_report = 0.0
        self.go("watch_none")

    def stage_watch_none(self):
        if self.since() >= self.next_report:
            self.report("none")
            self.next_report += 5.0
        if self.since() < NONE_WATCH:
            return
        p = player()
        # Patrols left at the scene from stage 1 may still be close; what matters is that none flew in.
        came = [s.get_name() for s in patrol_ships()
                if dist_m(s, p) < 800.0 and self.patrol_start.get(s.get_name(), 0.0) >= 800.0]
        check("nobody answers at None security", not came, "came: %s" % came)
        cmd("ClearHostiles")
        self.finish()


Runner()
