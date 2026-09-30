#!/usr/bin/env python3
"""
Adastrea procedural sound generator.

Writes every game sound as a 48 kHz / 16-bit mono WAV to
    Assets/Audio/generated/<Category>/<Event_Id>.wav
plus Assets/Audio/generated/audio_manifest.json, which
Tools/import_audio_assets.py turns into /Game/Audio assets and the
/Game/Audio/DA_AudioCatalog event catalog.

Design rules
- Deterministic: every sound has its own seed (stable hash of its event ID),
  so re-running produces bit-identical files.
- Tunable: each recipe starts with a named parameter dict (P = dict(...)).
  Listening feedback like "the fighter is too whiny" is a one-line change there.
- Understated sci-fi: filtered noise, FM and additive synthesis, envelopes,
  a little generated-IR reverb. No raw square waves.
- Loudness: each sound is normalised to its target with `integrated_loudness`
  (a BS.1770-style K-weighted, gated loudness; see its docstring) and peaks are
  held at or below -1 dBFS by a smooth limiter.
- Loops are seamless: they are rendered long and the tail is crossfaded into
  the head, so the whole file loops (loop_start=0, loop_end=num_samples).

Usage:
    python Tools/audio/generate_sfx.py            # all sounds
    python Tools/audio/generate_sfx.py Engine UI  # only IDs starting with these prefixes
"""

from __future__ import annotations

import json
import sys
import zlib
from pathlib import Path

import numpy as np
from scipy import signal
from scipy.io import wavfile

SR = 48000
PEAK_CEILING_DBFS = -1.0
REPO_ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = REPO_ROOT / "Assets" / "Audio" / "generated"
MANIFEST_PATH = OUT_DIR / "audio_manifest.json"

# Target loudness per group (LUFS, see integrated_loudness). SOUND_PLAN.md section 3:
# UI -18, engine -24, ambience -30. Everything else sits between.
LUFS = {
    "UI": -18.0,
    "UI.soft": -24.0,
    "Engine": -24.0,
    "Engine.layer": -27.0,
    "World": -20.0,
    "Interior": -22.0,
    "Ambient": -30.0,
}

# Folder (manifest "category") -> Unreal sound class, and default attenuation.
CATEGORY_SOUND_CLASS = {
    "Engine": "SC_Engine",
    "Flight": "SC_World",
    "Dock": "SC_World",
    "Mining": "SC_World",
    "Trade": "SC_UI",
    "Editor": "SC_UI",
    "Interior": "SC_Interior",
    "UI": "SC_UI",
    "Ambient": "SC_Ambient",
}


# ---------------------------------------------------------------------------
# Loudness / analysis (also imported by tests/test_audio_assets.py)
# ---------------------------------------------------------------------------

# BS.1770 K-weighting at 48 kHz (pre-filter shelf + RLB high-pass).
_K_SHELF_B = [1.53512485958697, -2.69169618940638, 1.19839281085285]
_K_SHELF_A = [1.0, -1.69065929318241, 0.73248077421585]
_K_HP_B = [1.0, -2.0, 1.0]
_K_HP_A = [1.0, -1.99004745483398, 0.99007225036621]


def k_weight(x: np.ndarray) -> np.ndarray:
    y = signal.lfilter(_K_SHELF_B, _K_SHELF_A, x)
    return signal.lfilter(_K_HP_B, _K_HP_A, y)


def integrated_loudness(x: np.ndarray, sr: int = SR) -> float:
    """Integrated loudness in LUFS (mono), BS.1770-style.

    K-weighting, gated block energies (absolute gate -70 LUFS, relative gate -10 LU),
    75% block overlap. Blocks are 400 ms as in the standard for sounds >= 1 s; for
    shorter one-shots (clicks, ticks) the block is shortened to 100 ms so the
    measurement reflects the audible part instead of being diluted by silence.
    This is a documented deviation for UI-length sounds, not a certified meter.
    """
    x = np.asarray(x, dtype=np.float64)
    if x.size == 0:
        return -120.0
    y = k_weight(x)
    block = int((0.4 if x.size >= sr else 0.1) * sr)
    block = min(block, y.size)
    hop = max(1, block // 4)
    starts = range(0, y.size - block + 1, hop)
    z = np.array([np.mean(y[s:s + block] ** 2) for s in starts])
    z = z[z > 0]
    if z.size == 0:
        return -120.0
    lk = -0.691 + 10 * np.log10(z)
    z = z[lk > -70.0]
    if z.size == 0:
        return -120.0
    rel = -0.691 + 10 * np.log10(np.mean(z)) - 10.0
    lk = -0.691 + 10 * np.log10(z)
    z = z[lk > rel]
    return float(-0.691 + 10 * np.log10(np.mean(z)))


def peak_dbfs(x: np.ndarray) -> float:
    p = float(np.max(np.abs(x))) if x.size else 0.0
    return 20 * np.log10(p) if p > 0 else -120.0


def spectral_centroid(x: np.ndarray, sr: int = SR) -> float:
    spec = np.abs(np.fft.rfft(x * np.hanning(x.size)))
    freqs = np.fft.rfftfreq(x.size, 1.0 / sr)
    return float(np.sum(freqs * spec) / max(np.sum(spec), 1e-12))


# ---------------------------------------------------------------------------
# DSP building blocks
# ---------------------------------------------------------------------------

def seed_for(event_id: str) -> int:
    return zlib.crc32(event_id.encode("utf-8")) & 0x7FFFFFFF


def t_axis(dur: float) -> np.ndarray:
    return np.arange(int(round(dur * SR))) / SR


def db(v: float) -> float:
    return 10 ** (v / 20.0)


def _sos(kind: str, freq, order: int = 2):
    nyq = SR / 2
    if isinstance(freq, (list, tuple)):
        wn = [min(max(f / nyq, 1e-5), 0.999) for f in freq]
    else:
        wn = min(max(freq / nyq, 1e-5), 0.999)
    return signal.butter(order, wn, btype=kind, output="sos")


def lowpass(x, f, order=2):
    return signal.sosfilt(_sos("lowpass", f, order), x)


def highpass(x, f, order=2):
    return signal.sosfilt(_sos("highpass", f, order), x)


def bandpass(x, lo, hi, order=2):
    return signal.sosfilt(_sos("bandpass", [lo, hi], order), x)


def white(rng, n):
    return rng.standard_normal(n)


def pink(rng, n):
    """Pink-ish noise via 1/f spectral shaping."""
    spec = np.fft.rfft(rng.standard_normal(n))
    f = np.fft.rfftfreq(n, 1.0 / SR)
    f[0] = f[1]
    spec /= np.sqrt(f)
    y = np.fft.irfft(spec, n)
    return y / (np.std(y) + 1e-12)


def brown(rng, n):
    y = np.cumsum(rng.standard_normal(n))
    y = highpass(y, 10)
    return y / (np.std(y) + 1e-12)


def smooth_random(rng, n, rate_hz, depth=1.0):
    """Slowly wandering control signal in [-depth, depth]-ish (lowpassed noise)."""
    y = lowpass(rng.standard_normal(n), rate_hz, order=2)
    y = y / (np.max(np.abs(y)) + 1e-12)
    return y * depth


def phase_from_freq(freq):
    """Integrate an instantaneous frequency array to phase (radians)."""
    return 2 * np.pi * np.cumsum(freq) / SR


def env_ad(n, attack, decay, curve=4.0):
    """Attack (linear) then exponential-ish decay over `decay` seconds."""
    t = np.arange(n) / SR
    a = np.clip(t / max(attack, 1e-4), 0, 1)
    d = np.exp(-curve * np.clip(t - attack, 0, None) / max(decay, 1e-4))
    return a * d


def env_adsr(n, a, d, s, r, sustain_len=None):
    t = np.arange(n) / SR
    total = n / SR
    if sustain_len is None and a + d + r > total:
        # Squeeze the stages to fit, so the release always reaches 0 (a cut-off release clicks).
        k = total / (a + d + r)
        a, d, r = a * k, d * k, r * k
    if sustain_len is None:
        sustain_len = max(total - a - d - r, 0)
    e = np.zeros(n)
    e = np.where(t < a, t / max(a, 1e-4), e)
    m = (t >= a) & (t < a + d)
    e = np.where(m, 1 - (1 - s) * (t - a) / max(d, 1e-4), e)
    m = (t >= a + d) & (t < a + d + sustain_len)
    e = np.where(m, s, e)
    rs = a + d + sustain_len
    m = t >= rs
    e = np.where(m, s * np.clip(1 - (t - rs) / max(r, 1e-4), 0, 1), e)
    return e


def fade_edges(x, fade_in=0.002, fade_out=0.01):
    x = x.copy()
    ni = min(int(fade_in * SR), x.size // 2)
    no = min(int(fade_out * SR), x.size // 2)
    if ni > 0:
        x[:ni] *= np.linspace(0, 1, ni)
    if no > 0:
        x[-no:] *= np.linspace(1, 0, no)
    return x


def tail_taper(x, frac=0.25, max_s=0.08):
    """Half-cosine fade over the end of a fixed-length layer, so a ring that hasn't
    decayed by the end of its buffer fades out instead of being cut (a cut clicks)."""
    x = x.copy()
    k = min(int(x.size * frac), int(max_s * SR))
    if k > 1:
        x[-k:] *= 0.5 + 0.5 * np.cos(np.linspace(0, np.pi, k))
    return x


def transient(rng, dur=0.004, band=(5000.0, 15000.0)):
    """Very short band-limited noise tick: the crisp 'air' at the front of a hit or a UI note.
    Peak-normalised; mix it in around 0.1-0.3."""
    n = max(8, int(dur * SR))
    y = bandpass(white(rng, n), *band) * env_ad(n, 0.0003, dur, 5)
    return y / (np.max(np.abs(y)) + 1e-12)


def pad(x, dur):
    n = int(round(dur * SR))
    if x.size >= n:
        return x[:n]
    return np.concatenate([x, np.zeros(n - x.size)])


def place(dst, src, at_s, gain=1.0):
    """Mixes src into dst at at_s. If src runs past the end of dst, the overhang is
    faded out rather than cut; size dst so that doesn't happen for anything audible."""
    i = int(round(at_s * SR))
    j = min(dst.size, i + src.size)
    if i < dst.size:
        part = src[: j - i]
        if j - i < src.size:
            part = tail_taper(part, frac=0.5, max_s=0.03)
        dst[i:j] += gain * part
    return dst


def fm_tone(n, fc, ratio=2.0, index=1.0, index_decay=None, rng=None):
    """Simple 2-op FM (sine carrier, sine modulator). Index can decay for bell-like tones."""
    t = np.arange(n) / SR
    idx = index * (np.exp(-t / index_decay) if index_decay else 1.0)
    mod = idx * np.sin(2 * np.pi * fc * ratio * t)
    return np.sin(2 * np.pi * fc * t + mod)


def bell(dur, f, decay=0.35, ratio=1.4, index=1.2, attack=0.003):
    n = int(dur * SR)
    tone = fm_tone(n, f, ratio=ratio, index=index, index_decay=decay * 0.5)
    tone += 0.25 * np.sin(2 * np.pi * f * 2.0 * np.arange(n) / SR) * np.exp(-np.arange(n) / SR / (decay * 0.4))
    return tone * env_ad(n, attack, decay)


def modal(dur, freqs, decays, amps, rng=None, excite_noise=0.0):
    """Sum of exponentially decaying partials (metal / impacts)."""
    n = int(dur * SR)
    t = np.arange(n) / SR
    y = np.zeros(n)
    for f, d, a in zip(freqs, decays, amps):
        y += a * np.sin(2 * np.pi * f * t) * np.exp(-t / d)
    if excite_noise and rng is not None:
        y += excite_noise * white(rng, n) * np.exp(-t / 0.004)
    return tail_taper(y)


def thump(dur, f_start, f_end, decay, drop_time=0.05):
    """Pitch-dropping sine (kick-like body)."""
    n = int(dur * SR)
    t = np.arange(n) / SR
    f = f_end + (f_start - f_end) * np.exp(-t / max(drop_time, 1e-4))
    return tail_taper(np.sin(phase_from_freq(f)) * env_ad(n, 0.001, decay))


def sweep_filter(x, cutoffs, kind="lowpass", bands=10, q_width=0.6):
    """Time-varying filter: crossfade between `bands` fixed filters by per-sample cutoff."""
    cutoffs = np.clip(np.asarray(cutoffs, dtype=np.float64), 30, SR * 0.45)
    lo, hi = float(np.min(cutoffs)), float(np.max(cutoffs))
    if hi / lo < 1.05:
        centers = np.array([lo])
    else:
        centers = np.geomspace(lo, hi, bands)
    outs = []
    for c in centers:
        if kind == "lowpass":
            outs.append(lowpass(x, c))
        elif kind == "highpass":
            outs.append(highpass(x, c))
        else:
            outs.append(bandpass(x, c * (1 - q_width / 2), c * (1 + q_width / 2)))
    outs = np.array(outs)
    if centers.size == 1:
        return outs[0]
    pos = np.interp(np.log(cutoffs), np.log(centers), np.arange(centers.size))
    i0 = np.floor(pos).astype(int)
    i1 = np.minimum(i0 + 1, centers.size - 1)
    w = pos - i0
    idx = np.arange(x.size)
    return outs[i0, idx] * (1 - w) + outs[i1, idx] * w


def reverb(x, rng, decay=0.6, wet=0.2, tone_hz=5000, predelay=0.012):
    """Convolve with a generated IR (decaying filtered noise + a few early reflections)."""
    n = int((decay * 1.5 + predelay) * SR)
    t = np.arange(n) / SR
    ir = white(rng, n) * np.exp(-6.9 * t / decay)  # -60 dB at `decay`
    ir = lowpass(ir, tone_hz)
    ir[: int(predelay * SR)] = 0
    for k, (d, g) in enumerate([(0.007, 0.5), (0.013, 0.35), (0.021, 0.25)]):
        ir[int(d * SR)] += g * (1 if k % 2 == 0 else -1)
    ir /= np.sqrt(np.sum(ir ** 2)) + 1e-12
    wet_sig = signal.fftconvolve(x, ir)[: x.size]
    return (1 - wet) * x + wet * wet_sig * (np.std(x) / (np.std(wet_sig) + 1e-12))


def reverb_tail(x, rng, decay=0.6, wet=0.2, tone_hz=5000):
    """Reverb for one-shots: keeps the tail by extending the buffer first."""
    ext = np.concatenate([x, np.zeros(int(decay * 1.2 * SR))])
    y = reverb(ext, rng, decay, wet, tone_hz)
    # trim trailing near-silence
    thr = np.max(np.abs(y)) * db(-60)
    nz = np.nonzero(np.abs(y) > thr)[0]
    end = int(nz[-1]) + 1 if nz.size else y.size
    return fade_edges(y[: max(end, x.size)], 0.0, 0.02)


def presence(y, formant_hz, gain, drive=0.0):
    """Makes a low sound audible on small speakers without thinning it: boosts a resonance
    band an octave or two above the fundamental (the ear infers the fundamental from it),
    then optionally saturates (tanh) to add harmonics. Keep the clean sub outside this."""
    if gain > 0:
        y = y + gain * bandpass(y, formant_hz * 0.7, formant_hz * 1.45)
    if drive > 0:
        y = y / (np.max(np.abs(y)) + 1e-12)
        y = np.tanh(drive * y) / np.tanh(drive)
    return y


def make_loop(render, dur, xfade=0.5):
    """Render dur+xfade seconds via render(n) and fold the tail into the head (equal power)."""
    n = int(round(dur * SR))
    nx = int(round(xfade * SR))
    body = render(n + nx)
    body = highpass(body, 12)
    fi = np.sin(np.linspace(0, np.pi / 2, nx)) ** 1
    fo = np.cos(np.linspace(0, np.pi / 2, nx)) ** 1
    out = body[:n].copy()
    out[:nx] = body[:nx] * fi + body[n:n + nx] * fo
    out = out - np.mean(out)
    # The loop is circular, so rotating it keeps it seamless. Start it at an upward zero
    # crossing so the file begins and ends near 0 (no importer DC warnings), choosing the one
    # where the level just before and just after match best (a level step reads as a seam
    # when the file is played once, or by tools that check the join).
    zc = np.nonzero((out[:-1] < 0) & (out[1:] >= 0))[0] + 1
    if zc.size:
        w = int(0.05 * SR)
        e = np.concatenate([out, out]) ** 2
        c = np.concatenate([[0.0], np.cumsum(e)])
        after = (c[zc + w] - c[zc]) / w
        before = (c[zc + out.size] - c[zc + out.size - w]) / w
        mismatch = np.abs(10 * np.log10((after + 1e-12) / (before + 1e-12)))
        step = (np.abs(out[zc]) + np.abs(out[zc - 1])) / (np.max(np.abs(out)) + 1e-12)
        k = int(zc[np.argmin(mismatch + 20 * step)])
        out = np.roll(out, -k)
    return out


def limiter(x, ceiling_db=PEAK_CEILING_DBFS - 0.1, release=0.05, lookahead=0.002):
    """Smooth peak limiter: per-sample gain reduction, held over a lookahead window and smoothed."""
    ceiling = db(ceiling_db)
    a = np.abs(x)
    if a.max() <= ceiling:
        return x
    g = np.minimum(1.0, ceiling / np.maximum(a, 1e-12))
    la = max(1, int(lookahead * SR))
    # min-filter so gain is already down when the peak arrives
    from scipy.ndimage import minimum_filter1d, uniform_filter1d
    g = minimum_filter1d(g, size=2 * la + 1)
    g = uniform_filter1d(g, size=la)
    # release smoothing
    rel = np.exp(-1.0 / (release * SR))
    g = signal.lfilter([1 - rel], [1, -rel], g - 1.0) + 1.0
    g = np.minimum(g, minimum_filter1d(np.minimum(1.0, ceiling / np.maximum(a, 1e-12)), size=2 * la + 1))
    return x * g


def normalise(x, target_lufs, loop=False):
    """Gain to target loudness, then limit peaks to the ceiling; iterate to converge."""
    x = x - np.mean(x)
    for _ in range(4):
        cur = integrated_loudness(x)
        x = x * db(target_lufs - cur)
        x = limiter(x)
        x = x - np.mean(x)  # limiting asymmetric transients can leave a little DC
    # final hard safety (should be a no-op)
    ceiling = db(PEAK_CEILING_DBFS - 0.05)
    m = np.max(np.abs(x))
    if m > ceiling:
        x = x * (ceiling / m)
    return x


# ---------------------------------------------------------------------------
# Registry
# ---------------------------------------------------------------------------

SOUNDS: dict = {}


def sound(event_id, category, target, loop=False, pitch_range=(1.0, 1.0), volume=1.0,
          attenuation=None, variations_of=None):
    """Registers a recipe. `target` is a key in LUFS or a number."""
    def deco(fn):
        SOUNDS[event_id] = dict(fn=fn, category=category, target=LUFS.get(target, target),
                                loop=loop, pitch_range=list(pitch_range), volume=volume,
                                attenuation=attenuation, variations_of=variations_of)
        return fn
    return deco


# ---------------------------------------------------------------------------
# ENGINE FAMILIES
# ---------------------------------------------------------------------------
# Tuning knobs per family. f0: body fundamental (Hz). harm_rolloff: amplitude
# exponent of harmonic k (a_k = k^-rolloff; lower = brighter). n_harm: harmonics.
# lp: final lowpass (Hz). rumble: lowpassed-noise level. hiss: high exhaust noise.
# throb_hz / throb_depth: slow amplitude pulse. sub: level of a sine an octave below.
# wander: pitch drift (fraction). High-revs loops use f0 * high_f0_mult, more
# harmonics (high_bright) and faster throb.
# Audibility and life (so big ships survive laptop speakers and don't sound frozen):
# formant_hz / formant_gain: hull resonance boost (the ear infers the low fundamental from
# these harmonics when the speaker can't play it). drive: tanh saturation (adds harmonics,
# glues layers). combust: noise bursts at the firing rate f0 (texture that moves).
# flutter: random amplitude wobble around 14 Hz (turbulence).
ENGINE_FAMILIES = {
    "Light":   dict(f0=125.0, n_harm=24, harm_rolloff=1.05, lp=7000.0, rumble=0.25, hiss=0.12,
                    throb_hz=0.0, throb_depth=0.0, sub=0.0, wander=0.006, high_f0_mult=1.18,
                    high_bright=1.25, formant_hz=0.0, formant_gain=0.0, drive=1.2, combust=0.15,
                    flutter=0.04),
    "Medium":  dict(f0=80.0, n_harm=18, harm_rolloff=1.35, lp=3200.0, rumble=0.35, hiss=0.05,
                    throb_hz=1.5, throb_depth=0.10, sub=0.0, wander=0.005, high_f0_mult=1.15,
                    high_bright=1.2, formant_hz=320.0, formant_gain=0.8, drive=1.5, combust=0.25,
                    flutter=0.05),
    "Heavy":   dict(f0=52.0, n_harm=14, harm_rolloff=1.7, lp=1400.0, rumble=0.45, hiss=0.015,
                    throb_hz=0.75, throb_depth=0.25, sub=0.15, wander=0.004, high_f0_mult=1.12,
                    high_bright=1.15, formant_hz=240.0, formant_gain=1.6, drive=2.0, combust=0.35,
                    flutter=0.06),
    "Capital": dict(f0=35.0, n_harm=10, harm_rolloff=2.1, lp=520.0, rumble=0.55, hiss=0.0,
                    throb_hz=0.5, throb_depth=0.35, sub=0.45, wander=0.003, high_f0_mult=1.1,
                    high_bright=1.1, formant_hz=175.0, formant_gain=2.2, drive=2.4, combust=0.45,
                    flutter=0.07),
}
ENGINE_LOOP_SECONDS = 6.0


def _engine_voice(family, high, rng):
    P = dict(ENGINE_FAMILIES[family])
    f0 = P["f0"] * (P["high_f0_mult"] if high else 1.0)
    rolloff = P["harm_rolloff"] / (P["high_bright"] if high else 1.0)
    lp_hz = P["lp"] * (P["high_bright"] if high else 1.0)
    throb_hz = P["throb_hz"] * (1.5 if high else 1.0)

    def render(n):
        t = np.arange(n) / SR
        drift = 1 + smooth_random(rng, n, 0.7, P["wander"])
        ph = phase_from_freq(f0 * drift)
        body = np.zeros(n)
        for k in range(1, P["n_harm"] + 1):
            detune = 1 + 0.0015 * rng.uniform(-1, 1)
            body += (k ** -rolloff) * np.sin(k * ph * detune + rng.uniform(0, 2 * np.pi))
        # second, slightly detuned voice for width/beating
        body += 0.35 * np.sin(ph * 1.004 + 1.3)
        body /= np.max(np.abs(body)) + 1e-12
        rumble = lowpass(white(rng, n), f0 * 2.5, order=2)
        rumble /= np.std(rumble) + 1e-12
        hiss = bandpass(white(rng, n), 2500, 9000)
        hiss /= np.std(hiss) + 1e-12
        sub = np.sin(ph * 0.5)
        # Combustion: band-limited noise bursts once per cycle of the fundamental.
        burst = (0.5 + 0.5 * np.cos(ph)) ** 6
        comb = bandpass(white(rng, n), f0 * 2, min(f0 * 12, lp_hz)) * burst
        comb /= np.std(comb) + 1e-12
        y = (body + P["rumble"] * 0.3 * rumble + P["hiss"] * (1.6 if high else 1.0) * 0.3 * hiss
             + P["combust"] * 0.3 * comb)
        y = presence(y, P["formant_hz"], P["formant_gain"], P["drive"])
        y = y + P["sub"] * sub  # clean sub under the saturated body
        y *= 1 + P["flutter"] * smooth_random(rng, n, 14)
        if throb_hz > 0:
            y *= 1 - P["throb_depth"] * (0.5 + 0.5 * np.sin(2 * np.pi * throb_hz * t))
        return lowpass(y, lp_hz, order=4)

    params = dict(P, f0_effective=f0, loop_seconds=ENGINE_LOOP_SECONDS, variant="High" if high else "Low")
    return make_loop(render, ENGINE_LOOP_SECONDS), params


def _register_engines():
    for fam in ENGINE_FAMILIES:
        for high in (False, True):
            eid = f"Engine.{fam}.{'High' if high else 'Low'}"

            def fn(rng, fam=fam, high=high):
                return _engine_voice(fam, high, rng)
            sound(eid, "Engine", "Engine", loop=True, attenuation="ATT_AIEngine")(fn)


_register_engines()


@sound("Engine.Whine", "Engine", "Engine.layer", loop=True, attenuation="ATT_AIEngine")
def engine_whine(rng):
    P = dict(freq=2100.0, partial2=1.52, partial2_level=0.35, noise_band=(1700, 2800), noise_level=0.25,
             wander=0.004, lp=6000.0, seconds=4.0)

    def render(n):
        drift = 1 + smooth_random(rng, n, 1.2, P["wander"])
        ph = phase_from_freq(P["freq"] * drift)
        y = np.sin(ph) + P["partial2_level"] * np.sin(ph * P["partial2"])
        nb = bandpass(white(rng, n), *P["noise_band"])
        y += P["noise_level"] * nb / (np.std(nb) + 1e-12) * 0.3
        return lowpass(y, P["lp"])
    return make_loop(render, P["seconds"]), P


@sound("Engine.Boost", "Engine", "Engine", loop=True, attenuation="ATT_AIEngine")
def engine_boost(rng):
    P = dict(band=(180.0, 2600.0), roar_level=1.0, tone_hz=95.0, tone_level=0.35, flutter_hz=11.0,
             flutter_depth=0.12, seconds=4.0)

    def render(n):
        t = np.arange(n) / SR
        roar = bandpass(pink(rng, n), *P["band"])
        roar /= np.std(roar) + 1e-12
        roar *= 1 - P["flutter_depth"] * (0.5 + 0.5 * np.sin(2 * np.pi * P["flutter_hz"] * t))
        tone = np.sin(2 * np.pi * P["tone_hz"] * t) + 0.4 * np.sin(2 * np.pi * P["tone_hz"] * 2 * t)
        return P["roar_level"] * roar + P["tone_level"] * tone
    return make_loop(render, P["seconds"]), P


@sound("Engine.BoostStart", "Engine", "Engine")
def engine_boost_start(rng):
    P = dict(dur=1.3, cutoff_from=250.0, cutoff_to=4000.0, thump_hz=(90.0, 45.0), thump_level=0.6,
             reverb_wet=0.2)
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    cut = P["cutoff_from"] * (P["cutoff_to"] / P["cutoff_from"]) ** np.clip(t / 0.5, 0, 1)
    whoosh = sweep_filter(pink(rng, n), cut, "lowpass") * env_adsr(n, 0.25, 0.3, 0.6, 0.6)
    whoosh /= np.max(np.abs(whoosh)) + 1e-12
    y = whoosh + P["thump_level"] * pad(thump(0.6, *P["thump_hz"], 0.25), P["dur"])
    return reverb_tail(y, rng, 0.8, P["reverb_wet"]), P


def _spool(rng, up):
    P = dict(dur=1.6, f_low=55.0, f_high=150.0, noise_lp=(300.0, 2200.0), noise_level=0.5, harmonics=6)
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    x = np.clip(t / (P["dur"] * 0.85), 0, 1)
    shape = x ** 0.7 if up else 1 - x ** 0.7
    f = P["f_low"] + (P["f_high"] - P["f_low"]) * shape
    ph = phase_from_freq(f)
    tone = sum((k ** -1.4) * np.sin(k * ph) for k in range(1, P["harmonics"] + 1))
    cut = P["noise_lp"][0] + (P["noise_lp"][1] - P["noise_lp"][0]) * shape
    nz = sweep_filter(white(rng, n), cut, "lowpass")
    nz /= np.std(nz) + 1e-12
    env = env_adsr(n, 0.35 if up else 0.02, 0.2, 0.9, 0.5) if up else np.clip(1 - t / P["dur"], 0, 1) ** 1.5
    y = (tone / 2 + P["noise_level"] * 0.3 * nz) * env
    return fade_edges(y, 0.01, 0.05), P


@sound("Engine.SpoolUp", "Engine", "Engine")
def engine_spool_up(rng):
    return _spool(rng, True)


@sound("Engine.SpoolDown", "Engine", "Engine")
def engine_spool_down(rng):
    return _spool(rng, False)


@sound("Engine.Idle", "Engine", "Engine.layer", loop=True, attenuation="ATT_AIEngine")
def engine_idle(rng):
    P = dict(f0=58.0, harmonics=5, rolloff=1.8, noise_lp=400.0, noise_level=0.4, pulse_hz=0.5,
             pulse_depth=0.15, seconds=6.0, formant_hz=280.0, formant_gain=3.0, drive=1.8)

    def render(n):
        t = np.arange(n) / SR
        ph = phase_from_freq(P["f0"] * (1 + smooth_random(rng, n, 0.5, 0.003)))
        y = sum((k ** -P["rolloff"]) * np.sin(k * ph) for k in range(1, P["harmonics"] + 1))
        nz = lowpass(white(rng, n), P["noise_lp"])
        y = presence(y + P["noise_level"] * nz / (np.std(nz) + 1e-12) * 0.3, P["formant_hz"], P["formant_gain"],
                     P["drive"])
        return y * (1 - P["pulse_depth"] * (0.5 + 0.5 * np.sin(2 * np.pi * P["pulse_hz"] * t)))
    return make_loop(render, P["seconds"]), P


# ---------------------------------------------------------------------------
# FLIGHT
# ---------------------------------------------------------------------------

@sound("Thruster.Puff", "Flight", "World", pitch_range=(0.9, 1.1), attenuation="ATT_World")
def thruster_puff(rng):
    P = dict(dur=0.35, band=(700.0, 4500.0), attack=0.004, decay=0.09, reverb_wet=0.12)
    n = int(P["dur"] * SR)
    y = bandpass(white(rng, n), *P["band"]) * env_ad(n, P["attack"], P["decay"])
    return reverb_tail(fade_edges(y), rng, 0.4, P["reverb_wet"]), P


@sound("Thruster.HeavyGroan", "Flight", "World", pitch_range=(0.92, 1.05), attenuation="ATT_World")
def thruster_heavy_groan(rng):
    P = dict(dur=1.4, f_from=62.0, f_to=44.0, noise_lp=320.0, noise_level=0.6, attack=0.18, release=0.7,
             formant_hz=230.0, formant_gain=1.8, drive=2.0)
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    f = P["f_from"] + (P["f_to"] - P["f_from"]) * (t / P["dur"])
    ph = phase_from_freq(f * (1 + smooth_random(rng, n, 6, 0.01)))
    tone = np.sin(ph) + 0.5 * np.sin(2 * ph) + 0.2 * np.sin(3 * ph)
    nz = lowpass(white(rng, n), P["noise_lp"])
    y = (tone / 1.7 + P["noise_level"] * nz / (np.std(nz) + 1e-12) * 0.3)
    y = presence(y, P["formant_hz"], P["formant_gain"], P["drive"])
    y *= env_adsr(n, P["attack"], 0.2, 0.8, P["release"])
    return fade_edges(lowpass(y, 900)), P


@sound("Flight.CollisionBump", "Flight", "World", pitch_range=(0.9, 1.1), attenuation="ATT_World")
def flight_collision_bump(rng):
    P = dict(thump=(75.0, 38.0, 0.35), clank=[190.0, 437.0, 822.0, 1370.0, 2950.0], clank_decay=[0.5, 0.3, 0.18, 0.1, 0.04],
             clank_level=0.45, noise_level=0.4, air_level=0.35, reverb_wet=0.25, dur=2.0)
    body = thump(P["dur"], P["thump"][0], P["thump"][1], P["thump"][2])
    clank = modal(P["dur"], P["clank"], P["clank_decay"], [1, 0.7, 0.5, 0.3, 0.25], rng, excite_noise=0.5)
    n = body.size
    nz = lowpass(white(rng, n), 1800) * env_ad(n, 0.001, 0.05)
    y = body + P["clank_level"] * clank / (np.max(np.abs(clank)) + 1e-12) + P["noise_level"] * nz
    place(y, transient(rng, 0.012, (3000.0, 16000.0)), 0.0, P["air_level"])
    return reverb_tail(y, rng, 0.9, P["reverb_wet"], 6000), P


@sound("Flight.SpeedWarning", "Flight", "UI.soft")
def flight_speed_warning(rng):
    P = dict(notes=[880.0, 659.25], note_len=0.16, gap=0.05, repeats=2, repeat_gap=0.12, fm_index=0.6, air_level=0.3)
    y = np.zeros(int(1.2 * SR))
    at = 0.0
    for _ in range(P["repeats"]):
        for f in P["notes"]:
            nn = int(P["note_len"] * SR)
            tone = fm_tone(nn, f, 2.0, P["fm_index"]) * env_adsr(nn, 0.008, 0.04, 0.6, 0.06)
            place(y, tone, at)
            place(y, transient(rng), at, P["air_level"])
            at += P["note_len"] + P["gap"]
        at += P["repeat_gap"]
    return reverb_tail(lowpass(y, 16000), rng, 0.4, 0.15, 8000), P


# ---------------------------------------------------------------------------
# DOCKING
# ---------------------------------------------------------------------------

@sound("Dock.Beacon", "Dock", "World", attenuation="ATT_World")
def dock_beacon(rng):
    P = dict(freq=1318.5, decay=0.55, ratio=1.0, index=0.8, reverb_wet=0.35)
    y = bell(1.0, P["freq"], P["decay"], P["ratio"], P["index"])
    place(y, transient(rng), 0.0, UI_AIR)
    return reverb_tail(y, rng, 1.2, P["reverb_wet"], 8000), P


@sound("Dock.ClampEngage", "Dock", "World", attenuation="ATT_World")
def dock_clamp_engage(rng):
    P = dict(servo_dur=0.35, servo_band=(300.0, 900.0), clamp_at=0.38, thump=(70.0, 40.0, 0.4),
             clank=[143.0, 311.0, 587.0, 1043.0, 2410.0], clank_decay=[0.7, 0.45, 0.25, 0.12, 0.05], reverb_wet=0.3,
             ring=2.2, air_level=0.35)
    y = np.zeros(int((P["clamp_at"] + P["ring"]) * SR))
    ns = int(P["servo_dur"] * SR)
    ts = np.arange(ns) / SR
    cut = P["servo_band"][0] + (P["servo_band"][1] - P["servo_band"][0]) * ts / P["servo_dur"]
    servo = sweep_filter(white(rng, ns), cut, "bandpass", q_width=0.3) * env_adsr(ns, 0.05, 0.1, 0.8, 0.05)
    place(y, 0.4 * servo / (np.max(np.abs(servo)) + 1e-12), 0.0)
    th = thump(P["ring"], *P["thump"])
    cl = modal(P["ring"], P["clank"], P["clank_decay"], [1, 0.8, 0.5, 0.3, 0.25], rng, excite_noise=0.6)
    place(y, th + 0.6 * cl / (np.max(np.abs(cl)) + 1e-12), P["clamp_at"])
    place(y, transient(rng, 0.015, (3000.0, 16000.0)), P["clamp_at"], P["air_level"])
    return reverb_tail(y, rng, 1.0, P["reverb_wet"], 5500), P


@sound("Dock.AirlockHiss", "Dock", "World", attenuation="ATT_World")
def dock_airlock_hiss(rng):
    P = dict(dur=2.2, band=(1800.0, 11000.0), attack=0.08, decay=0.3, sustain=0.7, release=1.1,
             seal_thud=(110.0, 60.0, 0.12), seal_level=0.5)
    n = int(P["dur"] * SR)
    hiss = bandpass(white(rng, n), *P["band"])
    hiss *= env_adsr(n, P["attack"], P["decay"], P["sustain"], P["release"])
    hiss *= 1 + 0.1 * smooth_random(rng, n, 8)
    hiss /= np.max(np.abs(hiss)) + 1e-12
    y = hiss + P["seal_level"] * pad(thump(0.4, *P["seal_thud"]), P["dur"])
    return reverb_tail(y, rng, 0.6, 0.15), P


@sound("Dock.Release", "Dock", "World", attenuation="ATT_World")
def dock_release(rng):
    P = dict(clunk=[180.0, 402.0, 760.0], clunk_decay=[0.35, 0.2, 0.1], thump=(90.0, 50.0, 0.2),
             hiss_at=0.12, hiss_dur=0.6, servo_from=700.0, servo_to=250.0, reverb_wet=0.25)
    y = np.zeros(int(1.6 * SR))
    cl = modal(0.8, P["clunk"], P["clunk_decay"], [1, 0.6, 0.3], rng, excite_noise=0.5)
    place(y, thump(0.8, *P["thump"]) + 0.5 * cl / (np.max(np.abs(cl)) + 1e-12), 0.0)
    nh = int(P["hiss_dur"] * SR)
    hiss = bandpass(white(rng, nh), 2000, 9000) * env_ad(nh, 0.02, 0.25, 3)
    place(y, 0.35 * hiss / (np.max(np.abs(hiss)) + 1e-12), P["hiss_at"])
    ns = int(0.7 * SR)
    ts = np.arange(ns) / SR
    cut = P["servo_from"] + (P["servo_to"] - P["servo_from"]) * ts / 0.7
    servo = sweep_filter(white(rng, ns), cut, "bandpass", q_width=0.3) * env_adsr(ns, 0.05, 0.1, 0.7, 0.3)
    place(y, 0.3 * servo / (np.max(np.abs(servo)) + 1e-12), 0.5)
    return reverb_tail(y, rng, 0.9, P["reverb_wet"], 3500), P


# ---------------------------------------------------------------------------
# MINING
# ---------------------------------------------------------------------------

@sound("Mining.LaserLoop", "Mining", "World", loop=True, attenuation="ATT_World")
def mining_laser_loop(rng):
    P = dict(freq=220.0, ratio=1.5, index=1.4, lp=4200.0, crackle_band=(2000.0, 6000.0), crackle_level=0.25,
             shimmer_hz=6.0, shimmer_depth=0.15, seconds=4.0)

    def render(n):
        t = np.arange(n) / SR
        drift = 1 + smooth_random(rng, n, 3, 0.004)
        ph = phase_from_freq(P["freq"] * drift)
        mod = P["index"] * (1 + 0.3 * smooth_random(rng, n, 2)) * np.sin(ph * P["ratio"])
        tone = np.sin(ph + mod) + 0.4 * np.sin(2 * ph + 0.5 * mod)
        tone *= 1 - P["shimmer_depth"] * (0.5 + 0.5 * np.sin(2 * np.pi * P["shimmer_hz"] * t))
        cr = bandpass(white(rng, n), *P["crackle_band"]) * np.abs(smooth_random(rng, n, 20))
        cr /= np.std(cr) + 1e-12
        return lowpass(tone + P["crackle_level"] * 0.3 * cr, P["lp"])
    return make_loop(render, P["seconds"]), P


@sound("Mining.OreTick", "Mining", "UI.soft", pitch_range=(0.94, 1.1))
def mining_ore_tick(rng):
    P = dict(freqs=[1850.0, 2780.0, 4310.0], decays=[0.05, 0.03, 0.02], dur=0.16)
    y = modal(P["dur"], P["freqs"], P["decays"], [1, 0.5, 0.25], rng, excite_noise=0.2)
    return fade_edges(y, 0.0005, 0.01), P


@sound("Mining.CargoFull", "Mining", "UI")
def mining_cargo_full(rng):
    P = dict(notes=[987.77, 783.99, 587.33], note_step=0.16, decay=0.35, fm_index=0.9)
    y = np.zeros(int(1.2 * SR))
    for i, f in enumerate(P["notes"]):
        place(y, bell(0.6, f, P["decay"], 2.0, P["fm_index"]), i * P["note_step"])
    return reverb_tail(y, rng, 0.6, 0.2), P


@sound("Mining.AsteroidDepleted", "Mining", "World", attenuation="ATT_World")
def mining_asteroid_depleted(rng):
    P = dict(dur=1.8, crack_count=40, crack_spread=0.35, crack_lp=3500.0, boom=(60.0, 32.0, 0.7), boom_level=0.8,
             rubble_level=0.35, reverb_wet=0.3)
    n = int(P["dur"] * SR)
    imp = np.zeros(n)
    times = np.sort(rng.exponential(P["crack_spread"] / 3, P["crack_count"]))
    for i, tm in enumerate(times):
        k = int(tm * SR)
        if k < n:
            imp[k] += rng.uniform(0.3, 1.0) * np.exp(-i / 15)
    cracks = lowpass(signal.fftconvolve(imp, white(rng, 240) * np.exp(-np.arange(240) / 40))[:n], P["crack_lp"])
    cracks /= np.max(np.abs(cracks)) + 1e-12
    boom = pad(thump(P["dur"], *P["boom"]), P["dur"])
    rubble = lowpass(white(rng, n), 900) * env_ad(n, 0.05, 1.0)
    rubble /= np.max(np.abs(rubble)) + 1e-12
    y = cracks + P["boom_level"] * boom + P["rubble_level"] * rubble
    return reverb_tail(y, rng, 1.2, P["reverb_wet"], 3000), P


# ---------------------------------------------------------------------------
# TRADE
# ---------------------------------------------------------------------------

UI_AIR = 0.18  # level of the transient() tick at the front of UI and chime notes


def _chime(rng, notes, step, decay=0.4, index=0.9, ratio=2.0, wet=0.2, total=1.2):
    y = np.zeros(int(total * SR))
    for i, f in enumerate(notes):
        place(y, bell(decay * 1.8, f, decay, ratio, index), i * step)
        place(y, transient(rng), i * step, UI_AIR)
    return reverb_tail(y, rng, 0.6, wet, 8000)


@sound("Trade.Buy", "Trade", "UI")
def trade_buy(rng):
    P = dict(notes=[1046.5, 1568.0], step=0.09, decay=0.35, index=0.8)
    return _chime(rng, P["notes"], P["step"], P["decay"], P["index"]), P


@sound("Trade.Sell", "Trade", "UI")
def trade_sell(rng):
    P = dict(notes=[1568.0, 1046.5, 2093.0], step=0.07, decay=0.3, index=1.1)
    return _chime(rng, P["notes"], P["step"], P["decay"], P["index"], ratio=3.5), P


@sound("Trade.CreditsDing", "Trade", "UI.soft", pitch_range=(0.99, 1.01))
def trade_credits_ding(rng):
    P = dict(freq=1760.0, decay=0.4, ratio=3.5, index=1.0)
    y = bell(1.0, P["freq"], P["decay"], P["ratio"], P["index"])
    return reverb_tail(y, rng, 0.5, 0.18), P


def _soft_buzz(rng, f, dur, pulses, gap, lp=900.0, drop=0.9):
    y = np.zeros(int((pulses * (dur + gap) + 0.1) * SR))
    for i in range(pulses):
        n = int(dur * SR)
        t = np.arange(n) / SR
        ff = f * (1 + (drop - 1) * t / dur)
        ph = phase_from_freq(ff)
        tone = sum(((-1) ** (k + 1)) * np.sin(k * ph) / k for k in range(1, 9))  # soft saw
        tone += 0.6 * np.sin(phase_from_freq(ff * 1.012))
        place(y, lowpass(tone, lp) * env_adsr(n, 0.01, 0.05, 0.8, 0.06), i * (dur + gap))
        place(y, transient(rng, 0.006), i * (dur + gap), UI_AIR * 2.0)
    return fade_edges(reverb_tail(y, rng, 0.3, 0.1))


@sound("Trade.Denied", "Trade", "UI")
def trade_denied(rng):
    P = dict(freq=185.0, pulse=0.14, pulses=2, gap=0.05, lp=900.0)
    return _soft_buzz(rng, P["freq"], P["pulse"], P["pulses"], P["gap"], P["lp"]), P


# ---------------------------------------------------------------------------
# STATION EDITOR
# ---------------------------------------------------------------------------

@sound("Editor.Place.Small", "Editor", "UI", pitch_range=(0.96, 1.04))
def editor_place_small(rng):
    P = dict(thump=(190.0, 95.0, 0.12), click_freqs=[1200.0, 2650.0], click_level=0.3, air_level=0.3, reverb_wet=0.15)
    th = thump(0.6, *P["thump"])
    cl = modal(0.6, P["click_freqs"], [0.02, 0.012], [1, 0.5], rng, excite_noise=0.3)
    y = th + P["click_level"] * cl
    place(y, transient(rng, 0.008), 0.0, P["air_level"])
    return reverb_tail(y, rng, 0.4, P["reverb_wet"], 7000), P


@sound("Editor.Place.Large", "Editor", "UI", pitch_range=(0.96, 1.04))
def editor_place_large(rng):
    P = dict(thump=(95.0, 46.0, 0.35), clank=[160.0, 371.0, 690.0], clank_decay=[0.5, 0.3, 0.15],
             clank_level=0.4, air_level=0.4, reverb_wet=0.3, dur=1.8)
    th = thump(P["dur"], *P["thump"])
    cl = modal(P["dur"], P["clank"], P["clank_decay"], [1, 0.6, 0.3], rng, excite_noise=0.4)
    y = th + P["clank_level"] * cl / (np.max(np.abs(cl)) + 1e-12)
    place(y, transient(rng, 0.014, (3000.0, 16000.0)), 0.0, P["air_level"])
    return reverb_tail(y, rng, 0.9, P["reverb_wet"], 6000), P


@sound("Editor.Remove", "Editor", "UI")
def editor_remove(rng):
    P = dict(dur=0.45, sweep_from=400.0, sweep_to=3000.0, pop=(420.0, 160.0, 0.05), pop_at=0.3)
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    cut = P["sweep_from"] * (P["sweep_to"] / P["sweep_from"]) ** (t / P["dur"])
    wh = sweep_filter(white(rng, n), cut, "bandpass", q_width=0.5) * np.clip(t / P["pop_at"], 0, 1) ** 2
    wh *= np.where(t < P["pop_at"], 1.0, np.exp(-(t - P["pop_at"]) / 0.03))
    y = 0.5 * wh / (np.max(np.abs(wh)) + 1e-12)
    place(y, thump(0.15, *P["pop"]), P["pop_at"])
    return reverb_tail(fade_edges(y), rng, 0.3, 0.12), P


@sound("Editor.Invalid", "Editor", "UI")
def editor_invalid(rng):
    P = dict(freq=160.0, pulse=0.09, pulses=2, gap=0.04, lp=700.0)
    return _soft_buzz(rng, P["freq"], P["pulse"], P["pulses"], P["gap"], P["lp"], drop=1.0), P


def _blips(rng, notes, note_len=0.07, gap=0.015, index=0.5):
    y = np.zeros(int((len(notes) * (note_len + gap) + 0.3) * SR))
    for i, f in enumerate(notes):
        nn = int(note_len * SR)
        place(y, tail_taper(fm_tone(nn, f, 2.0, index) * env_ad(nn, 0.003, note_len * 0.8, 3)), i * (note_len + gap))
        place(y, transient(rng), i * (note_len + gap), UI_AIR)
    return reverb_tail(lowpass(y, 14000), rng, 0.3, 0.12, 8000)


@sound("Editor.Undo", "Editor", "UI.soft")
def editor_undo(rng):
    P = dict(notes=[880.0, 659.25], note_len=0.07)
    return _blips(rng, P["notes"], P["note_len"]), P


@sound("Editor.Redo", "Editor", "UI.soft")
def editor_redo(rng):
    P = dict(notes=[659.25, 880.0], note_len=0.07)
    return _blips(rng, P["notes"], P["note_len"]), P


@sound("Editor.Rotate", "Editor", "UI.soft", pitch_range=(0.95, 1.05))
def editor_rotate(rng):
    P = dict(freqs=[2400.0, 3900.0, 5200.0], decays=[0.012, 0.008, 0.005], ticks=2, tick_gap=0.028)
    y = np.zeros(int(0.12 * SR))
    for i in range(P["ticks"]):
        place(y, modal(0.05, P["freqs"], P["decays"], [1, 0.5, 0.3], rng, excite_noise=0.3) * (1 - 0.3 * i),
              i * P["tick_gap"])
    return fade_edges(lowpass(y, 8000), 0.0005, 0.01), P


@sound("Editor.Save", "Editor", "UI")
def editor_save(rng):
    P = dict(notes=[523.25, 659.25, 783.99, 1046.5], step=0.08, decay=0.35, index=0.7)
    return _chime(rng, P["notes"], P["step"], P["decay"], P["index"], total=1.4), P


# ---------------------------------------------------------------------------
# INTERIORS
# ---------------------------------------------------------------------------

FOOTSTEP_VARIATIONS = 4


def _footstep(rng, idx):
    P = dict(thud=(125.0, 80.0, 0.06), ring_base=[430.0, 1010.0, 1780.0, 2890.0], ring_decay=[0.12, 0.08, 0.05, 0.03],
             ring_level=0.25, scuff_level=0.35, variation=idx)
    jitter = rng.uniform(0.9, 1.1)
    th = thump(0.35, P["thud"][0] * jitter, P["thud"][1] * jitter, P["thud"][2])
    freqs = [f * rng.uniform(0.93, 1.07) for f in P["ring_base"]]
    ring = modal(0.35, freqs, P["ring_decay"], [1, 0.6, 0.35, 0.2], rng, excite_noise=0.2)
    n = th.size
    scuff = bandpass(white(rng, n), 1500, 7000) * env_ad(n, 0.002, 0.03)
    y = th + P["ring_level"] * ring / (np.max(np.abs(ring)) + 1e-12) + P["scuff_level"] * scuff
    return reverb_tail(fade_edges(y, 0.0005, 0.02), rng, 0.35, 0.15, 4000), P


for _i in range(1, FOOTSTEP_VARIATIONS + 1):
    sound(f"Interior.Footstep.{_i:02d}", "Interior", "Interior", pitch_range=(0.95, 1.05), attenuation="ATT_World")(
        lambda rng, _i=_i: _footstep(rng, _i))


@sound("Interior.Door", "Interior", "Interior", attenuation="ATT_World")
def interior_door(rng):
    P = dict(dur=1.1, hiss_band=(1500.0, 8000.0), hiss_level=0.4, slide_from=250.0, slide_to=900.0,
             slide_level=0.6, stop_at=0.75, stop_thud=(140.0, 70.0, 0.08))
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    hiss = bandpass(white(rng, n), *P["hiss_band"]) * env_ad(n, 0.01, 0.25, 3)
    cut = P["slide_from"] + (P["slide_to"] - P["slide_from"]) * np.clip(t / P["stop_at"], 0, 1)
    slide = sweep_filter(white(rng, n), cut, "bandpass", q_width=0.4)
    slide *= np.clip(t / 0.05, 0, 1) * np.where(t < P["stop_at"], 1.0, np.exp(-(t - P["stop_at"]) / 0.03))
    y = (P["hiss_level"] * hiss / (np.max(np.abs(hiss)) + 1e-12)
         + P["slide_level"] * slide / (np.max(np.abs(slide)) + 1e-12))
    place(y, thump(0.3, *P["stop_thud"]) * 0.8, P["stop_at"])
    return reverb_tail(fade_edges(y), rng, 0.4, 0.15), P


def _cockpit(rng, enter):
    P = dict(latch=[520.0, 1240.0, 2300.0], latch_decay=[0.08, 0.05, 0.03], tones=[392.0, 587.33, 783.99],
             tone_step=0.12, tone_decay=0.3, tone_level=0.45)
    y = np.zeros(int(1.4 * SR))
    latch = thump(0.3, 150.0, 80.0, 0.07) + 0.4 * modal(0.3, P["latch"], P["latch_decay"], [1, 0.5, 0.3], rng, 0.3)
    notes = P["tones"] if enter else P["tones"][::-1]
    start = 0.18 if enter else 0.0
    place(y, latch, 0.0 if enter else len(notes) * P["tone_step"] + 0.1)
    for i, f in enumerate(notes):
        place(y, P["tone_level"] * bell(0.5, f, P["tone_decay"], 2.0, 0.4), start + i * P["tone_step"])
    return reverb_tail(y, rng, 0.4, 0.12), P


@sound("Interior.CockpitEnter", "Interior", "Interior")
def interior_cockpit_enter(rng):
    return _cockpit(rng, True)


@sound("Interior.CockpitExit", "Interior", "Interior")
def interior_cockpit_exit(rng):
    return _cockpit(rng, False)


@sound("Interior.ShipHum", "Interior", "Ambient", loop=True)
def interior_ship_hum(rng):
    P = dict(f0=60.0, harmonics=6, rolloff=1.3, air_lp=1100.0, air_level=0.5, lfo_hz=0.25, lfo_depth=0.1,
             seconds=8.0, formant_hz=300.0, formant_gain=1.2, drive=1.2)

    def render(n):
        t = np.arange(n) / SR
        ph = phase_from_freq(P["f0"] * (1 + smooth_random(rng, n, 0.3, 0.002)))
        hum = sum((k ** -P["rolloff"]) * np.sin(k * ph + k) for k in range(1, P["harmonics"] + 1))
        air = lowpass(pink(rng, n), P["air_lp"])
        air /= np.std(air) + 1e-12
        y = presence(hum / 2, P["formant_hz"], P["formant_gain"], P["drive"]) / 2 + P["air_level"] * 0.3 * air
        return y * (1 - P["lfo_depth"] * (0.5 + 0.5 * np.sin(2 * np.pi * P["lfo_hz"] * t)))
    return make_loop(render, P["seconds"]), P


@sound("Interior.ConsoleChirp", "Interior", "UI.soft", pitch_range=(0.92, 1.08), attenuation="ATT_World")
def interior_console_chirp(rng):
    P = dict(notes=[2093.0, 2637.0], note_len=0.045, gap=0.02, index=0.8)
    notes = list(P["notes"])
    if rng.random() < 0.5:
        notes = notes[::-1]
    return _blips(rng, notes, P["note_len"], P["gap"], P["index"]), P


# ---------------------------------------------------------------------------
# UI
# ---------------------------------------------------------------------------

@sound("UI.Hover", "UI", "UI.soft", pitch_range=(0.98, 1.02))
def ui_hover(rng):
    P = dict(freq=2400.0, decay=0.018, dur=0.06)
    n = int(P["dur"] * SR)
    y = np.sin(2 * np.pi * P["freq"] * np.arange(n) / SR) * env_ad(n, 0.002, P["decay"])
    place(y, transient(rng, 0.003, (7000.0, 16000.0)), 0.0, UI_AIR)
    return fade_edges(y, 0.001, 0.005), P


@sound("UI.Click", "UI", "UI")
def ui_click(rng):
    P = dict(freq=1500.0, decay=0.03, dur=0.09, body=(600.0, 300.0, 0.02), body_level=0.4)
    n = int(P["dur"] * SR)
    y = fm_tone(n, P["freq"], 1.5, 0.6) * env_ad(n, 0.001, P["decay"])
    y += P["body_level"] * pad(thump(P["dur"], *P["body"]), P["dur"])
    return fade_edges(y, 0.0005, 0.005), P


def _ui_sweep(rng, up):
    P = dict(dur=0.32, notes=[659.25, 987.77], whoosh_from=800.0, whoosh_to=3500.0, whoosh_level=0.25)
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    a, b = (P["whoosh_from"], P["whoosh_to"]) if up else (P["whoosh_to"], P["whoosh_from"])
    cut = a * (b / a) ** (t / P["dur"])
    wh = sweep_filter(white(rng, n), cut, "bandpass", q_width=0.6) * env_adsr(n, 0.08, 0.1, 0.6, 0.12)
    y = P["whoosh_level"] * wh / (np.max(np.abs(wh)) + 1e-12)
    notes = P["notes"] if up else P["notes"][::-1]
    for i, f in enumerate(notes):
        place(y, 0.6 * bell(0.25, f, 0.12, 2.0, 0.5), 0.02 + i * 0.07)
    return reverb_tail(fade_edges(y), rng, 0.3, 0.12), P


@sound("UI.Open", "UI", "UI")
def ui_open(rng):
    return _ui_sweep(rng, True)


@sound("UI.Close", "UI", "UI")
def ui_close(rng):
    return _ui_sweep(rng, False)


@sound("UI.Toast", "UI", "UI")
def ui_toast(rng):
    P = dict(notes=[1318.5, 1975.5], step=0.1, decay=0.3, index=0.5)
    return _chime(rng, P["notes"], P["step"], P["decay"], P["index"], total=1.0), P


@sound("UI.QuickSave", "UI", "UI")
def ui_quicksave(rng):
    P = dict(notes=[783.99, 987.77, 1174.66], step=0.08, decay=0.3, index=0.6)
    return _chime(rng, P["notes"], P["step"], P["decay"], P["index"], total=1.1), P


@sound("UI.QuickLoad", "UI", "UI")
def ui_quickload(rng):
    P = dict(notes=[1174.66, 783.99, 1567.98], step=0.09, decay=0.3, index=0.6)
    return _chime(rng, P["notes"], P["step"], P["decay"], P["index"], total=1.1), P


@sound("UI.Error", "UI", "UI")
def ui_error(rng):
    P = dict(freq=220.0, pulse=0.11, pulses=2, gap=0.045, lp=1000.0)
    return _soft_buzz(rng, P["freq"], P["pulse"], P["pulses"], P["gap"], P["lp"], drop=0.95), P


# ---------------------------------------------------------------------------
# AMBIENCE
# ---------------------------------------------------------------------------

@sound("Ambient.Space", "Ambient", "Ambient", loop=True)
def ambient_space(rng):
    P = dict(bed_lp=180.0, bed_level=0.75, pad_freqs=[55.0, 82.41, 110.0, 164.81, 220.0, 329.63], pad_level=0.35,
             pad_lfo_hz=[0.05, 0.07, 0.09, 0.11, 0.13, 0.06], air_band=(3000.0, 9000.0), air_level=0.05, seconds=12.0)

    def render(n):
        t = np.arange(n) / SR
        bed = lowpass(brown(rng, n), P["bed_lp"])
        bed /= np.std(bed) + 1e-12
        pads = np.zeros(n)
        for f, lf in zip(P["pad_freqs"], P["pad_lfo_hz"]):
            amp = 0.5 + 0.5 * np.sin(2 * np.pi * lf * t + rng.uniform(0, 6.28))
            pads += amp * np.sin(phase_from_freq(f * (1 + smooth_random(rng, n, 0.1, 0.002))))
        air = bandpass(white(rng, n), *P["air_band"])
        air /= np.std(air) + 1e-12
        return P["bed_level"] * bed + P["pad_level"] * pads + P["air_level"] * air
    return make_loop(render, P["seconds"], xfade=1.5), P


@sound("Ambient.StationHum", "Ambient", "Ambient", loop=True, attenuation="ATT_StationHum")
def ambient_station_hum(rng):
    P = dict(f0=50.0, harmonics=[1, 2, 3, 4, 6], levels=[1.0, 0.6, 0.35, 0.2, 0.1], pulse_hz=0.5, pulse_depth=0.3,
             air_lp=1500.0, air_level=0.4, seconds=8.0, formant_hz=250.0, formant_gain=1.4, drive=1.4)

    def render(n):
        t = np.arange(n) / SR
        ph = phase_from_freq(P["f0"] * (1 + smooth_random(rng, n, 0.2, 0.002)))
        hum = sum(l * np.sin(h * ph + h) for h, l in zip(P["harmonics"], P["levels"]))
        hum *= 1 - P["pulse_depth"] * (0.5 + 0.5 * np.sin(2 * np.pi * P["pulse_hz"] * t))
        air = lowpass(pink(rng, n), P["air_lp"])
        air /= np.std(air) + 1e-12
        hum = presence(hum / 2, P["formant_hz"], P["formant_gain"], P["drive"]) / 2
        return hum + P["air_level"] * 0.3 * air
    return make_loop(render, P["seconds"]), P


@sound("Ambient.AsteroidCreak", "Ambient", "Ambient", pitch_range=(0.85, 1.1), attenuation="ATT_World")
def ambient_asteroid_creak(rng):
    P = dict(dur=2.6, rate_from=18.0, rate_to=45.0, res_freqs=[230.0, 410.0, 610.0], res_q=12.0, rumble_level=0.4,
             reverb_wet=0.35)
    n = int(P["dur"] * SR)
    t = np.arange(n) / SR
    rate = P["rate_from"] + (P["rate_to"] - P["rate_from"]) * (0.5 - 0.5 * np.cos(2 * np.pi * t / P["dur"]))
    rate *= 1 + 0.3 * smooth_random(rng, n, 4)
    ph = np.cumsum(rate) / SR
    imp = np.zeros(n)
    idx = np.nonzero(np.diff(np.floor(ph)) > 0)[0]
    imp[idx] = rng.uniform(0.4, 1.0, idx.size)
    creak = np.zeros(n)
    for f in P["res_freqs"]:
        b, a = signal.iirpeak(f, P["res_q"], fs=SR)
        creak += signal.lfilter(b, a, imp)
    creak *= env_adsr(n, 0.3, 0.2, 0.8, 0.9)
    creak /= np.max(np.abs(creak)) + 1e-12
    rumble = lowpass(brown(rng, n), 120) * env_adsr(n, 0.5, 0.2, 0.7, 1.0)
    rumble /= np.max(np.abs(rumble)) + 1e-12
    y = creak + P["rumble_level"] * rumble
    return reverb_tail(fade_edges(y, 0.02, 0.1), rng, 1.5, P["reverb_wet"], 2500), P


@sound("Ambient.MapRoomTone", "Ambient", -33.0, loop=True)
def ambient_map_room_tone(rng):
    P = dict(noise_lp=1800.0, hum_hz=120.0, hum_level=0.12, seconds=8.0)

    def render(n):
        t = np.arange(n) / SR
        nz = lowpass(pink(rng, n), P["noise_lp"])
        nz /= np.std(nz) + 1e-12
        return nz + P["hum_level"] * np.sin(2 * np.pi * P["hum_hz"] * t)
    return make_loop(render, P["seconds"]), P


# ---------------------------------------------------------------------------
# Catalog-only entries (no file of their own)
# ---------------------------------------------------------------------------
# Interior.Footstep plays a random variation; the catalog entry points at .01 with the rest as Variations.
GROUPS = {
    "Interior.Footstep": [f"Interior.Footstep.{i:02d}" for i in range(1, FOOTSTEP_VARIATIONS + 1)],
}


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def file_stem(event_id: str) -> str:
    return event_id.replace(".", "_")


def _jsonable(v):
    if isinstance(v, (np.floating, np.integer)):
        return v.item()
    if isinstance(v, (list, tuple)):
        return [_jsonable(i) for i in v]
    if isinstance(v, dict):
        return {k: _jsonable(i) for k, i in v.items()}
    return v


def generate(event_id: str) -> tuple[np.ndarray, dict]:
    spec = SOUNDS[event_id]
    seed = seed_for(event_id)
    rng = np.random.default_rng(seed)
    x, params = spec["fn"](rng)
    x = np.asarray(x, dtype=np.float64)
    if not spec["loop"]:
        x = fade_edges(highpass(x, 20), 0.0005, 0.005)
    x = normalise(x, spec["target"], spec["loop"])
    return x, dict(seed=seed, params=_jsonable(params))


def write_wav(path: Path, x: np.ndarray):
    path.parent.mkdir(parents=True, exist_ok=True)
    pcm = np.clip(np.round(x * 32767.0), -32768, 32767).astype(np.int16)
    wavfile.write(str(path), SR, pcm)


def main(argv):
    prefixes = argv[1:]
    manifest = {"sample_rate": SR, "bit_depth": 16, "channels": 1, "peak_ceiling_dbfs": PEAK_CEILING_DBFS,
                "loudness_method": "BS.1770-style K-weighted gated integrated loudness; 100 ms blocks for sounds under 1 s "
                                   "(see generate_sfx.integrated_loudness)",
                "sounds": []}
    old = {}
    if prefixes and MANIFEST_PATH.exists():
        old = {s["id"]: s for s in json.loads(MANIFEST_PATH.read_text())["sounds"]}

    for eid, spec in SOUNDS.items():
        if prefixes and not any(eid.startswith(p) for p in prefixes):
            if eid in old:
                manifest["sounds"].append(old[eid])
            continue
        x, meta = generate(eid)
        rel = f"{spec['category']}/{file_stem(eid)}.wav"
        write_wav(OUT_DIR / rel, x)
        pcm = np.round(x * 32767.0) / 32767.0
        entry = dict(
            id=eid, file=rel, category=spec["category"], asset_name=f"SW_{file_stem(eid)}",
            sound_class=CATEGORY_SOUND_CLASS[spec["category"]], attenuation=spec["attenuation"],
            loop=spec["loop"], loop_start=0, loop_end=int(x.size) if spec["loop"] else None,
            duration_s=round(x.size / SR, 4), num_samples=int(x.size),
            target_lufs=spec["target"], measured_lufs=round(integrated_loudness(pcm), 2),
            peak_dbfs=round(peak_dbfs(pcm), 2), volume=spec["volume"], pitch_range=spec["pitch_range"],
            seed=meta["seed"], params=meta["params"],
        )
        manifest["sounds"].append(entry)
        print(f"{eid:28s} {x.size / SR:6.2f}s  {entry['measured_lufs']:6.1f} LUFS (target {spec['target']:.0f})  "
              f"peak {entry['peak_dbfs']:5.1f} dBFS{'  loop' if spec['loop'] else ''}")

    manifest["groups"] = [
        dict(id=gid, category=SOUNDS[members[0]]["category"], members=members,
             sound_class=CATEGORY_SOUND_CLASS[SOUNDS[members[0]]["category"]],
             attenuation=SOUNDS[members[0]]["attenuation"], pitch_range=SOUNDS[members[0]]["pitch_range"])
        for gid, members in GROUPS.items()
    ]
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"\n{len(manifest['sounds'])} sounds + {len(GROUPS)} groups -> {MANIFEST_PATH}")


if __name__ == "__main__":
    main(sys.argv)
