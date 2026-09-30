#!/usr/bin/env python3
"""
Quality gate for the generated sound library (Tools/audio/generate_sfx.py).

tests/test_audio_assets.py checks the technical basics (format, loudness, loop seams).
This checks the things that decide whether a sound holds up next to a commercial game,
as far as they can be measured without listening:

- small_speaker_loss: how much quieter the sound gets on a laptop speaker or cheap
  headset (loudness after a 150 Hz high-pass, vs full band). A big loss means the
  sound is mostly sub-bass and will vanish for most players.
- air: loudness of the 6-16 kHz band relative to the whole sound. Hits and UI
  sounds without it read as dull or "behind a blanket".
- cutoffs: places where a layer is cut off mid-ring instead of decaying. Found as a
  high-frequency click immediately followed by a drop in level (a deliberate second
  hit is a click followed by a rise).
- loop_repetition (loops only, informational): how strongly the loudness envelope
  repeats within the loop. High values make short loops easy to spot.

Each sound gets a profile (PROFILES, first matching rule wins) with the limits it must meet.

Usage:
    python Tools/audio/audio_qa.py                      # table + pass/fail for every sound
    python Tools/audio/audio_qa.py Engine Dock          # only IDs starting with these prefixes
    python Tools/audio/audio_qa.py --baseline origin/main Engine.Capital
        # also compares against the WAVs committed at that git ref, and writes
        # before/after spectrogram sheets to Saved/AudioQA/
    python Tools/audio/audio_qa.py --sheet Engine       # spectrogram sheets without a baseline

Exit code is 1 if any sound fails its profile.
"""

from __future__ import annotations

import argparse
import io
import json
import subprocess
import sys
from pathlib import Path

import numpy as np
from scipy import signal
from scipy.io import wavfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
import generate_sfx as gen  # noqa: E402

SR = gen.SR
REPO = gen.REPO_ROOT
GEN_DIR = gen.OUT_DIR
REPORT_DIR = REPO / "Saved" / "AudioQA"

# ---------------------------------------------------------------------------
# Profiles: (id prefix or exact id, limits). First match wins, so specific rules go first.
#   max_small_speaker_loss_db: every sound must survive a small speaker.
#   min_air_db: None = no requirement (deliberately dark sounds: heavy engines, hums, beds).
# ---------------------------------------------------------------------------
SMALL_SPEAKER_LOSS_DB = 6.0
AIR_HITS_DB = -22.0     # impacts and mechanical one-shots
AIR_UI_DB = -35.0       # UI, trade, editor and console tones
AIR_BRIGHT_DB = -25.0   # light-ship engine

PROFILES = [
    ("Engine.Light.", dict(min_air_db=AIR_BRIGHT_DB)),
    ("Engine.", dict(min_air_db=None)),
    ("Thruster.HeavyGroan", dict(min_air_db=None)),
    ("Thruster.", dict(min_air_db=AIR_HITS_DB)),
    ("Flight.CollisionBump", dict(min_air_db=AIR_HITS_DB)),
    ("Flight.", dict(min_air_db=AIR_UI_DB)),
    ("Dock.Beacon", dict(min_air_db=AIR_UI_DB)),
    ("Dock.", dict(min_air_db=AIR_HITS_DB)),
    ("Mining.LaserLoop", dict(min_air_db=AIR_UI_DB)),
    ("Mining.AsteroidDepleted", dict(min_air_db=AIR_HITS_DB)),
    ("Mining.", dict(min_air_db=AIR_UI_DB)),
    ("Editor.Place.", dict(min_air_db=AIR_HITS_DB)),
    ("Interior.Footstep.", dict(min_air_db=AIR_HITS_DB)),
    ("Interior.Door", dict(min_air_db=AIR_HITS_DB)),
    ("Interior.ShipHum", dict(min_air_db=None)),
    ("Ambient.", dict(min_air_db=None)),
    ("", dict(min_air_db=AIR_UI_DB)),  # Trade, Editor, Interior tones, UI
]


def profile_for(event_id: str) -> dict:
    for prefix, limits in PROFILES:
        if event_id.startswith(prefix):
            return dict(max_small_speaker_loss_db=SMALL_SPEAKER_LOSS_DB, max_cutoffs=0, **limits)
    raise AssertionError("PROFILES has a catch-all rule")


# ---------------------------------------------------------------------------
# Metrics
# ---------------------------------------------------------------------------

_HP150 = signal.butter(4, 150 / (SR / 2), "high", output="sos")
_AIR = signal.butter(4, [6000 / (SR / 2), 16000 / (SR / 2)], "band", output="sos")
_HF4K = signal.butter(4, 4000 / (SR / 2), "high", output="sos")
_LP1K = signal.butter(2, 1000 / (SR / 2), "low", output="sos")


def _frames_db(x, w):
    x = x[: x.size // w * w]
    return 10 * np.log10(np.mean(x.reshape(-1, w) ** 2, axis=1) + 1e-14)


def find_cutoffs(x: np.ndarray) -> list[float]:
    """Times (s) where a layer is cut off: a >= 12 dB high-frequency spike in a 2 ms frame,
    while the sound is within 50 dB of its peak, followed by a >= 3 dB drop in level."""
    w = int(0.002 * SR)
    hf = _frames_db(signal.sosfilt(_HF4K, x), w)
    lo = _frames_db(signal.sosfilt(_LP1K, x), w)
    full = _frames_db(x, w)
    if hf.size < 21:
        return []
    top = lo.max()
    hits = []
    for k in range(10, hf.size - 10):
        # A click: a sudden jump in high frequencies vs the previous frame and the recent median.
        if hf[k] - np.median(hf[k - 10:k]) <= 12 or hf[k] - hf[k - 1] < 20:
            continue
        # A new hit (a note, a crackle) peaks at the click frame and then decays. A cut
        # clicks while the level is already falling below where it just was.
        if full[k] >= full[k - 5:k].max() - 1.0:
            continue
        before = lo[k - 5:k].mean()
        if before > top - 50 and lo[k + 1:k + 6].mean() < before - 3 and full[k + 1:k + 4].mean() < full[k - 3:k].mean() - 4:
            t = round(k * w / SR, 3)
            if not hits or t - hits[-1] > 0.02:
                hits.append(t)
    return hits


def loop_repetition(x: np.ndarray) -> float:
    """Peak autocorrelation of the 10 ms loudness envelope at lags >= 50 ms (0 = no repetition, 1 = exact)."""
    w = int(0.01 * SR)
    e = np.abs(x[: x.size // w * w]).reshape(-1, w).mean(axis=1)
    e = e - e.mean()
    if e.size < 10:
        return 0.0
    ac = np.correlate(e, e, "full")[e.size:] / (np.sum(e * e) + 1e-12)
    return float(ac[4:].max())


def analyse(x: np.ndarray, loop: bool) -> dict:
    full = gen.integrated_loudness(x)
    return dict(
        lufs=round(full, 1),
        peak_dbfs=round(gen.peak_dbfs(x), 2),
        small_speaker_loss_db=round(full - gen.integrated_loudness(signal.sosfilt(_HP150, x)), 1),
        air_db=round(max(gen.integrated_loudness(signal.sosfilt(_AIR, x)) - full, -99.0), 1),
        centroid_hz=round(gen.spectral_centroid(x)),
        cutoffs=[] if loop else find_cutoffs(x),
        loop_repetition=round(loop_repetition(x), 2) if loop else None,
    )


def check(event_id: str, m: dict) -> list[str]:
    """Returns the ways a sound misses its profile (empty = pass)."""
    p = profile_for(event_id)
    fails = []
    if m["small_speaker_loss_db"] > p["max_small_speaker_loss_db"]:
        fails.append(f"small-speaker loss {m['small_speaker_loss_db']} dB > {p['max_small_speaker_loss_db']}")
    if p["min_air_db"] is not None and m["air_db"] < p["min_air_db"]:
        fails.append(f"air {m['air_db']} dB < {p['min_air_db']}")
    if len(m["cutoffs"]) > p["max_cutoffs"]:
        fails.append(f"cut off at {m['cutoffs']} s")
    return fails


# ---------------------------------------------------------------------------
# Loading (working tree or a git ref)
# ---------------------------------------------------------------------------

def load_manifest(ref: str | None = None) -> dict:
    if ref is None:
        return json.loads(gen.MANIFEST_PATH.read_text())
    return json.loads(_git_show(ref, gen.MANIFEST_PATH))


def load_wav(entry: dict, ref: str | None = None) -> np.ndarray:
    path = GEN_DIR / entry["file"]
    src = io.BytesIO(_git_show(ref, path, binary=True)) if ref else str(path)
    _, pcm = wavfile.read(src)
    return pcm.astype(np.float64) / 32767.0


def _git_show(ref, path: Path, binary=False):
    rel = path.relative_to(REPO).as_posix()
    out = subprocess.run(["git", "show", f"{ref}:{rel}"], cwd=REPO, capture_output=True, check=True).stdout
    return out if binary else out.decode("utf-8")


# ---------------------------------------------------------------------------
# Spectrogram sheets
# ---------------------------------------------------------------------------

def write_sheet(path: Path, rows: list[tuple[str, np.ndarray | None, np.ndarray]]):
    """rows: (label, baseline or None, current). One row per sound, before | after when a baseline is given."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    cols = 2 if any(b is not None for _, b, _ in rows) else 1
    fig, axs = plt.subplots(len(rows), cols, figsize=(6.5 * cols, 1.9 * len(rows)), squeeze=False)
    for r, (label, before, after) in enumerate(rows):
        for c, (x, tag) in enumerate([(before, "before"), (after, "after")][2 - cols:]):
            ax = axs[r][c]
            if x is None:
                ax.axis("off")
                continue
            f, t, S = signal.spectrogram(x, SR, nperseg=2048, noverlap=1792)
            ax.pcolormesh(t, f, 10 * np.log10(S + 1e-12), vmin=-110, vmax=-40, shading="auto", cmap="magma")
            ax.set_yscale("symlog", linthresh=100)
            ax.set_ylim(20, 20000)
            ax.set_title(f"{label}  ({tag})" if cols == 2 else label, fontsize=8, loc="left")
            ax.tick_params(labelsize=6)
    plt.tight_layout()
    path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(path, dpi=70)
    plt.close(fig)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("prefixes", nargs="*", help="only event IDs starting with these")
    ap.add_argument("--baseline", help="git ref to compare against (e.g. origin/main)")
    ap.add_argument("--sheet", action="store_true", help="write spectrogram sheets even without a baseline")
    ap.add_argument("--json", action="store_true", help="also write Saved/AudioQA/report.json")
    args = ap.parse_args(argv)

    cur = load_manifest()
    base = {s["id"]: s for s in load_manifest(args.baseline)["sounds"]} if args.baseline else {}
    entries = [s for s in cur["sounds"] if not args.prefixes or any(s["id"].startswith(p) for p in args.prefixes)]

    report, rows, failed = [], [], 0
    hdr = f"{'id':26s} {'spk loss':>8s} {'air':>6s} {'cent':>6s} {'rep':>5s}  result"
    print(hdr if not base else hdr + "   (baseline spk loss / air)")
    for e in entries:
        x = load_wav(e)
        m = analyse(x, e["loop"])
        fails = check(e["id"], m)
        failed += bool(fails)
        line = (f"{e['id']:26s} {m['small_speaker_loss_db']:8.1f} {m['air_db']:6.1f} {m['centroid_hz']:6d} "
                f"{'' if m['loop_repetition'] is None else m['loop_repetition']:>5}  "
                f"{'FAIL: ' + '; '.join(fails) if fails else 'ok'}")
        bx = None
        if e["id"] in base:
            bx = load_wav(base[e["id"]], args.baseline)
            bm = analyse(bx, e["loop"])
            line += f"   (was {bm['small_speaker_loss_db']} / {bm['air_db']}{', cut' if bm['cutoffs'] else ''})"
        print(line)
        report.append(dict(id=e["id"], metrics=m, fails=fails))
        rows.append((e["id"], bx, x))

    if args.baseline or args.sheet:
        for i in range(0, len(rows), 8):
            out = REPORT_DIR / f"sheet_{i // 8 + 1:02d}.png"
            write_sheet(out, rows[i:i + 8])
            print(f"sheet -> {out}")
    if args.json:
        REPORT_DIR.mkdir(parents=True, exist_ok=True)
        (REPORT_DIR / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"\n{len(entries) - failed}/{len(entries)} pass")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
