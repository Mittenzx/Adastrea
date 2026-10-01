"""PIE test for avatar Phase 1: code-driven locomotion (run inside the editor while PIE is up).

    py "<project>/Tools/pie_locomotion_test.py"

Boards the player's ship (the pawn PIE starts in), goes third person, then drives
the avatar with injected Enhanced Input and checks the body animates:
  idle (feet still), walk forward / strafe right / walk back (direction follows the
  move, feet stride), sprint, crouch (pelvis drops) and crouch-walk, and first person
  (body hidden from the player but still rendered for its shadow).
A stride check compares the planted foot's world speed with the body's: a foot that
slides along the deck moves at a good fraction of the walking speed.
Results are logged as "LOCO ..." lines ending with "LOCO SUMMARY". Screenshots
(HighResShot) land in Saved/Screenshots/WindowsEditor.
"""
import math

import unreal

results = []


def log(msg):
    unreal.log("LOCO " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def pc():
    return unreal.GameplayStatics.get_player_controller(world(), 0)


def check(name, ok, note=""):
    results.append((name, ok))
    log("%s %s %s" % ("PASS" if ok else "FAIL", name, note))


# (name, move vector, sprint, crouch, seconds, back to the start spot first)
# Each run starts from where the avatar landed; collision is off during the test.
SEGMENTS = [
    ("idle", (0, 0), False, False, 1.5, True),
    ("walk", (1, 0), False, False, 1.6, True),
    ("walk_back", (-1, 0), False, False, 1.2, False),
    ("strafe_right", (0, 1), False, False, 1.3, True),
    ("sprint", (1, 0), True, False, 1.2, True),
    ("crouch_idle", (0, 0), False, True, 1.5, True),
    ("crouch_walk", (1, 0), False, True, 2.0, True),
]


class Runner:
    def __init__(self):
        self.t = 0.0
        self.stage = "board"
        self.mark = 0.0
        self.seg = -1
        self.stats = {}
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def done(self):
        unreal.unregister_slate_post_tick_callback(self.h)
        passed = sum(1 for _, ok in results if ok)
        log("SUMMARY %d/%d passed; failed: %s" % (passed, len(results), [n for n, ok in results if not ok]))

    def since(self):
        return self.t - self.mark

    def tick(self, dt):
        try:
            if world() is None:
                log("no PIE world - start PIE first")
                self.done()
                return
            self.t += dt
            if self.t > 240:
                check("timeout", False, self.stage)
                self.done()
                return
            self._tick(dt)
        except Exception:
            import traceback
            log("EXCEPTION %s" % traceback.format_exc())
            self.done()

    # -- sampling ------------------------------------------------------------
    def sample(self, dt):
        a, m, anim = self.avatar, self.mesh, self.anim
        xf = a.get_actor_transform()
        local = {b: xf.inverse_transform_location(m.get_socket_location(b)) for b in ("foot_l", "foot_r", "pelvis")}
        world_feet = {b: m.get_socket_location(b) for b in ("foot_l", "foot_r")}
        s = self.stats.setdefault(self.cur, {"speed": [], "dir": [], "crouch": [], "gap": [], "pelvis": [],
                                             "planted": [], "body": []})
        if self.since() > 0.6:  # past the start-up transient
            s["speed"].append(anim.get_editor_property("speed"))
            s["dir"].append(anim.get_editor_property("direction"))
            s["crouch"].append(anim.get_editor_property("crouch_alpha"))
            # stride: how far apart the feet get along the direction of travel
            s["gap"].append((local["foot_l"] - local["foot_r"]).length())
            s["pelvis"].append(local["pelvis"].z)
            if self.prev_feet and dt > 0:
                # the slower-moving foot this frame is the planted one
                # horizontal only: the floor snap can jitter the capsule vertically
                v = min(math.hypot(world_feet[b].x - self.prev_feet[b].x, world_feet[b].y - self.prev_feet[b].y) / dt
                        for b in world_feet)
                s["planted"].append(v)
                here = a.get_actor_location()
                s["body"].append(math.hypot(here.x - self.prev_body.x, here.y - self.prev_body.y) / dt)
        self.prev_feet = world_feet
        self.prev_body = a.get_actor_location()

    def drive(self):
        name, move, sprint, crouch, _, _ = SEGMENTS[self.seg]
        if move != (0, 0):
            self.sub.inject_input_vector_for_action(self.move_action, unreal.Vector(move[0], move[1], 0), [], [])
        # bool actions: any non-zero magnitude reads as pressed
        if sprint:
            self.sub.inject_input_vector_for_action(self.sprint_action, unreal.Vector(1, 0, 0), [], [])
        if crouch:
            self.sub.inject_input_vector_for_action(self.crouch_action, unreal.Vector(1, 0, 0), [], [])

    # -- state machine ---------------------------------------------------------
    def _tick(self, dt):
        p = pc()
        pawn = p.get_controlled_pawn()
        if self.stage == "board":
            if isinstance(pawn, unreal.Spaceship):
                p.enter_ship_interior(pawn)
                self.stage, self.mark = "landing", self.t
            elif self.since() > 20:
                check("board", False, "PIE pawn is %s" % pawn)
                self.done()
        elif self.stage == "landing":
            if isinstance(pawn, unreal.SpaceshipAvatar) and self.since() > 6:
                self.avatar = pawn
                self.mesh = pawn.get_editor_property("mesh")
                self.anim = self.mesh.get_anim_instance()
                check("anim instance is code locomotion", isinstance(self.anim, unreal.AdastreaLocomotionAnimInstance),
                      type(self.anim).__name__)
                pawn.set_first_person_view(False)
                self.sub = [s for s in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem)
                            if s.get_world() == world()][0]
                self.move_action = pawn.get_editor_property("move_action")
                self.sprint_action = pawn.get_editor_property("sprint_action")
                self.crouch_action = pawn.get_editor_property("crouch_action")
                # This test is about the body, not the deck layout: switch the capsule's
                # collision off so walls, furniture and the helm seat's trigger can't stop
                # or sit the avatar mid-run (the floor snap is a line trace and still holds).
                pawn.set_actor_enable_collision(False)
                self.start = pawn.get_actor_location()
                p.set_control_rotation(unreal.Rotator(0, 0, pawn.get_actor_rotation().yaw))
                self.next_segment()
            elif self.since() > 30:
                check("board", False, "never became the avatar")
                self.done()
        elif self.stage == "segment":
            name, _, _, _, length, _ = SEGMENTS[self.seg]
            if pawn != self.avatar:
                check("stayed on foot", False, "lost the avatar during '%s' (walked into the seat?)" % name)
                self.done()
                return
            self.drive()
            self.sample(dt)
            if name in ("walk", "crouch_walk") and not self.shot and self.since() > length * 0.6:
                unreal.SystemLibrary.execute_console_command(world(), "HighResShot 1280x720")
                self.shot = True
            if self.since() > length:
                self.next_segment()
        elif self.stage == "settle":
            # let sprint/crouch release (Completed) and the body slow down between segments
            if SEGMENTS[self.seg][5] and self.since() > 0.5 and not self.reset_done:
                self.avatar.set_actor_location(self.start, False, True)
                self.reset_done = True
            if self.since() > 0.8:
                self.stage, self.mark = "segment", self.t
                self.prev_feet = None
        elif self.stage == "firstperson":
            if self.since() > 0.5:
                self.avatar.set_first_person_view(True)
                self.stage, self.mark = "fp_check", self.t
        elif self.stage == "fp_check":
            if self.since() > 0.5:
                m = self.mesh
                check("first person: body still rendered (for its shadow)", m.is_visible() and not m.get_editor_property("hidden_in_game"))
                check("first person: body hidden from own camera", m.get_editor_property("owner_no_see"))
                self.avatar.set_first_person_view(False)
                check("third person: body visible to own camera", not m.get_editor_property("owner_no_see"))
                self.avatar.set_actor_location(self.start, False, True)
                self.avatar.set_actor_enable_collision(True)
                self.evaluate()
                self.done()

    def next_segment(self):
        self.seg += 1
        if self.seg >= len(SEGMENTS):
            self.stage, self.mark = "firstperson", self.t
            return
        self.cur = SEGMENTS[self.seg][0]
        self.shot = False
        self.prev_feet = None
        self.reset_done = False
        self.stage, self.mark = "settle", self.t

    # -- verdicts ----------------------------------------------------------------
    def evaluate(self):
        def avg(v):
            return sum(v) / len(v) if v else float("nan")

        def spread(v):
            return (max(v) - min(v)) if v else 0.0

        st = self.stats
        for name, s in st.items():
            log("%s speed=%.0f dir=%.0f crouch=%.2f stride_gap=%.0f..%.0f pelvis=%.0f planted=%.0f body=%.0f n=%d" % (
                name, avg(s["speed"]), avg(s["dir"]), avg(s["crouch"]),
                min(s["gap"] or [0]), max(s["gap"] or [0]), avg(s["pelvis"]), avg(sorted(s["planted"])[:max(1, len(s["planted"]) // 3)]),
                avg(s["body"]), len(s["speed"])))

        idle, walk = st["idle"], st["walk"]
        check("idle: not moving", avg(idle["speed"]) < 5, "%.1f" % avg(idle["speed"]))
        check("idle: feet still", spread(idle["gap"]) < 8, "gap spread %.1f" % spread(idle["gap"]))
        check("walk: speed ~190", 150 < avg(walk["speed"]) < 210, "%.0f" % avg(walk["speed"]))
        check("walk: direction forward", abs(avg(walk["dir"])) < 20, "%.0f" % avg(walk["dir"]))
        check("walk: legs stride", spread(walk["gap"]) > 25, "gap spread %.0f" % spread(walk["gap"]))
        check("strafe right: direction ~+90", 60 < avg(st["strafe_right"]["dir"]) < 120, "%.0f" % avg(st["strafe_right"]["dir"]))
        back = [abs(d) for d in st["walk_back"]["dir"]]
        check("walk back: direction ~180", avg(back) > 150, "%.0f" % avg(back))
        check("sprint: faster than walk", avg(st["sprint"]["speed"]) > avg(walk["speed"]) * 1.6, "%.0f" % avg(st["sprint"]["speed"]))
        check("sprint: legs stride", spread(st["sprint"]["gap"]) > 30, "gap spread %.0f" % spread(st["sprint"]["gap"]))
        ci, cw = st["crouch_idle"], st["crouch_walk"]
        check("crouch: pose blended in", avg(ci["crouch"]) > 0.9, "%.2f" % avg(ci["crouch"]))
        check("crouch: pelvis lower than standing", avg(idle["pelvis"]) - avg(ci["pelvis"]) > 20,
              "%.0f cm lower" % (avg(idle["pelvis"]) - avg(ci["pelvis"])))
        check("crouch walk: slow and striding", avg(cw["speed"]) < 120 and spread(cw["gap"]) > 15,
              "speed %.0f gap spread %.0f" % (avg(cw["speed"]), spread(cw["gap"])))
        for name in ("walk", "walk_back", "strafe_right", "crouch_walk"):
            s = st[name]
            planted = sorted(s["planted"])[:max(1, len(s["planted"]) // 3)]
            # a planted foot should be (nearly) still while the body moves past it
            check("%s: planted foot doesn't slide" % name, avg(planted) < 0.35 * avg(s["body"]),
                  "planted %.0f vs body %.0f cm/s" % (avg(planted), avg(s["body"])))


Runner()
