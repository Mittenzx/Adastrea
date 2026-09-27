"""
Analysis tests for the generated sound library (Tools/audio/generate_sfx.py).

Checks every WAV listed in Assets/Audio/generated/audio_manifest.json:
- format: 48 kHz, 16-bit, mono
- peak at or below -1 dBFS
- loudness within +/-2 LU of the manifest target. Loudness is measured with
  generate_sfx.integrated_loudness: BS.1770-style K-weighted, gated integrated
  loudness (400 ms blocks; 100 ms blocks for sounds shorter than 1 s). It is
  not a certified meter, but it is real K-weighted loudness, not plain RMS.
- no DC offset
- sensible durations (one-shots 0.03-4 s, loops 4-16 s)
- loops: no click at the seam (the wrap-around step is no bigger than the
  largest normal sample step) and matching level either side of the seam
- engine families: spectral centroid Light > Medium > Heavy > Capital
- manifest and files agree both ways; group members exist
- the generator is deterministic (re-rendering a sound gives the same samples)

Requires numpy and scipy (skipped otherwise).
"""

import json
import sys
from pathlib import Path

import pytest

np = pytest.importorskip("numpy")
pytest.importorskip("scipy")
from scipy.io import wavfile  # noqa: E402

REPO = Path(__file__).resolve().parents[1]
GEN_DIR = REPO / "Assets" / "Audio" / "generated"
MANIFEST = GEN_DIR / "audio_manifest.json"
sys.path.insert(0, str(REPO / "Tools" / "audio"))
import generate_sfx as gen  # noqa: E402

LUFS_TOLERANCE = 2.0
DC_LIMIT = 1e-3
ENGINE_FAMILY_ORDER = ["Light", "Medium", "Heavy", "Capital"]

REQUIRED_IDS = [
    *[f"Engine.{f}.{r}" for f in ENGINE_FAMILY_ORDER for r in ("Low", "High")],
    "Engine.Whine", "Engine.Boost", "Engine.BoostStart", "Engine.SpoolUp", "Engine.SpoolDown", "Engine.Idle",
    "Thruster.Puff", "Thruster.HeavyGroan", "Flight.CollisionBump", "Flight.SpeedWarning",
    "Dock.Beacon", "Dock.ClampEngage", "Dock.AirlockHiss", "Dock.Release",
    "Mining.LaserLoop", "Mining.OreTick", "Mining.CargoFull", "Mining.AsteroidDepleted",
    "Trade.Buy", "Trade.Sell", "Trade.CreditsDing", "Trade.Denied",
    "Editor.Place.Small", "Editor.Place.Large", "Editor.Remove", "Editor.Invalid", "Editor.Undo", "Editor.Redo",
    "Editor.Rotate", "Editor.Save",
    *[f"Interior.Footstep.{i:02d}" for i in range(1, 5)],
    "Interior.Door", "Interior.CockpitEnter", "Interior.CockpitExit", "Interior.ShipHum", "Interior.ConsoleChirp",
    "UI.Hover", "UI.Click", "UI.Open", "UI.Close", "UI.Toast", "UI.QuickSave", "UI.QuickLoad", "UI.Error",
    "Ambient.Space", "Ambient.StationHum", "Ambient.AsteroidCreak", "Ambient.MapRoomTone",
]
REQUIRED_LOOPS = [f"Engine.{f}.{r}" for f in ENGINE_FAMILY_ORDER for r in ("Low", "High")] + [
    "Engine.Whine", "Engine.Boost", "Engine.Idle", "Mining.LaserLoop", "Interior.ShipHum",
    "Ambient.Space", "Ambient.StationHum", "Ambient.MapRoomTone"]

MAN = json.loads(MANIFEST.read_text()) if MANIFEST.exists() else {"sounds": [], "groups": []}
SOUNDS = {s["id"]: s for s in MAN["sounds"]}
_cache = {}


def load(entry):
    """Returns (sample_rate, int16 pcm, float signal in [-1, 1])."""
    if entry["id"] not in _cache:
        _cache[entry["id"]] = wavfile.read(str(GEN_DIR / entry["file"]))
    sr, pcm = _cache[entry["id"]]
    return sr, pcm, pcm.astype(np.float64) / 32767.0


ids = pytest.mark.parametrize("eid", sorted(SOUNDS))


def test_manifest_exists_and_complete():
    assert MANIFEST.exists(), "run: python Tools/audio/generate_sfx.py"
    missing = [i for i in REQUIRED_IDS if i not in SOUNDS]
    assert not missing, f"missing event IDs: {missing}"
    for i in REQUIRED_LOOPS:
        assert SOUNDS[i]["loop"], f"{i} should loop"
    assert not any(i.startswith("Music") for i in SOUNDS), "no music this pass"


def test_manifest_and_files_match():
    listed = {(GEN_DIR / s["file"]).resolve() for s in MAN["sounds"]}
    on_disk = {p.resolve() for p in GEN_DIR.rglob("*.wav")}
    assert not listed - on_disk, f"manifest entries without files: {sorted(listed - on_disk)}"
    assert not on_disk - listed, f"files not in manifest: {sorted(on_disk - listed)}"
    assert len({s["id"] for s in MAN["sounds"]}) == len(MAN["sounds"]), "duplicate IDs"
    assert len({s["asset_name"] for s in MAN["sounds"]}) == len(MAN["sounds"]), "duplicate asset names"


def test_groups_reference_existing_sounds():
    groups = {g["id"]: g for g in MAN.get("groups", [])}
    assert len(groups.get("Interior.Footstep", {}).get("members", [])) >= 4
    for g in groups.values():
        assert g["id"] not in SOUNDS, f"group {g['id']} collides with a sound ID"
        for m in g["members"]:
            assert m in SOUNDS, f"group {g['id']} member {m} missing"


@ids
def test_format(eid):
    sr, pcm, _ = load(SOUNDS[eid])
    assert sr == 48000
    assert pcm.dtype == np.int16
    assert pcm.ndim == 1


@ids
def test_peak(eid):
    _, _, x = load(SOUNDS[eid])
    assert gen.peak_dbfs(x) <= -1.0 + 1e-3, f"{eid} peak {gen.peak_dbfs(x):.2f} dBFS"


@ids
def test_loudness(eid):
    e = SOUNDS[eid]
    _, _, x = load(e)
    lufs = gen.integrated_loudness(x)
    assert abs(lufs - e["target_lufs"]) <= LUFS_TOLERANCE, f"{eid}: {lufs:.1f} LUFS vs target {e['target_lufs']}"


@ids
def test_no_dc_offset(eid):
    _, _, x = load(SOUNDS[eid])
    assert abs(float(np.mean(x))) < DC_LIMIT


@ids
def test_duration(eid):
    e = SOUNDS[eid]
    sr, pcm, _ = load(e)
    dur = pcm.size / sr
    if e["loop"]:
        assert 4.0 <= dur <= 16.0, f"{eid} loop is {dur:.2f}s"
        assert e["loop_start"] == 0 and e["loop_end"] == pcm.size
    else:
        assert 0.03 <= dur <= 4.0, f"{eid} one-shot is {dur:.2f}s"
    assert e["num_samples"] == pcm.size


@pytest.mark.parametrize("eid", sorted(i for i, s in SOUNDS.items() if s["loop"]))
def test_loop_seam(eid):
    _, _, x = load(SOUNDS[eid])
    steps = np.abs(np.diff(x))
    seam = abs(x[0] - x[-1])
    # The wrap-around step must look like an ordinary sample step, not a click.
    limit = float(np.percentile(steps, 99.9)) * 1.5 + 2.0 / 32767
    assert seam <= limit, f"{eid}: seam step {seam:.5f} > {limit:.5f}"
    # Level continuity: 50 ms either side of the seam within 3 dB.
    w = int(0.05 * 48000)
    a = np.sqrt(np.mean(x[-w:] ** 2)) + 1e-9
    b = np.sqrt(np.mean(x[:w] ** 2)) + 1e-9
    assert abs(20 * np.log10(a / b)) < 3.0, f"{eid}: level jump across the seam"
    # High-frequency energy across the seam is no more than in the body (a click would spike it).
    wrapped = np.concatenate([x[-w:], x[:w]])
    inside = x[x.size // 2 - w: x.size // 2 + w]
    assert np.mean(np.diff(wrapped) ** 2) < np.mean(np.diff(inside) ** 2) * 3.0 + 1e-10, f"{eid}: seam click"


def test_engine_centroid_ordering():
    for rev in ("Low", "High"):
        cents = [gen.spectral_centroid(load(SOUNDS[f"Engine.{fam}.{rev}"])[2]) for fam in ENGINE_FAMILY_ORDER]
        assert cents == sorted(cents, reverse=True), f"{rev}: {dict(zip(ENGINE_FAMILY_ORDER, cents))}"


def test_high_revs_brighter_than_low():
    for fam in ENGINE_FAMILY_ORDER:
        lo = gen.spectral_centroid(load(SOUNDS[f"Engine.{fam}.Low"])[2])
        hi = gen.spectral_centroid(load(SOUNDS[f"Engine.{fam}.High"])[2])
        assert hi > lo, f"{fam}: High ({hi:.0f} Hz) not brighter than Low ({lo:.0f} Hz)"


def test_capital_has_almost_no_highs():
    _, _, x = load(SOUNDS["Engine.Capital.Low"])
    spec = np.abs(np.fft.rfft(x)) ** 2
    f = np.fft.rfftfreq(x.size, 1 / 48000)
    assert spec[f > 2000].sum() / spec.sum() < 0.001


@pytest.mark.parametrize("eid", ["UI.Click", "Engine.Light.Low", "Interior.Footstep.03"])
def test_deterministic(eid):
    x, _ = gen.generate(eid)
    pcm = np.clip(np.round(x * 32767.0), -32768, 32767).astype(np.int16)
    _, disk, _ = load(SOUNDS[eid])
    # Allow a few LSBs for FFT/BLAS differences between platforms and library versions.
    diff = int(np.max(np.abs(pcm.astype(np.int32) - disk.astype(np.int32))))
    assert diff <= 4, f"{eid}: regenerated samples differ from the committed WAV by up to {diff} LSB"
