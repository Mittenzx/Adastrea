"""Drive PIE from the shell through UClaudeDriverSubsystem (run with the system Python, not in the editor).

    python Tools/claude_play.py pie start                 start PIE in the editor's current map
    python Tools/claude_play.py run "fly hostile 800; attack hostile 60"
    python Tools/claude_play.py wait [timeout_s]          poll until the queue is done, print the results
    python Tools/claude_play.py state                     print the JSON state snapshot
    python Tools/claude_play.py cancel
    python Tools/claude_play.py pie stop

Options (before the verb): --root <project dir> picks the editor by project_root (default: the
checkout this script is in); --pid <editor pid> picks one editor when several share that root.

Each call is one short remote-exec statement, so nothing keeps running in the editor's Python
(no slate tick callbacks to leak). The commands themselves run in C++ every frame.
"""
import json
import os
import sys
import time

sys.path.insert(0, r"C:\Program Files\Epic Games\UE_5.8\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python")
import remote_execution as rexlib  # noqa: E402

DEFAULT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Runs inside the editor. Gets the PIE world's driver subsystem as `d` (None outside PIE).
PRELUDE = (
    "import unreal, json\n"
    "w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()\n"
    "d = unreal.ClaudeDriverSubsystem.get(w) if w else None\n"
)


def norm(path):
    return path.replace("\\", "/").rstrip("/").lower()


class Editor:
    def __init__(self, root, pid=None):
        self.rex = rexlib.RemoteExecution()
        self.rex.start()
        deadline = time.time() + 20.0
        nodes = []
        while time.time() < deadline:
            nodes = [n for n in self.rex.remote_nodes if norm(n.get("project_root", "")) == norm(root)]
            if nodes:
                break
            time.sleep(0.2)
        if not nodes:
            self.rex.stop()
            sys.exit("no editor found with project_root %s" % root)
        for node in nodes:
            self.rex.open_command_connection(node["node_id"])
            if pid is None or self.exec_stmt("print(__import__('os').getpid())").strip() == str(pid):
                return
            self.rex.close_command_connection()
        self.rex.stop()
        sys.exit("no editor with pid %s under %s" % (pid, root))

    def exec_stmt(self, code):
        """Run code (wrapped into one statement) and return what it printed."""
        result = self.rex.run_command("exec(%r)" % code, unattended=True, exec_mode=rexlib.MODE_EXEC_STATEMENT)
        if not result.get("success"):
            raise RuntimeError(result.get("result"))
        return "\n".join(o["output"] for o in result.get("output", []) if o.get("type") == "Info")

    def close(self):
        self.rex.stop()


def driver_call(ed, body):
    out = ed.exec_stmt(PRELUDE + "if d is None:\n    print('NO_PIE')\nelse:\n" + "".join("    %s\n" % line for line in body.splitlines()))
    if out.strip() == "NO_PIE":
        sys.exit("PIE isn't running (python Tools/claude_play.py pie start)")
    return out


def get_state(ed):
    return json.loads(driver_call(ed, "print(d.get_state_json())"))


def main(argv):
    root, pid = DEFAULT_ROOT, None
    while argv and argv[0].startswith("--"):
        flag, value, argv = argv[0], argv[1], argv[2:]
        if flag == "--root":
            root = value
        elif flag == "--pid":
            pid = int(value)
    if not argv:
        sys.exit(__doc__)
    verb, args = argv[0], argv[1:]

    ed = Editor(root, pid)
    try:
        if verb == "pie":
            sub = "unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)"
            call = "editor_request_begin_play" if args and args[0] == "start" else "editor_request_end_play"
            ed.exec_stmt("import unreal\n%s.%s()" % (sub, call))
            print("PIE %s requested" % ("start" if call.endswith("begin_play") else "stop"))
        elif verb == "run":
            driver_call(ed, "d.run(%r)" % " ".join(args))
            print("queued")
        elif verb == "cancel":
            driver_call(ed, "d.cancel()")
            print("cancelled")
        elif verb == "state":
            print(json.dumps(get_state(ed), indent=1))
        elif verb == "wait":
            timeout = float(args[0]) if args else 300.0
            state = get_state(ed)
            # Results finished since the last command started (the queue may have started before this call).
            started = state["time"] - state.get("current", {}).get("elapsed", 0.0)
            deadline = time.time() + timeout
            while state["busy"] and time.time() < deadline:
                time.sleep(1.0)
                state = get_state(ed)
            for result in state["history"]:
                if result["t"] < started:
                    continue
                print("%s %-28s %s" % ("ok  " if result["ok"] else "FAIL", result["cmd"], result["note"]))
            if state["busy"]:
                print("still busy after %.0f s: %s" % (timeout, state.get("current", {}).get("cmd")))
            ship = state.get("ship")
            if ship:
                print("ship: %s hull %s/%s shield %s speed %s m/s" % (ship["name"], ship["hull"], ship["hull_max"], ship.get("shield"), ship["speed_mps"]))
            log = state.get("log")
            if log:
                print("log since PIE start: %d errors, %d warnings" % (log["errors"], log["warnings"]))
                for line in log["recent"]:
                    print("  " + line)
        else:
            sys.exit("unknown verb %s\n%s" % (verb, __doc__))
    finally:
        ed.close()


if __name__ == "__main__":
    main(sys.argv[1:])
