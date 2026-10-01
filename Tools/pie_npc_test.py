"""PIE test for avatar Phase 2: crew NPCs walking a deck (run inside the editor while PIE is up).

    py "<project>/Tools/pie_npc_test.py" [Ship]

With no Ship, boards the ship PIE starts in. With a ship name (e.g. Cruiser, Battleship),
summons that ship's blueprint (from its deck contract) far from everything and boards it,
for bigger multi-room decks.

Checks: crew spawns on its own, more can be added (adastrea.NPC.Spawn; only on decks
with room for more than one), and over ~30 s
they walk (move metres, reach destinations, rarely give up), stay on the deck (nobody
falls through or leaves it), animate to their speed, and spread through the deck.
Talking to one stops and turns them; a ragdolled one gets up and carries on; sitting
back down at the helm removes the crew.
Results are logged as "NPCT ..." lines ending with "NPCT SUMMARY". Screenshots land
in Saved/Screenshots/WindowsEditor.
"""
import glob
import json
import math
import os
import sys

import unreal

GEN = "C:/Users/akuma/Adastrea/Assets/FBX/generated"
WATCH_S = 30.0
results = []


def log(msg):
    unreal.log("NPCT " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def pc():
    return unreal.GameplayStatics.get_player_controller(world(), 0)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(world(), c)


def check(name, ok, note=""):
    results.append((name, ok))
    log("%s %s %s" % ("PASS" if ok else "FAIL", name, note))


def npcs():
    return list(unreal.GameplayStatics.get_all_actors_of_class(world(), unreal.AdastreaNPC))


def ship_bp(name):
    for fp in glob.glob(os.path.join(GEN, "SM_Int_*_Decks_contract.json")):
        with open(fp) as fh:
            c = json.load(fh)
        if c.get("ship") == name:
            return "%s.%s_C" % (c["bp"], c["bp"].rsplit("/", 1)[1])
    return None


class Runner:
    def __init__(self, ship_name):
        self.ship_name = ship_name
        self.t = 0.0
        self.mark = 0.0
        self.stage = "summon" if ship_name else "board"
        self.track = {}
        self.anim_checked = []
        self.speeds = []
        self.watch_world_t0 = None
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def go(self, stage):
        self.stage, self.mark = stage, self.t

    def since(self):
        return self.t - self.mark

    def done(self):
        unreal.unregister_slate_post_tick_callback(self.h)
        passed = sum(1 for _, ok in results if ok)
        log("SUMMARY %s %d/%d passed; failed: %s" % (self.ship_name or "start-ship", passed, len(results),
                                                      [n for n, ok in results if not ok]))

    def tick(self, dt):
        try:
            if world() is None:
                log("no PIE world - start PIE first")
                self.done()
                return
            self.t += dt
            if self.t > 300:
                check("timeout", False, self.stage)
                self.done()
                return
            self._tick(dt)
        except Exception:
            import traceback
            log("EXCEPTION %s" % traceback.format_exc())
            self.done()

    def _tick(self, dt):
        p = pc()
        pawn = p.get_controlled_pawn()
        if self.stage == "summon":
            path = ship_bp(self.ship_name)
            if not path:
                check("ship contract", False, self.ship_name)
                self.done()
                return
            self.cls = unreal.load_object(None, path)
            self.before = set(a.get_name() for a in unreal.GameplayStatics.get_all_actors_of_class(world(), self.cls))
            p.un_possess()
            p.set_actor_location(unreal.Vector(400000, -300000, 80000), False, True)
            cmd("EnableCheats")
            cmd("summon " + path)
            self.go("summoned")
        elif self.stage == "summoned":
            new = [a for a in unreal.GameplayStatics.get_all_actors_of_class(world(), self.cls) if a.get_name() not in self.before]
            if new:
                new[0].set_actor_location(unreal.Vector(400000, -300000, 50000), False, True)
                p.possess(new[0])
                self.go("board")
            elif self.since() > 3:
                check("summon", False, self.ship_name)
                self.done()
        elif self.stage == "board":
            if isinstance(pawn, unreal.Spaceship) and self.since() > 0.5:
                p.enter_ship_interior(pawn)
                self.go("landing")
            elif self.since() > 20:
                check("board", False, str(pawn))
                self.done()
        elif self.stage == "landing":
            if isinstance(pawn, unreal.SpaceshipAvatar) and self.since() > 6:
                self.avatar = pawn
                self.interior = pawn.get_editor_property("current_interior")
                pawn.set_first_person_view(False)
                self.floor_z = pawn.get_actor_location().z - 88.0
                crew = npcs()
                check("crew spawned on boarding", len(crew) >= 1, "%d aboard" % len(crew))
                self.auto_count = len(crew)
                # A couple more than the ship's own crew, to see them share the deck - but
                # only where the population code judged there's room for more than one:
                # a fighter's single 80 cm corridor can't let two people pass.
                self.extra = 2 if self.auto_count >= 2 else 0
                if self.extra:
                    cmd("adastrea.NPC.Spawn %d" % self.extra)
                self.go("spawned")
            elif self.since() > 30:
                check("board", False, "never became the avatar")
                self.done()
        elif self.stage == "spawned" and self.since() > 0.5:
            crew = npcs()
            if self.extra:
                check("adastrea.NPC.Spawn adds crew", len(crew) >= self.auto_count + 1, "%d -> %d" % (self.auto_count, len(crew)))
            self.start = {n.get_name(): n.get_actor_location() for n in crew}
            self.track = {n.get_name(): {"dist": 0.0, "last": n.get_actor_location(), "minz": 1e9, "maxz": -1e9,
                                          "cells": set()} for n in crew}
            cmd("adastrea.NPC.Debug 4")
            self.watch_world_t0 = unreal.GameplayStatics.get_time_seconds(world())
            self.go("watch")
        elif self.stage == "watch":
            for n in npcs():
                tr = self.track.get(n.get_name())
                if not tr:
                    continue
                loc = n.get_actor_location()
                tr["dist"] += math.hypot(loc.x - tr["last"].x, loc.y - tr["last"].y)
                tr["last"] = loc
                tr["minz"] = min(tr["minz"], loc.z)
                tr["maxz"] = max(tr["maxz"], loc.z)
                tr["cells"].add((int(loc.x // 200), int(loc.y // 200)))
                v = n.get_velocity()
                self.speeds.append(math.hypot(v.x, v.y))
                if math.hypot(v.x, v.y) > 60 and len(self.anim_checked) < 40:
                    anim = n.get_editor_property("mesh").get_anim_instance()
                    self.anim_checked.append((math.hypot(v.x, v.y), anim.get_editor_property("speed")))
            if abs(self.since() - 12.0) < dt:
                cmd("HighResShot 1280x720")
            if self.since() > WATCH_S:
                self.evaluate_watch()
                self.go("talk")
        elif self.stage == "talk":
            walkers = [n for n in npcs() if n.get_velocity().length() > 60]
            self.talker = (walkers or npcs())[0]
            try:
                self.talker.interact(p)  # IWorldInteractable, as the avatar's E does
            except Exception as e:
                log("interact() not reachable from Python (%s); using the controller" % e)
                self.talker.get_controller().start_conversation(self.avatar, 5.0)
            self.go("talking")
        elif self.stage == "talking" and self.since() > 1.5:
            brain = self.talker.get_controller()
            v = self.talker.get_velocity()
            to_player = self.avatar.get_actor_location() - self.talker.get_actor_location()
            fwd = self.talker.get_actor_forward_vector()
            facing = (fwd.x * to_player.x + fwd.y * to_player.y) / max(1.0, math.hypot(to_player.x, to_player.y))
            check("talking: NPC stops", math.hypot(v.x, v.y) < 25, "%.0f cm/s" % math.hypot(v.x, v.y))
            check("talking: NPC turns to the player", facing > 0.7, "facing dot %.2f" % facing)
            check("talking: activity", brain.get_activity() == unreal.NPCActivity.TALKING, str(brain.get_activity()))
            self.ragdolled = npcs()[-1]
            self.ragdolled.enter_ragdoll(unreal.Vector(0, 0, 0), "None")
            self.go("ragdoll")
        elif self.stage == "ragdoll" and self.since() > 2.5:
            check("ragdolled NPC is disabled", self.ragdolled.get_controller().get_activity() == unreal.NPCActivity.DISABLED)
            self.ragdolled.exit_ragdoll()
            self.rag_arrived = self.ragdolled.get_controller().get_editor_property("arrived_count")
            self.rag_pos = self.ragdolled.get_actor_location()
            self.go("recover")
        elif self.stage == "recover" and self.since() > 12:
            moved = (self.ragdolled.get_actor_location() - self.rag_pos).length()
            check("NPC carries on after getting up", moved > 100 or
                  self.ragdolled.get_controller().get_editor_property("arrived_count") > self.rag_arrived, "moved %.0f cm" % moved)
            self.avatar.sit_down()
            self.go("left")
        elif self.stage == "left" and self.since() > 1.5:
            check("crew removed when the player leaves", len(npcs()) == 0, "%d left" % len(npcs()))
            self.done()

    def evaluate_watch(self):
        crew = npcs()
        dists = [round(self.track[n.get_name()]["dist"] / 100.0, 1) for n in crew if n.get_name() in self.track]
        log("walked (m): %s" % dists)
        game_s = unreal.GameplayStatics.get_time_seconds(world()) - self.watch_world_t0
        moving = sorted(v for v in self.speeds if v > 20)
        top = moving[-1] if moving else 0.0
        log("watched %.1f s wall / %.1f s game; walking speed median %.0f, max %.0f cm/s" % (
            self.since(), game_s, moving[len(moving) // 2] if moving else 0.0, top))
        check("walking pace is a person's", top < 200, "max %.0f cm/s" % top)
        check("every NPC walks around", all(d > 2.0 for d in dists), str(dists))
        arrivals = sum(n.get_controller().get_editor_property("arrived_count") for n in crew)
        gaveup = sum(n.get_controller().get_editor_property("gave_up_count") for n in crew)
        check("NPCs reach their destinations", arrivals >= len(crew), "%d arrivals, %d gave up" % (arrivals, gaveup))
        check("NPCs rarely get stuck", gaveup <= max(1, arrivals // 3), "%d gave up vs %d arrivals" % (gaveup, arrivals))
        lows = [round(self.track[n.get_name()]["minz"] - 88.0 - self.floor_z) for n in crew]
        check("nobody falls through the deck", all(z > -150 for z in lows), "lowest feet vs entry floor: %s" % lows)
        local_ok = []
        for n in crew:
            ll = self.interior.get_actor_transform().inverse_transform_location(n.get_actor_location())
            local_ok.append(abs(ll.x) < 20000 and abs(ll.y) < 20000)
        check("everyone stays aboard", all(local_ok))
        cells = set()
        for n in crew:
            cells |= self.track[n.get_name()]["cells"]
        check("crew spreads over the deck", len(cells) >= 6, "%d 2 m cells visited" % len(cells))
        if self.anim_checked:
            ratio = [a / max(1.0, v) for v, a in self.anim_checked]
            check("walkers animate at their speed", all(0.8 < r < 1.2 for r in ratio),
                  "anim/velocity %.2f..%.2f over %d samples" % (min(ratio), max(ratio), len(ratio)))
        else:
            check("walkers animate at their speed", False, "no walking samples")


names = [a for a in sys.argv[1:] if not a.startswith("--")]
Runner(names[0] if names else None)
