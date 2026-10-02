"""Generate the galaxy overview map: docs/galaxy/galaxy_overview.html (+ galaxy_map.svg).

Charted systems and sectors come from Content/Data/Universe/Galaxy.json; systems and sectors the
galaxy plan adds later (docs/11-TECHNICAL_SPECS/GALAXY_PLAN.md) come from PLANNED below. Positions
of planned systems are placeholders in light-years, chosen to sit in their region's direction.

Usage: python Tools/gen_galaxy_overview.py
"""
from __future__ import annotations

import html
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "docs/galaxy"

# Systems the plan adds. sectors: (name, type, security, hazard, hazards text)
PLANNED = {
    "meridian": dict(name="Meridian", star="G8V", region="compact_core", pos=(5, -4), wave=1,
                     desc="Food and habitat: the garden world's orbit, farms and the biggest population. The most valuable system there is, because people are.",
                     links=["adastrea", "lumen"],
                     sectors=[("Meridian Gardens", "Farming", "High", "None", ""),
                              ("Meridian Hab", "Habitat", "High", "None", ""),
                              ("Meridian Reach", "Ice", "Medium", "Mild", "Debris")]),
    "halt": dict(name="Halt", star="K2V", region="varos_marches", pos=(-14, 11), wave=1,
                 desc="The Syndicate's toll gate between the Core and the Graveyard. Every ship is scanned and charged.",
                 links=["varos", "nyx", "ashfall"],
                 sectors=[("Halt Gate", "Toll Gate", "Low", "None", ""),
                          ("Halt Shoals", "Smuggling", "None", "Severe", "Debris")]),
    "brand": dict(name="Brand", star="M2V", region="varos_marches", pos=(-4, 15), wave=2,
                  desc="Syndicate shipyards and refit docks: the only warships for sale outside the Core.",
                  links=["varos"],
                  sectors=[("Brand Yards", "Shipyard", "Low", "None", ""),
                           ("Brand Anchorage", "Military", "Low", "Mild", "Debris")]),
    "tamsin": dict(name="Tamsin", star="K7V", region="free_belts", pos=(15, -17), wave=2,
                   desc="Comets and ice: water and volatiles for half the galaxy.",
                   links=["corvid", "orrin"],
                   sectors=[("Tamsin Comets", "Ice", "Medium", "Mild", "Debris"),
                            ("Tamsin Drift", "Mining", "Low", "Mild", "Debris"),
                            ("Tamsin Halo", "Ice", "Low", "Severe", "Debris, Distance")]),
    "orrin": dict(name="Orrin", star="G5V", region="free_belts", pos=(18, -7), wave=2,
                  desc="A cluster of gas giants: bulk hydrogen, methane and nitrogen, Helium-3 in the storm bands.",
                  links=["kestrel", "tamsin"],
                  sectors=[("Orrin Skim", "Gas Harvesting", "Medium", "Mild", "Gas storms"),
                           ("Orrin Storms", "Gas Harvesting", "Low", "Severe", "Gas storms"),
                           ("Orrin Depot", "Fuel Depot", "Medium", "None", "")]),
    "ashfall": dict(name="Ashfall", star="M0V", region="graveyard", pos=(-15, 20), wave=2,
                    desc="Where two empire fleets died. Salvage everywhere, and the mines are still armed.",
                    links=["nyx", "requiem"],
                    sectors=[("Ashfall Field", "Salvage", "None", "Severe", "Minefield, Debris"),
                             ("Ashfall Hulks", "Derelicts", "None", "Severe", "War machines"),
                             ("Ashfall Mines", "Minefield", "None", "Extreme", "Minefield")]),
    "requiem": dict(name="Requiem", star="K0III", region="graveyard", pos=(-24, 22), wave=2,
                    desc="A dead empire fortress world with its defence platforms still awake. The hardest place in the galaxy and the best old-war tech.",
                    links=["ashfall"],
                    sectors=[("Requiem Approach", "War Platforms", "None", "Extreme", "War machines, Minefield"),
                             ("Requiem Fortress", "Derelict Fortress", "None", "Extreme", "War machines")]),
    "vigil": dict(name="Vigil", star="A1V", region="pale_expanse", pos=(28, 7), wave=3,
                  desc="An unexplained beacon that nobody built.",
                  links=["thule", "hollow"],
                  sectors=[("Vigil Beacon", "Anomaly", "None", "Severe", "Distance"),
                           ("Vigil Reach", "Exploration", "None", "Severe", "Radiation storms, Distance")]),
    "hollow": dict(name="Hollow", star="rogue mass", region="pale_expanse", pos=(31, -3), wave=3,
                   desc="A rogue planet or dark mass that bends the lanes. Gravity tides and the rarest ores.",
                   links=["vigil"],
                   sectors=[("Hollow Rim", "Mining", "None", "Extreme", "Gravity tides"),
                            ("Hollow Deep", "Anomaly", "None", "Extreme", "Gravity tides, Distance")]),
    "murk": dict(name="Murk", star="M5V", region="sable_dark", pos=(-23, -1), wave=3,
                 desc="A dark nebula where sensors are nearly blind. Pirate ambushes.",
                 links=["sable"], hidden=["halt"],
                 sectors=[("Murk Shallows", "Nebula", "None", "Severe", "Nebula"),
                          ("Murk Deep", "Pirate Base", "None", "Severe", "Nebula")]),
    "lantern": dict(name="Lantern", star="K4V", region="sable_dark", pos=(-22, -12), wave=3,
                    desc="The pirates' free town: fences, refits, re-registering stolen ships. A guild hub ship is often parked here.",
                    links=["sable"], hidden=["lumen"],
                    sectors=[("Lantern Town", "Free Town", "None", "Mild", "Nebula"),
                             ("Lantern Yards", "Refit Yards", "None", "Mild", "Nebula")]),
}
# Sectors the plan adds to systems that already exist.
PLANNED_SECTORS = {
    "lumen": [("Lumen Shade", "Farming", "Medium", "None", "")],
    "nyx": [("Nyx Storm", "Radiation", "None", "Extreme", "Radiation storms")],
    "thule": [("Thule Corona", "Gas Harvesting", "None", "Extreme", "Heat, Radiation storms")],
    "sable": [("Sable Haven", "Pirate Base", "None", "Mild", "Nebula"),
              ("Sable Veil", "Nebula", "None", "Severe", "Nebula")],
}
# Region one-liners (from the plan's "danger by region" and resources tables).
REGION_NOTES = {
    "compact_core": ("High → Medium", "None → Mild", "Iron, copper, nickel, silicon, ice, food, energy", "Rare metals, Helium-3, salvage",
                     "Small dangers: debris at Cinder, smugglers at the edges."),
    "varos_marches": ("Low → None", "Mild → Severe", "Scrap, salvage, hull plate, zinc, manganese", "Food, water, fuel",
                      "The Syndicate takes its cut; the Gauntlet is a fight."),
    "free_belts": ("Medium → Low", "Mild → Severe", "Titanium, cobalt, tungsten, chromium, lithium, ice, bulk gases", "Food, components, people",
                   "The space itself: tides, ice, storms. Pirates prey on miners."),
    "graveyard": ("None", "Severe → Extreme", "Salvage of every kind, derelict tech, uranium", "Everything. Nobody lives there.",
                  "Old war machines disable you and nobody comes."),
    "pale_expanse": ("None", "Severe → Extreme", "Platinum, palladium, gold, silver, rare earths, carbon crystal, Helium-3", "Everything",
                     "Radiation, heat and distance. Get stranded, stay stranded."),
    "sable_dark": ("None", "Mild → Severe", "Little of its own. It takes what it needs.", "People, fuel, parts",
                   "Pirate home space. Capture is likely."),
}


def hazards_text(sec: dict) -> str:
    parts = []
    for h in sec.get("hazards", []):
        t = h["type"]
        parts.append({"RadiationStorm": "Radiation storms", "GravityTide": "Gravity tides", "GasStorm": "Gas storms",
                      "WarMachines": "War machines"}.get(t, t))
    return ", ".join(parts)


def build_data() -> dict:
    galaxy = json.loads((ROOT / "Content/Data/Universe/Galaxy.json").read_text(encoding="utf-8"))
    regions = {r["id"]: dict(name=r["name"], type=r["type"], color=r["color"], desc=r["description"]) for r in galaxy["regions"]}
    systems = {}
    links = set()
    for s in galaxy["systems"]:
        secs = [dict(name=x["name"], type=x["type"], security=x.get("security", "None"), hazard=x.get("hazard", "None"),
                     hazards=hazards_text(x), built=bool(x.get("level")), planned=False,
                     resources=", ".join(r["item"] for r in x.get("resources", [])))
                for x in s.get("sectors", [])]
        for name, typ, sec, haz, hz in PLANNED_SECTORS.get(s["id"], []):
            secs.append(dict(name=name, type=typ, security=sec, hazard=haz, hazards=hz, built=False, planned=True, resources=""))
        systems[s["id"]] = dict(name=s["name"], star=s.get("starClass", ""), region=s.get("region", ""), pos=s["position"],
                                charted=True, wave=1, desc=s.get("description", ""), sectors=secs,
                                color=s.get("starColor", [1, 0.9, 0.7]))
        for l in s.get("jumpLinks", []):
            links.add(tuple(sorted((s["id"], l))) + ("lane",))
    for sid, p in PLANNED.items():
        systems[sid] = dict(name=p["name"], star=p["star"], region=p["region"], pos=list(p["pos"]), charted=False,
                            wave=p["wave"], desc=p["desc"], color=[0.85, 0.85, 0.9],
                            sectors=[dict(name=n, type=t, security=sc, hazard=hz, hazards=h, built=False, planned=True, resources="")
                                     for n, t, sc, hz, h in p["sectors"]])
        for l in p.get("links", []):
            links.add(tuple(sorted((sid, l))) + ("planned",))
        for l in p.get("hidden", []):
            links.add(tuple(sorted((sid, l))) + ("hidden",))
    # A charted lane wins over a planned one between the same pair.
    best = {}
    rank = {"lane": 0, "planned": 1, "hidden": 2}
    for a, b, kind in links:
        if (a, b) not in best or rank[kind] < rank[best[(a, b)]]:
            best[(a, b)] = kind
    # Sable has no lane yet: its wave-2 lane is planned from Murk/Lantern only.
    systems["sable"]["wave"] = 2
    return dict(regions=regions, systems=systems, lanes=[[a, b, k] for (a, b), k in best.items()], notes=REGION_NOTES)


# ---------------------------------------------------------------------------- SVG

SCALE = 22.0
X0, X1, Y0, Y1 = -30.0, 36.0, -22.0, 27.0
W, H = (X1 - X0) * SCALE, (Y1 - Y0) * SCALE
SEC_COL = {"High": "#4fd68a", "Medium": "#d6dc58", "Low": "#ff9a42", "None": "#ff5a4a"}
HAZ_COL = {"None": "#6f7f8f", "Mild": "#ffd34d", "Severe": "#ff7433", "Extreme": "#ff3d9a"}


# Region label positions (ly) where the automatic spot is ambiguous.
LABEL_AT = {"varos_marches": (2.5, 19.5)}


def px(pos):
    return ((pos[0] - X0) * SCALE, (Y1 - pos[1]) * SCALE)


def rgb(c, a=1.0):
    return f"rgba({int(c[0]*255)},{int(c[1]*255)},{int(c[2]*255)},{a})"


def worst(levels, order):
    return max(levels, key=order.index) if levels else order[0]


def svg(data: dict) -> str:
    e = html.escape
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W:.0f} {H:.0f}" role="img" '
           f'aria-label="Galaxy map: six regions, {len(data["systems"])} systems">']
    out.append('<rect width="100%" height="100%" fill="#070a12"/>')
    # Light-year grid every 5 ly.
    g = []
    x = int(X0 // 5 * 5) + 5
    while x < X1:
        g.append(f'<line x1="{px((x,0))[0]:.1f}" y1="0" x2="{px((x,0))[0]:.1f}" y2="{H:.0f}"/>'); x += 5
    y = int(Y0 // 5 * 5) + 5
    while y < Y1:
        g.append(f'<line x1="0" y1="{px((0,y))[1]:.1f}" x2="{W:.0f}" y2="{px((0,y))[1]:.1f}"/>'); y += 5
    out.append(f'<g stroke="#16203a" stroke-width="1">{"".join(g)}</g>')
    # Region territories: union of discs, drawn opaque inside a translucent group so overlaps don't stack.
    for rid, r in data["regions"].items():
        members = [s for s in data["systems"].values() if s["region"] == rid]
        if not members:
            continue
        discs = "".join(f'<circle cx="{px(s["pos"])[0]:.1f}" cy="{px(s["pos"])[1]:.1f}" r="{5.2*SCALE:.0f}"/>' for s in members)
        out.append(f'<g class="terr" data-region="{rid}" fill="{rgb(r["color"])}" opacity="0.13">{discs}</g>')
        edge = "".join(f'<circle cx="{px(s["pos"])[0]:.1f}" cy="{px(s["pos"])[1]:.1f}" r="{5.2*SCALE:.0f}"/>' for s in members)
        out.append(f'<g fill="none" stroke="{rgb(r["color"])}" stroke-width="1" stroke-dasharray="2 5" opacity="0.45">{edge}</g>')
    # Region names, above the region's top-most system.
    for rid, r in data["regions"].items():
        members = [px(s["pos"]) for s in data["systems"].values() if s["region"] == rid]
        if not members:
            continue
        cx = sum(m[0] for m in members) / len(members)
        top = min(m[1] for m in members) - 5.2 * SCALE + 16
        if rid in LABEL_AT:  # hand-placed where the centroid lands in a neighbour's territory
            cx, top = px(LABEL_AT[rid])
        out.append(f'<text class="rname" x="{cx:.0f}" y="{max(top, 22):.0f}" fill="{rgb(r["color"])}" text-anchor="middle">'
                   f'{e(r["name"].upper())}</text>')
    # Lanes.
    for a, b, kind in data["lanes"]:
        pa, pb = px(data["systems"][a]["pos"]), px(data["systems"][b]["pos"])
        style = {"lane": 'stroke="#7fb8d6" stroke-width="2" opacity="0.85"',
                 "planned": 'stroke="#7fb8d6" stroke-width="1.5" stroke-dasharray="7 5" opacity="0.55"',
                 "hidden": 'stroke="#c07bff" stroke-width="1.5" stroke-dasharray="2 6" opacity="0.8"'}[kind]
        out.append(f'<line x1="{pa[0]:.1f}" y1="{pa[1]:.1f}" x2="{pb[0]:.1f}" y2="{pb[1]:.1f}" {style}/>')
    # Systems.
    hz_order = ["None", "Mild", "Severe", "Extreme"]
    sec_order = ["None", "Low", "Medium", "High"]
    for sid, s in data["systems"].items():
        x, y = px(s["pos"])
        n = len(s["sectors"])
        r = 6 + n * 1.3
        haz = worst([z["hazard"] for z in s["sectors"]], hz_order)
        sec = worst([z["security"] for z in s["sectors"]], sec_order)
        out.append(f'<g class="sys" data-id="{sid}" tabindex="0" role="button" aria-label="{e(s["name"])}">')
        out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r+12:.1f}" fill="transparent"/>')
        if haz != "None":
            out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r+6:.1f}" fill="none" stroke="{HAZ_COL[haz]}" '
                       f'stroke-width="{1 + hz_order.index(haz)}" opacity="0.8"/>')
        if s["charted"]:
            out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r*2:.1f}" fill="{rgb(s["color"], 0.12)}"/>')
            out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r:.1f}" fill="{rgb(s["color"])}"/>')
        else:
            out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r:.1f}" fill="#070a12" stroke="#c8d2e6" stroke-width="1.5" stroke-dasharray="3 3"/>')
        out.append(f'<text class="sname" x="{x:.1f}" y="{y + r + 20:.1f}" text-anchor="middle">{e(s["name"].upper())}</text>')
        built = sum(1 for z in s["sectors"] if z["built"])
        sub = f'{n} sectors' + (f' · {built} built' if built else '') + ('' if s["charted"] else f' · wave {s["wave"]}')
        out.append(f'<text class="ssub" x="{x:.1f}" y="{y + r + 34:.1f}" text-anchor="middle">{e(sub)}</text>')
        out.append(f'<rect x="{x - 14:.1f}" y="{y - r - 15:.1f}" width="28" height="5" rx="1" fill="{SEC_COL[sec]}" opacity="0.9"/>')
        out.append('</g>')
    # Scale bar.
    out.append(f'<g class="scale"><line x1="20" y1="{H-24:.0f}" x2="{20+10*SCALE:.0f}" y2="{H-24:.0f}" stroke="#8a9bb8" stroke-width="2"/>'
               f'<text x="{24+10*SCALE:.0f}" y="{H-20:.0f}" fill="#8a9bb8">10 ly</text></g>')
    out.append('</svg>')
    return "".join(out)


# ---------------------------------------------------------------------------- HTML

PAGE = r"""<title>Adastrea Galaxy Map</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Oxanium:wght@500;700&family=Barlow:wght@400;500;600&family=JetBrains+Mono:wght@400;500&display=swap">
<style>
/* Layout: a star chart first; the selected system's details beside it (below it on phones), region cards after. */
:root {
  --void: #070a12; --panel: #0d1322; --line: #1d2945; --ink: #dfe6f3; --dim: #8a9bb8; --accent: #7fb8d6;
  --display: "Oxanium", "Segoe UI", system-ui, sans-serif;
  --body: "Barlow", "Segoe UI", system-ui, sans-serif;
  --mono: "JetBrains Mono", ui-monospace, Consolas, monospace;
  color-scheme: dark;
}
html, body { background: var(--void); color: var(--ink); }
body { font: 15px/1.5 var(--body); padding-inline: 16px; padding-block: 20px 40px; }
.wrap { max-width: 1400px; margin: 0 auto; display: grid; gap: 22px; }
header { display: flex; flex-wrap: wrap; align-items: baseline; gap: 6px 18px; }
h1 { font: 700 26px/1.1 var(--display); letter-spacing: .06em; margin: 0; text-wrap: balance; }
.lede { color: var(--dim); margin: 0; max-width: 70ch; }
.stats { display: flex; flex-wrap: wrap; gap: 6px 20px; font: 500 13px var(--mono); color: var(--dim); }
.stats b { color: var(--ink); font-weight: 500; }
.main { display: grid; grid-template-columns: minmax(0, 1fr) 340px; gap: 18px; align-items: start; }
@media (max-width: 960px) { .main { grid-template-columns: minmax(0, 1fr); } }
.map { border: 1px solid var(--line); border-radius: 6px; overflow: hidden; background: var(--void); min-width: 0; }
.map svg { display: block; width: 100%; height: auto; }
.rname { font: 700 15px var(--display); letter-spacing: .32em; opacity: .9; }
.sname { font: 600 12px var(--display); letter-spacing: .12em; fill: var(--ink); }
.ssub { font: 400 10.5px var(--mono); fill: var(--dim); }
.scale text { font: 11px var(--mono); }
.sys { cursor: pointer; outline: none; }
.sys:hover .sname, .sys:focus-visible .sname, .sys.on .sname { fill: #fff; }
.sys.on circle:nth-child(1), .sys:focus-visible circle:nth-child(1) { stroke: #fff; stroke-width: 1.5; }
.side { background: var(--panel); border: 1px solid var(--line); border-radius: 6px; padding: 16px; display: grid; gap: 12px; min-width: 0; }
.side h2 { font: 700 20px var(--display); letter-spacing: .08em; margin: 0; }
.chip { display: inline-block; font: 500 11px var(--mono); letter-spacing: .06em; padding: 2px 7px; border-radius: 3px; border: 1px solid currentColor; }
.meta { font: 13px var(--mono); color: var(--dim); }
.secs { display: grid; gap: 8px; margin: 0; padding: 0; list-style: none; }
.secs li { border-top: 1px solid var(--line); padding-top: 8px; display: grid; gap: 2px; }
.secs .nm { font-weight: 600; display: flex; justify-content: space-between; gap: 8px; }
.secs .tag { font: 11px var(--mono); color: var(--dim); white-space: nowrap; }
.secs .row { font: 12px var(--mono); display: flex; flex-wrap: wrap; gap: 4px 12px; }
.legend { display: grid; gap: 7px; font-size: 13px; }
.legend div { display: flex; align-items: center; gap: 10px; }
.legend svg { flex: none; }
.legend h3 { font: 600 12px var(--display); letter-spacing: .16em; color: var(--dim); margin: 6px 0 0; }
.regions { display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 14px; }
.region { background: var(--panel); border: 1px solid var(--line); border-radius: 6px; padding: 14px 16px; display: grid; gap: 8px; align-content: start; min-width: 0; }
.region h3 { margin: 0; font: 700 16px var(--display); letter-spacing: .1em; display: flex; align-items: center; gap: 10px; }
.region .sw { width: 12px; height: 12px; border-radius: 50%; flex: none; }
.region p { margin: 0; color: var(--dim); }
.region dl { margin: 0; display: grid; grid-template-columns: 92px minmax(0, 1fr); gap: 4px 10px; font-size: 13.5px; }
.region dt { color: var(--dim); font: 12px var(--mono); padding-top: 2px; }
.region dd { margin: 0; }
.region .sys-list { font: 12.5px var(--mono); color: var(--accent); }
h2.sec { font: 700 15px var(--display); letter-spacing: .2em; color: var(--dim); margin: 6px 0 0; }
.foot { color: var(--dim); font-size: 13px; max-width: 80ch; }
@media (prefers-reduced-motion: no-preference) { .sys circle { transition: stroke .15s; } }
</style>

<div class="wrap">
  <header>
    <h1>ADASTREA GALAXY</h1>
    <p class="lede">The remote galaxy far from the old empires' wars, split into six regions. Solid stars are charted today; hollow ones are planned. Click a system for its sectors.</p>
    <div class="stats">__STATS__</div>
  </header>

  <div class="main">
    <div class="map">__SVG__</div>
    <aside class="side" id="side" aria-live="polite"></aside>
  </div>

  <h2 class="sec">REGIONS</h2>
  <div class="regions">__REGIONS__</div>

  <p class="foot">Generated by Tools/gen_galaxy_overview.py from Galaxy.json and the galaxy plan (docs/11-TECHNICAL_SPECS/GALAXY_PLAN.md). Names and positions of planned systems are placeholders. Security is how fast help comes; hazard is what the place itself does to you.</p>
</div>

<script>
const DATA = __DATA__;
const SEC = {High:"#4fd68a", Medium:"#d6dc58", Low:"#ff9a42", None:"#ff5a4a"};
const HAZ = {None:"#6f7f8f", Mild:"#ffd34d", Severe:"#ff7433", Extreme:"#ff3d9a"};
const side = document.getElementById("side");
const esc = s => String(s).replace(/[&<>"]/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;"}[c]));
const legend = `<div class="legend">
  <h3>MAP KEY</h3>
  <div><svg width="22" height="22"><circle cx="11" cy="11" r="7" fill="#ffe0a0"/></svg>Charted system</div>
  <div><svg width="22" height="22"><circle cx="11" cy="11" r="7" fill="none" stroke="#c8d2e6" stroke-dasharray="3 3"/></svg>Planned system (wave 1, 2 or 3)</div>
  <div><svg width="34" height="10"><line x1="0" y1="5" x2="34" y2="5" stroke="#7fb8d6" stroke-width="2"/></svg>Jump lane</div>
  <div><svg width="34" height="10"><line x1="0" y1="5" x2="34" y2="5" stroke="#7fb8d6" stroke-dasharray="7 5"/></svg>Planned lane</div>
  <div><svg width="34" height="10"><line x1="0" y1="5" x2="34" y2="5" stroke="#c07bff" stroke-dasharray="2 6" stroke-width="1.5"/></svg>Hidden lane (found by players)</div>
  <h3>BAR ABOVE A STAR: SECURITY (BEST SECTOR)</h3>
  <div>${["High","Medium","Low","None"].map(k=>`<span class="chip" style="color:${SEC[k]}">${k}</span>`).join(" ")}</div>
  <h3>RING AROUND A STAR: WORST HAZARD</h3>
  <div>${["Mild","Severe","Extreme"].map(k=>`<span class="chip" style="color:${HAZ[k]}">${k}</span>`).join(" ")}</div>
</div>`;
function show(id) {
  document.querySelectorAll(".sys").forEach(g => g.classList.toggle("on", g.dataset.id === id));
  if (!id) { side.innerHTML = `<h2>KNOWN SPACE</h2><p class="meta">Click a system on the map.</p>` + legend; return; }
  const s = DATA.systems[id], r = DATA.regions[s.region] || {name:"", color:[.6,.6,.6]};
  const rc = `rgb(${r.color.map(v=>Math.round(v*255)).join(",")})`;
  const secs = s.sectors.map(z => `<li>
      <div class="nm"><span>${esc(z.name)}</span><span class="tag">${z.built ? "level built" : z.planned ? "planned" : "charted, no level"}</span></div>
      <div class="row"><span>${esc(z.type)}</span><span style="color:${SEC[z.security]}">Sec ${z.security}</span><span style="color:${HAZ[z.hazard]}">Hazard ${z.hazard}</span></div>
      ${z.hazards ? `<div class="row" style="color:var(--dim)">${esc(z.hazards)}</div>` : ""}
      ${z.resources ? `<div class="row" style="color:var(--dim)">${esc(z.resources.replace(/([a-z])([A-Z])/g,"$1 $2"))}</div>` : ""}
    </li>`).join("");
  side.innerHTML = `<div><h2>${esc(s.name.toUpperCase())}</h2>
      <div class="meta">${esc(s.star)} · ${s.charted ? "charted" : "planned, wave " + s.wave} · ${s.pos[0]}, ${s.pos[1]} ly</div></div>
    <div><span class="chip" style="color:${rc}">${esc(r.name)} · ${esc(r.type || "")}</span></div>
    <p style="margin:0">${esc(s.desc)}</p>
    <ul class="secs">${secs}</ul>` + legend;
}
document.querySelectorAll(".sys").forEach(g => {
  g.addEventListener("click", () => show(g.dataset.id));
  g.addEventListener("keydown", e => { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); show(g.dataset.id); } });
});
show("adastrea");
</script>
"""


def regions_html(data: dict) -> str:
    out = []
    for rid, r in data["regions"].items():
        sec, haz, rich, short, danger = data["notes"][rid]
        members = [s for s in data["systems"].values() if s["region"] == rid]
        names = ", ".join(s["name"] + ("" if s["charted"] else "*") for s in members)
        n_sec = sum(len(s["sectors"]) for s in members)
        out.append(f'''<section class="region"><h3><span class="sw" style="background:{rgb(r["color"])}"></span>{html.escape(r["name"].upper())}</h3>
<p>{html.escape(r["desc"])}</p><dl>
<dt>Type</dt><dd>{html.escape(r["type"])}</dd>
<dt>Systems</dt><dd class="sys-list">{html.escape(names)} <span style="color:var(--dim)">({n_sec} sectors)</span></dd>
<dt>Security</dt><dd>{html.escape(sec)}</dd><dt>Hazard</dt><dd>{html.escape(haz)}</dd>
<dt>Rich in</dt><dd>{html.escape(rich)}</dd><dt>Short of</dt><dd>{html.escape(short)}</dd>
<dt>Danger</dt><dd>{html.escape(danger)}</dd></dl></section>''')
    return "".join(out)


def main() -> None:
    data = build_data()
    systems = data["systems"].values()
    n_sec = sum(len(s["sectors"]) for s in systems)
    stats = (f'<span><b>6</b> regions</span><span><b>{len(data["systems"])}</b> systems '
             f'({sum(1 for s in systems if s["charted"])} charted)</span>'
             f'<span><b>{n_sec}</b> sectors ({sum(1 for s in systems for z in s["sectors"] if not z["planned"])} charted, '
             f'{sum(1 for s in systems for z in s["sectors"] if z["built"])} with a level)</span>')
    map_svg = svg(data)
    page = (PAGE.replace("__SVG__", map_svg).replace("__REGIONS__", regions_html(data)).replace("__STATS__", stats)
            .replace("__DATA__", json.dumps({"systems": data["systems"], "regions": data["regions"]})))
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "galaxy_overview.html").write_text(page, encoding="utf-8")
    # Standalone SVG (fonts fall back to system faces outside the page).
    style = ('<style>.rname{font:700 15px sans-serif;letter-spacing:.32em}.sname{font:600 12px sans-serif;letter-spacing:.12em;fill:#dfe6f3}'
             '.ssub{font:10.5px monospace;fill:#8a9bb8}.scale text{font:11px monospace}</style>')
    (OUT_DIR / "galaxy_map.svg").write_text(map_svg.replace(">", ">" + style, 1), encoding="utf-8")
    print(f"wrote {OUT_DIR / 'galaxy_overview.html'} and galaxy_map.svg: {len(data['systems'])} systems, {n_sec} sectors")


if __name__ == "__main__":
    main()
