#!/usr/bin/env python3
"""UserPromptSubmit hook: queue likely corrections/preferences for /reflect.

Capture only; never blocks or modifies the prompt. Appends JSONL to
.claude/learnings-queue.jsonl (gitignored).
"""
import json, re, sys, time
from pathlib import Path

PATTERNS = [
    r"^\s*(no|nope|wrong|incorrect)\b[,.!: ]",
    r"\bactually[, ]",
    r"\b(use|prefer)\b .{1,60}\b(not|instead of|rather than)\b",
    r"\bdon'?t (use|do|add|make|touch|commit|run)\b",
    r"\b(stop|never|always)\b .{1,80}",
    r"\bthat'?s (not|wrong)\b",
    r"\bremember (that|to)\b",
    r"\bfrom now on\b",
    r"\bi (prefer|want you to)\b",
]

def main():
    try:
        data = json.load(sys.stdin)
    except Exception:
        return
    prompt = (data.get("prompt") or "").strip()
    if not prompt or prompt.startswith("/") or len(prompt) > 600:
        return
    if not any(re.search(p, prompt, re.I) for p in PATTERNS):
        return
    root = Path(data.get("cwd") or ".")
    q = root / ".claude" / "learnings-queue.jsonl"
    q.parent.mkdir(parents=True, exist_ok=True)
    with q.open("a", encoding="utf-8") as f:
        f.write(json.dumps({"ts": int(time.time()), "session": data.get("session_id"),
                            "text": prompt}) + "\n")

if __name__ == "__main__":
    try:
        main()
    except Exception:
        pass
