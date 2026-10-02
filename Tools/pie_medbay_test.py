"""PIE test for the rescue medical bay (run inside the editor while PIE is up in CombatTest).

    py "<project>/Tools/pie_medbay_test.py"

Disables the player's ship 1 km from the CombatTest station (by damage, no hostiles), then
checks the rescue: the pilot ejects in a pod, wakes on foot in the station's medical bay,
can't leave through the airlock while the ship is still being towed in, and boards the
docked, repaired ship through the airlock once it's in. A screenshot of the medical bay
lands in Saved/Screenshots/WindowsEditor.

Progress is logged as "MEDT ..." lines ending with "MEDT SUMMARY".
"""
import unreal

STATION_OFFSET = 100000.0  # cm
TIMEOUT = 240.0  # game seconds
results = []


def log(msg):
    unreal.log("MEDT " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def pc():
    return unreal.GameplayStatics.get_player_controller(world(), 0)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(world(), c, pc())


def check(name, ok, note=""):
    results.append((name, ok))
    log("%s %s %s" % ("PASS" if ok else "FAIL", name, note))


class Runner:
    def __init__(self):
        self.t = 0.0
        self.mark = 0.0
        self.stage = "setup"
        self.ship = None
        self.woke = False
        self.blocked_checked = False
        self.shot_taken = False
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def go(self, stage):
        self.stage, self.mark = stage, self.t

    def since(self):
        return self.t - self.mark

    def finish(self):
        unreal.unregister_slate_post_tick_callback(self.h)
        passed = sum(1 for _, ok in results if ok)
        log("SUMMARY %d/%d passed" % (passed, len(results)))

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

    def stage_setup(self):
        p = pc().get_controlled_pawn()
        stations = list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.SpaceStation))
        if not isinstance(p, unreal.Spaceship) or p.is_wrecked() or not stations:
            check("setup", False, "player isn't flying a working ship, or no station")
            self.finish()
            return
        for c in ("HostileWaves 0", "ClearHostiles", "ClearWrecks"):
            cmd(c)
        self.ship = p
        station = stations[0]
        away = p.get_actor_location() - station.get_actor_location()
        away = away.normal() if away.length() > 1 else unreal.Vector(1, 0, 0)
        p.set_actor_location(station.get_actor_location() + away * STATION_OFFSET, False, True)
        self.go("disable")

    def stage_disable(self):
        if self.since() < 2.0:
            return
        hc = self.ship.get_editor_property("health_component")
        hc.apply_damage(100000.0, None)
        check("ship disabled", self.ship.is_wrecked())
        self.go("watch")

    def stage_watch(self):
        pawn = pc().get_controlled_pawn()
        walking = pc().is_walking_station()
        if not self.woke and walking:
            self.woke = True
            check("woke on foot in the station", isinstance(pawn, unreal.SpaceshipAvatar), "after %.0f s, pawn %s" % (self.since(), pawn.get_name() if pawn else None))
            self.wake_t = self.t
        if self.woke and not self.shot_taken and self.t - self.wake_t > 4.0:
            self.shot_taken = True
            cmd("HighResShot 1280x720")
            log("screenshot of the medical bay")
        if self.woke and not self.blocked_checked and self.t - self.wake_t > 5.0:
            self.blocked_checked = True
            if self.ship.call_method("IsDocked"):
                log("ship already docked before the airlock check; skipping it")
            else:
                pc().exit_station_interior(False)
                check("airlock refused while the ship is out", pc().is_walking_station())
        if self.woke and self.ship.call_method("IsDocked") and not self.ship.is_wrecked():
            log("ship docked after %.0f s" % self.since())
            self.go("board")
            return
        if self.since() > TIMEOUT:
            check("ship towed in and docked", False, "timed out; walking=%s docked=%s wreck=%s" % (walking, self.ship.call_method("IsDocked"), self.ship.is_wrecked()))
            self.finish()

    def stage_board(self):
        if self.since() < 2.0:
            return
        check("ship towed in and docked", True)
        pc().exit_station_interior(False)
        pawn = pc().get_controlled_pawn()
        check("boarded through the airlock", pawn == self.ship and not pc().is_walking_station(), "pawn %s" % (pawn.get_name() if pawn else None))
        self.finish()


Runner()
