"""PIE walk test for the full-deck ship interiors (run inside the editor while PIE is up).

    py "C:/Users/akuma/Adastrea/Tools/pie_walk_ship_decks.py" [Ship ...]

For each ship: spawn its blueprint far from everything, possess it, enter the
interior, check the deck mounted (parts, walk collision, socket lights, avatar
standing on the floor), then WALK it: steer the avatar with injected move input
along the contract's walk route (Tools/build_ship_decks.py: entry -> every
room -> the helm) and finally into the helm seat. The seat trigger has to put
the player back in the ship.

It returns immediately and runs on a slate tick. Results go to the log as
"DECKWALK ..." lines, ending with "DECKWALK SUMMARY". Waypoints the avatar
can't reach within 5 s are logged as STUCK and skipped (teleported past), so
one run lists every problem.
"""
import glob
import json
import math
import os
import sys

import unreal

GEN = "C:/Users/akuma/Adastrea/Assets/FBX/generated"
REACH = 30.0
STUCK_S = 5.0
SHIP_TIMEOUT_S = 420.0


def log(msg):
    unreal.log("DECKWALK " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def pc():
    return unreal.GameplayStatics.get_player_controller(world(), 0)


def load_contracts(names):
    out = []
    for fp in sorted(glob.glob(os.path.join(GEN, "SM_Int_*_Decks_contract.json"))):
        with open(fp) as fh:
            c = json.load(fh)
        if "ship" in c and (not names or c["ship"] in names):
            out.append(c)
    return out


class Runner:
    def __init__(self, contracts):
        self.queue = list(contracts)
        self.results = {}
        self.stage = "next"
        self.h = unreal.register_slate_post_tick_callback(self.tick)
        self.sub = None

    # -- helpers -----------------------------------------------------------
    def to_world(self, x, y, z=100.0):
        return self.interior.get_actor_transform().transform_location(unreal.Vector(x, y, z))

    def to_local(self, v):
        return self.interior.get_actor_transform().inverse_transform_location(v)

    def finish(self, ok, note):
        c = self.c
        r = self.results.setdefault(c["ship"], {})
        r.update(ok=ok and not r.get("stuck"), note=note)
        log("RESULT %s %s %s stuck=%d" % (c["ship"], "PASS" if r["ok"] else "FAIL", note, len(r.get("stuck", []))))
        try:
            if self.ship:
                if pc().get_controlled_pawn() != self.ship:
                    pc().exit_ship_interior(self.ship)
                self.ship.destroy_actor()
        except Exception as e:
            log("cleanup: %s" % e)
        self.stage = "next"

    # -- state machine -------------------------------------------------------
    def tick(self, dt):
        try:
            self._tick(dt)
        except Exception:
            import traceback
            log("EXCEPTION %s" % traceback.format_exc())
            if hasattr(self, "c"):
                self.finish(False, "exception")
            else:
                unreal.unregister_slate_post_tick_callback(self.h)

    def _tick(self, dt):
        w = world()
        if w is None:
            log("no PIE world - start PIE first")
            unreal.unregister_slate_post_tick_callback(self.h)
            return
        if self.stage == "next":
            if not self.queue:
                unreal.unregister_slate_post_tick_callback(self.h)
                passed = [k for k, v in self.results.items() if v.get("ok")]
                log("SUMMARY %d/%d passed; failed: %s" % (len(passed), len(self.results),
                                                         [k for k in self.results if k not in passed]))
                return
            self.c = self.queue.pop(0)
            self.t = 0.0
            self.frames = 0
            self.ship = None
            # This build's Python has no deferred spawn and EditorAssetLibrary refuses
            # to load during PIE, so spawn through the cheat manager's summon.
            cls_path = "%s.%s_C" % (self.c["bp"], self.c["bp"].rsplit("/", 1)[1])
            cls = unreal.load_object(None, cls_path)
            self.before = set(a.get_name() for a in unreal.GameplayStatics.get_all_actors_of_class(w, cls))
            self.cls = cls
            # summon spawns just ahead of the pawn: from an empty spot with no pawn,
            # so a big hull doesn't collide with the ship we're holding
            p = pc()
            p.un_possess()
            p.set_actor_location(unreal.Vector(400000 + 60000 * len(self.results), -300000, 80000), False, True)
            unreal.SystemLibrary.execute_console_command(w, "EnableCheats")
            unreal.SystemLibrary.execute_console_command(w, "summon " + cls_path)
            self.stage = "spawned"
            return
        self.t += dt
        self.frames += 1
        if self.t > SHIP_TIMEOUT_S:
            self.finish(False, "timeout at waypoint %d" % getattr(self, "wi", -1))
            return
        if self.stage == "spawned":
            new = [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, self.cls) if a.get_name() not in self.before]
            if not new:
                if self.frames > 30:
                    self.finish(False, "summon spawned nothing")
                return
            self.ship = new[0]
            loc = unreal.Vector(400000 + 60000 * len(self.results), -300000, 50000)
            self.ship.set_actor_location(loc, False, True)
            pc().possess(self.ship)
            self.frames = 0
            self.stage = "enter"
            log("=== %s: spawned %s" % (self.c["ship"], self.ship.get_name()))
            return
        if self.stage == "enter":
            if self.frames == 20:
                pc().enter_ship_interior(self.ship)
            if self.frames >= 60:
                # boarding lowers the avatar from the ship into the interior pocket
                # over ~80 frames: start only once it stands on the deck
                a = pc().get_controlled_pawn()
                it = a.get_editor_property("current_interior") if isinstance(a, unreal.SpaceshipAvatar) else None
                landed = it and it.get_actor_transform().inverse_transform_location(a.get_actor_location()).z < 200
                if landed or self.frames > 600:
                    self.check_mount()
            return
        if self.stage == "walk":
            self.walk(dt)

    def check_mount(self):
        a = pc().get_controlled_pawn()
        if not isinstance(a, unreal.SpaceshipAvatar):
            self.finish(False, "no avatar after enter (%s)" % a)
            return
        self.avatar = a
        self.interior = a.get_editor_property("current_interior")
        comps = self.interior.get_components_by_class(unreal.StaticMeshComponent)
        names = [c.static_mesh.get_name() for c in comps if c.static_mesh]
        prefix = self.c["prefix"]
        mine = [n for n in names if n.startswith(prefix)]
        want = ["%s_%s" % (prefix, p) for p in self.c["parts"]]
        missing = [p for p in want if p not in mine]
        lights = len(self.interior.get_components_by_class(unreal.PointLightComponent))
        loc = self.to_local(a.get_actor_location())
        ent = self.c["sockets_ue_design_cm"]["Entry"]
        r = self.results.setdefault(self.c["ship"], {})
        r.update(parts=len(mine), missing=missing, lights=lights, stuck=[])
        log("%s mounted %d/%d parts missing=%s lights=%d/%d avatar_local=(%.0f,%.0f,%.0f) entry=(%.0f,%.0f)" % (
            self.c["ship"], len(mine), len(want), missing, lights, self.c["light_count"],
            loc.x, loc.y, loc.z, ent[0], ent[1]))
        if missing:
            self.finish(False, "missing parts")
            return
        self.sub = [s for s in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem)
                    if "Default__" not in s.get_path_name()][0]
        self.move = a.get_editor_property("move_action")
        seat = self.c["sockets_ue_design_cm"]["Seat"]
        self.route = [tuple(p) for p in self.c["walk"]["route"]] + [(seat[0], seat[1], "SEAT")]
        self.wi = 0
        self.best = 1e9
        self.since = 0.0
        self.walked = 0.0
        self.last = a.get_actor_location()
        self.zmin, self.zmax = 1e9, -1e9
        self.stage = "walk"

    def walk(self, dt):
        p = pc()
        if p.get_controlled_pawn() == self.ship:
            # the "helm" goal sits inside the seat trigger, so the return can come on
            # the last leg: pass once every room goal before it was reached
            seat = self.c["sockets_ue_design_cm"]["Seat"]

            def in_trigger(wp):   # ExitTrigger half extents 100 x 150, plus the capsule
                return abs(wp[0] - seat[0]) < 142 and abs(wp[1] - seat[1]) < 192
            last_room = max([i for i, wp in enumerate(self.route)
                             if wp[2] and wp[2] not in ("helm", "SEAT", "Bridge") and not in_trigger(wp)] or [-1])
            ok = self.wi > last_room
            self.finish(ok, "seat returned to ship after %.0f m walked, z %.0f..%.0f" % (
                self.walked / 100.0, self.zmin, self.zmax) if ok else "left the interior early at wp %d" % self.wi)
            return
        a = self.avatar
        loc = a.get_actor_location()
        self.walked += (loc - self.last).length()
        self.last = loc
        ll = self.to_local(loc)
        if ll.z > self.zmax + 150 and self.zmax > -1e8:
            log("%s height jump to %.0f at local (%.0f,%.0f) going to wp %d" % (self.c["ship"], ll.z, ll.x, ll.y, self.wi))
        self.zmin, self.zmax = min(self.zmin, ll.z), max(self.zmax, ll.z)
        if ll.z < -150:
            self.finish(False, "fell through the floor at local (%.0f,%.0f,%.0f)" % (ll.x, ll.y, ll.z))
            return
        x, y, tag = self.route[self.wi]
        dx, dy = x - ll.x, y - ll.y
        dist = math.hypot(dx, dy)
        if dist < REACH and tag != "SEAT":
            if tag:
                log("%s reached %s" % (self.c["ship"], tag))
            self.wi += 1
            self.best, self.since = 1e9, 0.0
            return
        if dist < self.best - 10:
            self.best, self.since = dist, 0.0
        else:
            self.since += dt
        if self.since > STUCK_S:
            if tag == "SEAT":
                self.finish(False, "could not reach/trigger the seat (%.0f cm away)" % dist)
                return
            r = self.results[self.c["ship"]]
            r["stuck"].append((self.wi, tag, round(ll.x), round(ll.y)))
            log("%s STUCK going to wp %d %s (%.0f,%.0f) at local (%.0f,%.0f) %.0f cm short" % (
                self.c["ship"], self.wi, tag, x, y, ll.x, ll.y, dist))
            a.set_actor_location(self.to_world(x, y, ll.z), False, True)
            self.wi += 1
            self.best, self.since = 1e9, 0.0
            return
        yaw_local = math.degrees(math.atan2(dy, dx))
        p.set_control_rotation(unreal.Rotator(0, 0, self.interior.get_actor_rotation().yaw + yaw_local))
        # ease off near the waypoint so big low-FPS steps don't cut door corners
        self.sub.inject_input_vector_for_action(self.move, unreal.Vector(max(0.3, min(1.0, dist / 150.0)), 0, 0), [], [])


names = [a for a in sys.argv[1:] if not a.startswith("--")]
RUNNER = Runner(load_contracts(names))
log("queued %d ships" % len(RUNNER.queue))
