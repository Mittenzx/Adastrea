"""PIE test for avatar Phase 0: Manny body and ragdoll (run inside the editor while PIE is up).

    py "<project>/Tools/pie_ragdoll_test.py"

Boards the player's ship (the pawn PIE starts in), switches the avatar to third person,
spawns three crew dummies, then:
  1. checks the avatar and the dummies stand on the deck with a skeletal body,
  2. ragdolls everyone with a push and checks the bodies come to rest ON the deck
     (not through it),
  3. gets everyone back up and checks they're standing again,
  4. ragdolls in zero-g and checks the bodies stay off the deck,
  5. ragdolls the player and sits down at the helm (the ragdoll must end).
Results are logged as "RAGDOLL ..." lines ending with "RAGDOLL SUMMARY". Screenshots
(HighResShot) land in Saved/Screenshots/WindowsEditor.
"""
import unreal

results = []


def log(msg):
    unreal.log("RAGDOLL " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def pc():
    return unreal.GameplayStatics.get_player_controller(world(), 0)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(world(), c)


def check(name, ok, note=""):
    results.append((name, ok))
    log("%s %s %s" % ("PASS" if ok else "FAIL", name, note))


def bodies():
    return list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.AdastreaCharacter))


def pelvis_z(c):
    return c.get_editor_property("mesh").get_socket_location("pelvis").z


class Runner:
    def __init__(self):
        self.frames = 0
        self.t = 0.0
        self.stage = "board"
        self.mark = 0.0
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def done(self):
        unreal.unregister_slate_post_tick_callback(self.h)
        passed = sum(1 for _, ok in results if ok)
        log("SUMMARY %d/%d passed; failed: %s" % (passed, len(results), [n for n, ok in results if not ok]))

    def go(self, stage):
        self.stage = stage
        self.mark = self.t

    def since(self):
        return self.t - self.mark

    def tick(self, dt):
        try:
            if world() is None:
                log("no PIE world - start PIE first")
                self.done()
                return
            self.t += dt
            self.frames += 1
            if self.t > 240:
                check("timeout", False, self.stage)
                self.done()
                return
            self._tick()
        except Exception:
            import traceback
            log("EXCEPTION %s" % traceback.format_exc())
            self.done()

    def _tick(self):
        p = pc()
        pawn = p.get_controlled_pawn()
        if self.stage == "board":
            if isinstance(pawn, unreal.Spaceship):
                self.ship = pawn
                p.enter_ship_interior(pawn)
                self.go("landing")
            elif self.since() > 20:
                check("board", False, "PIE pawn is %s, not a ship" % pawn)
                self.done()
        elif self.stage == "landing":
            if isinstance(pawn, unreal.SpaceshipAvatar) and self.since() > 6:
                self.avatar = pawn
                self.interior = pawn.get_editor_property("current_interior")
                pawn.set_first_person_view(False)
                self.floor = pawn.get_actor_location().z - 88.0
                mesh = pawn.get_editor_property("mesh")
                skm = mesh.get_skeletal_mesh_asset()
                check("avatar has Manny body", skm is not None and "Manny" in skm.get_name(),
                      str(skm.get_name() if skm else None))
                check("avatar body visible in 3P", mesh.is_visible())
                log("floor z=%.1f interior=%s" % (self.floor, self.interior.get_name() if self.interior else None))
                cmd("adastrea.SpawnCrewDummy 3")
                self.go("dummies")
            elif self.since() > 30:
                check("board", False, "never became the avatar")
                self.done()
        elif self.stage == "dummies" and self.since() > 3:
            # the dummies only: boarding also brings the ship's crew (AdastreaNPC) aboard
            ds = [b for b in bodies() if b != self.avatar and not isinstance(b, unreal.AdastreaNPC)]
            check("3 dummies spawned", len(ds) == 3, str(len(ds)))
            standing = [b for b in ds if abs(b.get_actor_location().z - 88.0 - self.floor) < 30]
            check("dummies stand on deck", len(standing) == len(ds),
                  str([round(b.get_actor_location().z - 88.0 - self.floor, 1) for b in ds]))
            cmd("HighResShot 1280x720")
            cmd("adastrea.Ragdoll all push")
            self.go("fallen")
        elif self.stage == "fallen" and self.since() > 4:
            heights = [round(pelvis_z(b) - self.floor, 1) for b in bodies()]
            check("all ragdolled", all(b.is_ragdoll() for b in bodies()))
            # A lying pelvis joint sits ~8-30 cm above the deck. Below ~5 cm means the
            # body is sinking into it (a query-only deck once let them sink ~20 cm).
            check("ragdolls rest on deck", all(5 < h < 60 for h in heights), str(heights))
            cmd("HighResShot 1280x720")
            cmd("adastrea.Ragdoll all")  # toggle: stand back up
            self.go("standing")
        elif self.stage == "standing" and self.since() > 2:
            check("all stood up", not any(b.is_ragdoll() for b in bodies()))
            feet = [round(b.get_actor_location().z - 88.0 - self.floor, 1) for b in bodies()]
            check("stood up on deck", all(abs(f) < 30 for f in feet), str(feet))
            check("avatar mesh reattached", self.avatar.get_editor_property("mesh").get_attach_parent() is not None)
            cmd("adastrea.Ragdoll all zerog push")
            self.go("zerog")
        elif self.stage == "zerog" and self.since() > 3:
            heights = [round(pelvis_z(b) - self.floor, 1) for b in bodies()]
            # Pelvis starts ~95 cm up and nothing pulls it down; most bodies stay well up.
            check("zero-g bodies float", sum(1 for h in heights if h > 60) >= 3, str(heights))
            cmd("HighResShot 1280x720")
            cmd("adastrea.Ragdoll all")
            self.go("helm")
        elif self.stage == "helm" and self.since() > 2:
            for b in bodies():
                if b != self.avatar and not isinstance(b, unreal.AdastreaNPC):
                    b.destroy_actor()
            cmd("adastrea.Ragdoll push")
            self.go("sit")
        elif self.stage == "sit" and self.since() > 2:
            check("player ragdolled", self.avatar.is_ragdoll())
            self.avatar.sit_down()
            self.go("seated")
        elif self.stage == "seated" and self.since() > 2:
            check("back at helm", isinstance(pc().get_controlled_pawn(), unreal.Spaceship))
            check("avatar ragdoll cleared on sit", not self.avatar.is_ragdoll())
            self.done()


Runner()
