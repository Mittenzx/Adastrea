"""In-engine screenshots of a ship interior for Tools/interior_benchmark.py (run inside the editor while PIE is up).

    py "C:/Users/akuma/Adastrea/Tools/pie_capture_interior.py" [Ship] [--views a,b]

Spawns the ship blueprint far from everything, boards it (so ASpaceshipInterior
mounts the deck, spawns its socket lights and fixtures exactly as in play), then
puts a camera on each eye-height view of render_battleship_decks.VIEWS and takes a
HighResShot. Shots land in Saved/Screenshots/WindowsEditor and are copied to
Saved/InteriorBenchmark/<prefix>_ue_<view>.png. Log lines start with "INTCAP".
"""
import glob
import math
import os
import shutil
import sys

import unreal

ROOT = "C:/Users/akuma/Adastrea"
SHOTS = ROOT + "/Saved/Screenshots/WindowsEditor"
OUT = ROOT + "/Saved/InteriorBenchmark"
RES = "1400x860"
SETTLE_S = 3.0      # Lumen/exposure settle after a camera cut
SHOT_TIMEOUT_S = 20.0

SHIPS = {
    "Battleship": ("/Game/Blueprints/Ships/BP_Battleship", "SM_Int_Battleship_Decks"),
}
# Same eye-height poses as the Blender previews (Tools/render_battleship_decks.py):
# (tag, eye, look-at, blender lens mm on a 36 mm sensor) in BLENDER design cm;
# pose() flips y into UE interior space.
VIEWS = [
    ("spine_fwd", (-1000, 0, 165), (1600, 0, 150), 16),
    ("spine_aft", (1450, 30, 165), (-1100, -20, 150), 16),
    ("cic_spawn", (2270, 0, 165), (1600, 0, 120), 16),
    ("cic_fwd", (1700, -300, 180), (2800, 100, 200), 16),
    ("hangar_catwalk", (-1250, -250, 620), (-2500, 300, 200), 14),
    ("hangar_floor", (-1480, -400, 165), (-2700, 500, 350), 14),
    ("eng_floor", (-3250, -600, 165), (-3900, 300, 500), 14),
    ("eng_gallery", (-4200, 600, 570), (-3500, -400, 300), 14),
    ("berths", (-1050, 250, 165), (-250, 480, 130), 16),
    ("ready_room", (-250, -250, 165), (-1050, -600, 140), 16),
    ("mess", (500, 180, 165), (-185, 600, 110), 16),
    ("medbay", (-150, -200, 165), (650, -600, 90), 16),
    ("briefing", (760, 200, 170), (1585, 450, 150), 16),
    ("armory", (1500, -200, 170), (800, -600, 130), 16),
]


def log(msg):
    unreal.log("INTCAP " + msg)


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def pc():
    return unreal.GameplayStatics.get_player_controller(world(), 0)


def cmd(c):
    unreal.SystemLibrary.execute_console_command(world(), c)


class Capture:
    def __init__(self, ship, only):
        self.bp, self.prefix = SHIPS[ship]
        self.views = [v for v in VIEWS if not only or v[0] in only]
        self.stage = "spawn"
        self.t = 0.0
        self.frames = 0
        self.ship = self.cam = None
        os.makedirs(OUT, exist_ok=True)
        self.h = unreal.register_slate_post_tick_callback(self.tick)

    def done(self, note):
        unreal.unregister_slate_post_tick_callback(self.h)
        # Leave nothing behind: a leftover interior sits at the same parking spot
        # and overlaps the next run's deck (a yawed one shows up as wrong rooms).
        try:
            if self.ship and pc().get_controlled_pawn() != self.ship:
                pc().exit_ship_interior(self.ship)
        except Exception:
            pass
        for a in (self.cam, getattr(self, "interior", None), self.ship):
            try:
                if a:
                    a.destroy_actor()
            except Exception:
                pass
        log("DONE " + note)

    def tick(self, dt):
        try:
            self._tick(dt)
        except Exception:
            import traceback
            log("EXCEPTION " + traceback.format_exc())
            self.done("exception")

    def summon(self, cls_path):
        w = world()
        cls = unreal.load_object(None, cls_path)
        before = set(a.get_name() for a in unreal.GameplayStatics.get_all_actors_of_class(w, cls))
        cmd("EnableCheats")
        cmd("summon " + cls_path)
        return cls, before

    def _tick(self, dt):
        if world() is None:
            log("no PIE world - start PIE first")
            unreal.unregister_slate_post_tick_callback(self.h)
            return
        self.t += dt
        self.frames += 1
        if self.stage == "spawn":
            p = pc()
            p.un_possess()
            p.set_actor_location(unreal.Vector(400000, -300000, 80000), False, True)
            self.cls, self.before = self.summon("%s.%s_C" % (self.bp, self.bp.rsplit("/", 1)[1]))
            self.stage, self.frames = "spawned", 0
        elif self.stage == "spawned":
            new = [a for a in unreal.GameplayStatics.get_all_actors_of_class(world(), self.cls)
                   if a.get_name() not in self.before]
            if not new:
                if self.frames > 30:
                    self.done("summon spawned nothing")
                return
            self.ship = new[0]
            # summon faces the controller's rotation, which persists between runs; a
            # yawed ship left the deck meshes rendering 180 deg from the component
            # transform, so pin the ship to yaw 0 before boarding
            self.ship.set_actor_location_and_rotation(unreal.Vector(400000, -300000, 50000),
                                                      unreal.Rotator(0, 0, 0), False, True)
            pc().possess(self.ship)
            self.stage, self.frames = "enter", 0
        elif self.stage == "enter":
            if self.frames == 20:
                pc().enter_ship_interior(self.ship)
            if self.frames >= 60:
                a = pc().get_controlled_pawn()
                it = a.get_editor_property("current_interior") if isinstance(a, unreal.SpaceshipAvatar) else None
                if it:
                    self.interior = it
                    self.camcls, self.cambefore = self.summon("/Script/Engine.CameraActor")
                    self.stage, self.frames = "cam", 0
                elif self.frames > 600:
                    self.done("never boarded")
        elif self.stage == "cam":
            new = [a for a in unreal.GameplayStatics.get_all_actors_of_class(world(), self.camcls)
                   if a.get_name() not in self.cambefore]
            if not new:
                if self.frames > 30:
                    self.done("camera summon failed")
                return
            self.cam = new[0]
            n = len(self.interior.get_components_by_class(unreal.PointLightComponent))
            log("boarded %s, %d point lights on the interior" % (self.interior.get_name(), n))
            self.vi = 0
            self.pose()
        elif self.stage == "settle":
            # re-assert the pose every tick: the pawn/camera manager can take the
            # view back (seen as random wall close-ups when this wasn't done)
            self.apply()
            if self.t >= SETTLE_S:
                cm = pc().player_camera_manager
                d = (cm.get_camera_location() - self.eye).length()
                if d > 5.0:
                    log("view %s not at the camera yet (%.0f cm off), waiting" % (self.views[self.vi][0], d))
                    self.t = SETTLE_S - 1.0
                    return
                self.before_shots = set(glob.glob(SHOTS + "/*.png"))
                cmd("HighResShot " + RES)
                self.stage, self.t = "shoot", 0.0
        elif self.stage == "shoot":
            new = sorted(set(glob.glob(SHOTS + "/*.png")) - self.before_shots, key=os.path.getmtime)
            if new and self.t > 0.5:
                tag = self.views[self.vi][0]
                dst = "%s/%s_ue_%s.png" % (OUT, self.prefix, tag)
                shutil.copyfile(new[-1], dst)
                log("shot %s -> %s" % (tag, dst))
                self.vi += 1
                self.pose()
            elif self.t > SHOT_TIMEOUT_S:
                log("shot %s timed out" % self.views[self.vi][0])
                self.vi += 1
                self.pose()

    def pose(self):
        if self.vi >= len(self.views):
            self.done("%d views" % len(self.views))
            return
        tag, eye, look, lens = self.views[self.vi]
        # The deck meshes don't always inherit the actor's yaw (seen: actor yawed
        # 180 deg, meshes not), so place cameras from the Shell component itself.
        shell = [c for c in self.interior.get_components_by_class(unreal.StaticMeshComponent)
                 if c.static_mesh and c.static_mesh.get_name() == self.prefix + "_Shell"][0]
        xf = unreal.Transform(shell.get_world_location(), shell.get_world_rotation(), unreal.Vector(1, 1, 1))
        e = xf.transform_location(unreal.Vector(eye[0], -eye[1], eye[2]))
        l = xf.transform_location(unreal.Vector(look[0], -look[1], look[2]))
        self.eye, self.rot = e, unreal.MathLibrary.find_look_at_rotation(e, l)
        self.fov = math.degrees(2 * math.atan(18.0 / lens))
        self.apply()
        self.stage, self.t = "settle", 0.0

    def apply(self):
        self.cam.set_actor_location_and_rotation(self.eye, self.rot, False, True)
        cc = self.cam.get_editor_property("camera_component")
        cc.set_editor_property("field_of_view", self.fov)
        cc.set_editor_property("constrain_aspect_ratio", False)
        if pc().get_view_target() != self.cam:
            pc().set_view_target_with_blend(self.cam, 0.0)


args = sys.argv[1:]
only = []
if "--views" in args:
    i = args.index("--views")
    only = args[i + 1].split(",")
    del args[i:i + 2]
Capture(args[0] if args else "Battleship", only)
