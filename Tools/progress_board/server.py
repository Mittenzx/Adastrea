"""Adastrea progress board: a live, expandable view of what's been built.

Reads git history on request, sorts every commit into the system tree in
taxonomy.json (by commit scope, files touched and subject keywords), and
serves index.html, which polls /api/data so new commits show up on their own.

    python Tools/progress_board/server.py [--port 8765]

Then open http://localhost:8765. Stdlib only.
"""

import argparse
import fnmatch
import hashlib
import json
import os
import re
import subprocess
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
TAXONOMY = HERE / "taxonomy.json"
INDEX = HERE / "index.html"
GITHUB = "https://github.com/Mittenzx/Adastrea"

SCOPE_RE = re.compile(r"^[a-z]+\(([^)]+)\)!?:", re.I)
PR_RE = re.compile(r"Merge pull request #(\d+) from [^/\s]+/(\S+)")
# Commits that touch this many files are bulk asset/resave passes; their file
# lists would light up half the tree, so only their scope and subject count.
BULK_FILES = 250


def git(*args):
    # Under pythonw (the always-on scheduled task) every console child would
    # flash its own window on each poll; CREATE_NO_WINDOW keeps git hidden.
    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    out = subprocess.run(["git", "-C", str(REPO), *args], capture_output=True, creationflags=flags)
    return out.stdout.decode("utf-8", "replace")


def ref_state():
    refs = git("for-each-ref", "--format=%(refname) %(objectname)", "refs/heads", "refs/remotes")
    return hashlib.sha1(refs.encode()).hexdigest()


def main_ref():
    for ref in ("origin/main", "main"):
        if git("rev-parse", "--verify", "--quiet", ref).strip():
            return ref
    return "HEAD"


# ---------------------------------------------------------------- taxonomy

class Rule:
    def __init__(self, node):
        self.scopes = {s.lower() for s in node.get("scopes", [])}
        self.paths = [p.lower() for p in node.get("paths", [])]
        # pathsOnly: the files rule needs every changed file to match, so a
        # feature commit that also touches the roadmap isn't counted as docs.
        self.only = node.get("pathsOnly", False)
        words = node.get("words", [])
        self.words = re.compile(r"\b(?:" + "|".join(re.escape(w) for w in words) + r")\b", re.I) if words else None

    def matches(self, scopes, subject, files):
        if self.scopes & scopes:
            return True
        if self.words and self.words.search(subject):
            return True
        if not files or not self.paths:
            return False
        hit = [any(self.path_hit(r, f) for r in self.paths) for f in files]
        return all(hit) if self.only else any(hit)

    @staticmethod
    def path_hit(rule, f):
        return fnmatch.fnmatchcase(f, "*" + rule + "*") if "*" in rule else rule in f


def flatten(nodes, parent=None, out=None):
    out = [] if out is None else out
    for n in nodes:
        out.append((n, parent))
        flatten(n.get("children", []), n["id"], out)
    return out


# ---------------------------------------------------------------- git model

def load_graph(ref):
    """PR number/branch for each commit on main, and in-flight branch labels."""
    parents = {}
    subjects = {}
    for line in git("log", "--branches", "--remotes", "--pretty=format:%H%x09%P%x09%s").splitlines():
        h, p, s = (line.split("\t", 2) + ["", ""])[:3]
        parents[h] = p.split()
        subjects[h] = s

    first_parent = git("rev-list", "--first-parent", "--reverse", ref).split()
    on_main = set()
    pr_of = {}

    def absorb(start, label):
        stack = [start]
        while stack:
            c = stack.pop()
            if c in on_main or c not in parents:
                continue
            on_main.add(c)
            if label:
                pr_of.setdefault(c, label)
            stack.extend(parents[c])

    for c in first_parent:
        ps = parents.get(c, [])
        on_main.add(c)
        m = PR_RE.search(subjects.get(c, ""))
        if m and len(ps) > 1:
            label = {"pr": int(m.group(1)), "branch": m.group(2)}
            pr_of[c] = label
            for p in ps[1:]:
                absorb(p, label)
        else:
            for p in ps[1:]:
                absorb(p, None)

    # In-flight work: commits on a branch that main doesn't have yet.
    in_flight = {}
    for line in git("for-each-ref", "--format=%(refname:short) %(objectname)", "refs/heads", "refs/remotes").splitlines():
        name, tip = line.rsplit(" ", 1)
        if name.endswith("/HEAD") or name in ("main", "origin/main", "origin"):
            continue
        short = name.split("/", 1)[1] if name.startswith("origin/") else name
        stack, seen = [tip], set()
        while stack:
            c = stack.pop()
            if c in on_main or c in seen or c not in parents:
                continue
            seen.add(c)
            in_flight.setdefault(c, short)
            stack.extend(parents[c])
    return pr_of, in_flight


def build():
    tax = json.loads(TAXONOMY.read_text(encoding="utf-8"))
    flat = flatten(tax["nodes"])
    rules = [(n["id"], Rule(n)) for n, _ in flat]
    parent_of = {n["id"]: p for n, p in flat}

    ref = main_ref()
    pr_of, in_flight = load_graph(ref)

    raw = git("log", "--branches", "--remotes", "--no-merges", "--name-only",
              "--pretty=format:\x1e%H\x1f%aI\x1f%an\x1f%s")
    commits = {}
    node_commits = {n["id"]: set() for n, _ in flat}
    unsorted = []
    for block in raw.split("\x1e"):
        if not block.strip():
            continue
        head, _, rest = block.partition("\n")
        parts = head.split("\x1f")
        if len(parts) < 4:
            continue
        sha, date, author, subject = parts
        files = [f.strip().lower() for f in rest.splitlines() if f.strip()]
        if not files:  # empty bot commits ("Initial plan")
            continue
        m = SCOPE_RE.match(subject)
        scopes = {s.strip().lower() for s in m.group(1).split(",")} if m else set()
        path_files = files if len(files) < BULK_FILES else []

        hits = [nid for nid, rule in rules if rule.matches(scopes, subject, path_files)]
        if not hits:
            unsorted.append(sha)
        for nid in hits:
            while nid:  # roll up to every ancestor
                node_commits[nid].add(sha)
                nid = parent_of[nid]

        c = {"d": date, "s": subject, "a": author, "f": len(files), "n": hits}
        if sha in pr_of:
            c["pr"] = pr_of[sha]["pr"]
            c["b"] = pr_of[sha]["branch"]
        elif sha in in_flight:
            c["wip"] = in_flight[sha]
        commits[sha] = c

    def by_date(shas):
        return sorted(shas, key=lambda s: commits[s]["d"], reverse=True)

    for n, _ in flat:
        n["commits"] = by_date(node_commits[n["id"]])
        for k in ("scopes", "paths", "words", "pathsOnly"):
            n.pop(k, None)

    return {
        "generated": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "main": ref,
        "head": git("rev-parse", "--abbrev-ref", "HEAD").strip(),
        "github": GITHUB,
        "nodes": tax["nodes"],
        "unsorted": by_date(unsorted),
        "commits": commits,
    }


# ---------------------------------------------------------------- server

class Cache:
    def __init__(self):
        self.lock = threading.Lock()
        self.key = None
        self.body = None
        self.etag = None

    def get(self):
        key = ref_state() + str(TAXONOMY.stat().st_mtime_ns)
        with self.lock:
            if key != self.key:
                t = time.time()
                self.body = json.dumps(build(), separators=(",", ":")).encode()
                self.etag = hashlib.sha1(self.body).hexdigest()[:16]
                self.key = key
                print(f"rebuilt board data in {time.time() - t:.1f}s ({len(self.body) // 1024} KB)")
            return self.body, self.etag


CACHE = Cache()
TODO_LOCK = threading.Lock()


def edit_todo(req):
    """add / toggle / edit / delete one to-do item, written back to taxonomy.json."""
    with TODO_LOCK:
        tax = json.loads(TAXONOMY.read_text(encoding="utf-8"))
        node = next((n for n, _ in flatten(tax["nodes"]) if n["id"] == req.get("id")), None)
        if node is None:
            raise ValueError("unknown node " + str(req.get("id")))
        todo = node.setdefault("todo", [])
        op, i = req.get("op"), req.get("index")
        text = str(req.get("text", "")).strip()[:300]
        if op == "add" and text:
            todo.append({"text": text, "done": False})
        elif op in ("toggle", "edit", "delete"):
            if not isinstance(i, int) or not 0 <= i < len(todo):
                raise ValueError("bad index")
            if op == "toggle":
                todo[i]["done"] = not todo[i].get("done")
                if todo[i]["done"]:
                    todo[i]["doneAt"] = time.strftime("%Y-%m-%d")
                else:
                    todo[i].pop("doneAt", None)
            elif op == "edit" and text:
                todo[i]["text"] = text
            elif op == "delete":
                todo.pop(i)
        else:
            raise ValueError("bad op")
        tmp = TAXONOMY.with_suffix(".tmp")
        tmp.write_text(json.dumps(tax, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        os.replace(tmp, TAXONOMY)


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        path = self.path.split("?")[0]
        if path in ("/", "/index.html"):
            self.send(200, INDEX.read_bytes(), "text/html; charset=utf-8")
        elif path == "/api/data":
            try:
                body, etag = CACHE.get()
            except Exception as e:  # keep serving; the page shows the error
                self.send(500, json.dumps({"error": str(e)}).encode(), "application/json")
                return
            if self.headers.get("If-None-Match") == etag:
                self.send(304, b"", None, etag)
            else:
                self.send(200, body, "application/json", etag)
        else:
            self.send(404, b"not found", "text/plain")

    def do_POST(self):
        if self.path.split("?")[0] != "/api/todo":
            self.send(404, b"not found", "text/plain")
            return
        try:
            req = json.loads(self.rfile.read(int(self.headers.get("Content-Length", 0))) or b"{}")
            edit_todo(req)
            body, etag = CACHE.get()
            self.send(200, body, "application/json", etag)
        except Exception as e:
            self.send(400, json.dumps({"error": str(e)}).encode(), "application/json")

    def send(self, code, body, ctype, etag=None):
        self.send_response(code)
        if ctype:
            self.send_header("Content-Type", ctype)
        if etag:
            self.send_header("ETag", etag)
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *args):
        pass


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=8765)
    ap.add_argument("--dump", action="store_true", help="print a summary and exit")
    args = ap.parse_args()
    if args.dump:
        data = build()
        for n, depth in walk(data["nodes"]):
            print("  " * depth + f"{n['name']}: {len(n['commits'])}")
        print(f"unsorted: {len(data['unsorted'])} of {len(data['commits'])}")
        for s in data["unsorted"][:40]:
            print("   ", data["commits"][s]["s"])
        return
    CACHE.get()
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"Adastrea progress board on http://localhost:{args.port}")
    server.serve_forever()


def walk(nodes, depth=0):
    for n in nodes:
        yield n, depth
        yield from walk(n.get("children", []), depth + 1)


if __name__ == "__main__":
    main()
