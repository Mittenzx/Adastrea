#!/usr/bin/env python3
"""
Generate an SVG/HTML crafting-tree diagram from Content/Data/CraftingTree.json.

Lays out the tree left-to-right by tier (1..6), with nodes colored by
EMaterialCategory group, edges drawn ingredient -> output. Produces:
  - docs/11-TECHNICAL_SPECS/CRAFTING_TREE_DIAGRAM.html  (interactive, inline SVG)
  - docs/11-TECHNICAL_SPECS/CRAFTING_TREE_DIAGRAM.svg    (standalone SVG)

Run:  python docs/11-TECHNICAL_SPECS/generate_crafting_diagram.py
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TREE = os.path.join(ROOT, "Content", "Data", "CraftingTree.json")
OUT_DIR = os.path.join(ROOT, "docs", "11-TECHNICAL_SPECS")

# ---- category colors (dark-theme friendly) -------------------------------
CAT_STROKE = {
    "Mineral":        "#fbbf24",   # amber
    "RefinedMineral": "#fb923c",   # orange
    "Component":      "#22d3ee",   # cyan
    "Electronics":    "#34d399",   # emerald
    "ConstructionPart": "#a78bfa", # violet
    "ShipPart":       "#f472b6",   # pink
    "Food":           "#4ade80",   # green
    "Research":       "#818cf8",   # indigo
    "Organic":        "#a3e635",   # lime
    "Synthetic":      "#c084fc",   # purple
    "Other":          "#94a3b8",   # slate
}
CAT_FILL = {
    "Mineral":        "rgba(120,53,15,0.28)",
    "RefinedMineral": "rgba(154,52,18,0.28)",
    "Component":      "rgba(8,51,68,0.40)",
    "Electronics":    "rgba(6,78,59,0.40)",
    "ConstructionPart":"rgba(76,29,149,0.40)",
    "ShipPart":       "rgba(131,24,67,0.40)",
    "Food":           "rgba(20,83,45,0.40)",
    "Research":       "rgba(49,46,129,0.40)",
    "Organic":        "rgba(54,83,20,0.40)",
    "Synthetic":      "rgba(88,28,135,0.40)",
    "Other":          "rgba(30,41,59,0.50)",
}
DEFAULT_STROKE = "#94a3b8"
DEFAULT_FILL   = "rgba(30,41,59,0.50)"


def parse():
    with open(TREE) as f:
        data = json.load(f)
    recipes = data["Recipes"]
    items = data.get("Items", {})
    # output item -> recipe
    produced = {}
    for r in recipes:
        produced[r["OutputItem"]] = r
    return recipes, produced, items


def main():
    recipes, produced, items = parse()

    # Determine material category for each output from ingestion (+ fallback)
    # Use recipe Category (trade) mapped to a display; prefer material category.
    # Build display category: crude map trade category -> EMaterialCategory-ish color group.
    def cat_key(r):
        c = r.get("Category", "Other")
        mapping = {
            "RawMaterials": "Mineral",
            "RefinedGoods": "RefinedMineral",
            "Components": "Component",
            "Technology": "Electronics",
            "Food": "Food",
            "Luxury": "Other",
            "Contraband": "Other",
            "Military": "ShipPart",
            "Medical": "Food",
            "Data": "Research",
            "Other": "Other",
        }
        return mapping.get(c, "Other")

    groups = {}
    for r in recipes:
        t = r["Tier"]
        g = groups.setdefault(t, [])
        g.append(r)

    # ---- tier columns -----------------------------------------------------
    TIER_RANGE = sorted(groups.keys())          # e.g. [1..6]
    TIER_LABELS = {1:"RAW EXTRACTION",2:"REFINED MATERIALS",
                   3:"COMPONENTS & ELECTRONICS",4:"SHIP PARTS / WEAPONS",
                   5:"STATION CONSTRUCTION PARTS",6:"STATION MODULES (ASSEMBLY)"}
    NODE_W = 190
    NODE_H = 44
    COL_GAP_X = 300
    ROW_GAP = 58
    MARGIN_X = 60
    MARGIN_TOP = 120   # room for tier headers
    COL_LABEL_H = 46

    # Resolve y-position of each node. We order within a column by a
    # barycenter pass over input edges (ingredients in earlier columns).
    positions = {}          # ItemID -> (col_x, y)

    # First pass: assign raw/extraction nodes (children of tier 1 column)
    # They are the acquisition recipes themselves (tier 1 outputs are raws).

    # column x
    col_x = {}
    x = MARGIN_X
    for t in TIER_RANGE:
        col_x[t] = x
        x += COL_GAP_X
    width = x - COL_GAP_X + NODE_W + MARGIN_X + 290  # +right reserve for the last tier's header label

    # per-column ordered node lists
    col_order = {t: [] for t in TIER_RANGE}

    # Tier 1 (raws) have no recipe inputs; just order by list.
    # For higher tiers, order by barycenter of ingredient y positions.

    # Iteratively compute y positions
    # We'll assign y per column based on dependencies from left columns.

    # node list per column
    col_items = {t: [r["OutputItem"] for r in groups[t]] for t in TIER_RANGE}

    # y assignment: greedy layered by tier, using barycenter of recipe inputs.
    max_y = 0
    for t in TIER_RANGE:
        col = col_items[t]
        # y index determined by average of dependency positions (already placed)
        def dep_y(id_):
            if id_ in positions:
                return positions[id_][1]
            return None
        scored = []
        for item in col:
            r = produced[item]
            deps = [i["ItemID"] for i in r["Ingredients"]]
            ys = [dep_y(d) for d in deps if dep_y(d) is not None]
            if ys:
                score = sum(ys) / len(ys)
            else:
                score = 1e9  # push no-input (raw) to top
            scored.append((score, item))
        scored.sort(key=lambda s: (s[0], s[1]))
        y = MARGIN_TOP + COL_LABEL_H
        for _, item in scored:
            positions[item] = (col_x[t], y)
            y += ROW_GAP
        max_y = max(max_y, y)

    height = max_y - ROW_GAP + NODE_H + 60

    # ---- per-node research domain (for filters + legend) ----
    DOMAIN_SLUG = {
        "PhysicsLab": "propulsion", "MaterialsLab": "materials",
        "ElectronicsLab": "computing", "WeaponsLab": "defence", "BiologyLab": "bio",
    }
    RESEARCH_TO_DOMAIN = {
        "PropulsionResearch": "propulsion", "AdvancedPropulsionResearch": "propulsion",
        "MaterialsResearch": "materials", "NanoMaterialsResearch": "materials",
        "ComputingResearch": "computing", "QuantumComputingResearch": "computing",
        "DefenceResearch": "defence", "AdvancedDefenceResearch": "defence",
        "BioResearch": "bio", "AdvancedBioResearch": "bio", "ResearchData": "base",
    }
    DOMAIN_LABEL = {
        "propulsion": "Physics Lab", "materials": "Materials Lab",
        "computing": "Electronics Lab", "defence": "Weapons Lab",
        "bio": "Biology Lab", "base": "Base / raw & refined",
    }
    DOMAIN_COLOR = {
        "propulsion": "#f472b6", "materials": "#fb923c", "computing": "#22d3ee",
        "defence": "#fb7185", "bio": "#4ade80", "base": "#94a3b8",
    }
    domain_of = {}
    for item, r in produced.items():
        dom = RESEARCH_TO_DOMAIN.get(item)
        if dom is None:
            for lab, slug in DOMAIN_SLUG.items():
                if item.startswith(lab) and item.endswith("LabModule"):
                    dom = slug; break
        if dom is None:
            req = r.get("ResearchRequired")
            if req in RESEARCH_TO_DOMAIN:
                dom = RESEARCH_TO_DOMAIN[req]
        domain_of[item] = dom or "base"
    # a base item inherits its Mk version's research domain
    for item in list(produced):
        if "_Mk" in item:
            base = item.rsplit("_Mk", 1)[0]
            r = produced[item]
            req = r.get("ResearchRequired")
            if req in RESEARCH_TO_DOMAIN and domain_of.get(base) == "base":
                domain_of[base] = RESEARCH_TO_DOMAIN[req]

    # out-adjacency (item -> consumers) for interactive highlighting
    out_adj = {i: [] for i in produced}
    for r in recipes:
        for ing in r["Ingredients"]:
            if ing["ItemID"] in out_adj:
                out_adj[ing["ItemID"]].append(r["OutputItem"])
    item_list = sorted(produced.keys())
    stats_dict = {k: {"n": v["ItemName"], "d": v["Description"], "w": v["WeightKg"],
                      "m": v["VolumeM3"], "s": v["StorageType"], "r": v["Rarity"],
                      "b": v["BaseValue"]} for k, v in items.items()}

    # ---- arrow marker + svg ----------------------------------------------
    def node_center(item):
        x0, y0 = positions[item]
        return x0, y0, x0 + NODE_W, y0 + NODE_H, x0 + NODE_W / 2, y0 + NODE_H / 2

    edges = []
    for r in recipes:
        out = r["OutputItem"]
        ox, oy = positions[out][0], positions[out][1]
        left = ox
        for ing in r["Ingredients"]:
            iid = ing["ItemID"]
            if iid not in positions:
                continue
            ix, iy = positions[iid]
            right = ix + NODE_W
            edges.append((iid, out, right, iy + NODE_H/2, left, oy + NODE_H/2,
                          ing["Qty"]))

    svg_parts = []
    svg_parts.append(
        f'<svg id="tree" xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">')
    # grid pattern
    svg_parts.append(
        '<defs><pattern id="grid" width="40" height="40" patternUnits="userSpaceOnUse">'
        '<path d="M 40 0 L 0 0 0 40" fill="none" stroke="#1e293b" stroke-width="0.5"/></pattern></defs>')
    svg_parts.append(f'<rect width="{width}" height="{height}" fill="url(#grid)"/>')
    # arrowheads
    svg_parts.append(
        '<defs><marker id="arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">'
        '<path d="M 0 0 L 10 5 L 0 10 z" fill="#94a3b8"/></marker></defs>')

    # tier columns (vertical divider + label)
    for t in TIER_RANGE:
        cx = col_x[t]
        svg_parts.append(
            f'<text x="{cx+NODE_W+18}" y="{MARGIN_TOP-6}" fill="#64748b" font-family="monospace" font-size="13" font-weight="bold" letter-spacing="1">{TIER_LABELS.get(t, f"TIER {t}")}  ▸ T{t}</text>')
        svg_parts.append(
            f'<line x1="{cx-6}" y1="{MARGIN_TOP-28}" x2="{cx-6}" y2="{height-10}" stroke="#1e293b" stroke-width="1" stroke-dasharray="3,4"/>')

    # edges (draw before nodes)
    for (src, dst, x1, y1, x2, y2, qty) in edges:
        mx = (x1 + x2) / 2
        svg_parts.append(
            f'<path class="edge" data-src="{src}" data-dst="{dst}" d="M {x1} {y1} C {mx} {y1}, {mx} {y2}, {x2} {y2}" fill="none" stroke="#475569" stroke-width="1.2" marker-end="url(#arrow)" opacity="0.55"/>')

    # nodes
    for item, (nx, ny) in positions.items():
        r = produced[item]
        ck = cat_key(r)
        stroke = CAT_STROKE.get(ck, DEFAULT_STROKE)
        fill = CAT_FILL.get(ck, DEFAULT_FILL)
        dcol = DOMAIN_COLOR.get(domain_of[item], DEFAULT_STROKE)
        is_mk = "1" if "_Mk" in item else "0"
        ings = [i["ItemID"] for i in r["Ingredients"]]
        svg_parts.append(
            f'<g class="node" data-item="{item}" data-tier="{r["Tier"]}" data-cat="{ck}" data-domain="{domain_of[item]}" data-mk="{is_mk}" data-rl="{r.get("ResearchLevel",1)}" data-ing=\'{json.dumps(ings)}\'>'
            # domain accent strip (left edge)
            f'<rect x="{nx}" y="{ny}" width="5" height="{NODE_H}" rx="2" fill="{dcol}" opacity="0.9"/>'
            f'<rect x="{nx}" y="{ny}" width="{NODE_W}" height="{NODE_H}" rx="6" fill="#0f172a"/>'
            f'<rect x="{nx}" y="{ny}" width="{NODE_W}" height="{NODE_H}" rx="6" fill="{fill}" stroke="{stroke}" stroke-width="1.4"/>'
            f'<text x="{nx+12}" y="{ny+17}" fill="#e2e8f0" font-family="monospace" font-size="12" font-weight="bold">{item}</text>'
            f'<text x="{nx+12}" y="{ny+33}" fill="#94a3b8" font-family="monospace" font-size="9">{r["ProducedIn"]} · ×{r["OutputQty"]}</text>'
            f'</g>')

    svg_parts.append('</svg>')
    svg = "".join(svg_parts)

    # ---- html wrapper -----------------------------------------------------
    J = lambda o: json.dumps(o)
    html = f"""<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Adastrea — Crafting & Building Tree (interactive)</title>
<style>
  :root {{ color-scheme: dark; }}
  * {{ box-sizing: border-box; }}
  body {{ background:#020617; color:#cbd5e1; font-family:system-ui,Segoe UI,Roboto; margin:0; }}
  .controls {{ position:sticky; top:0; z-index:80; background:#020617f2; backdrop-filter:blur(6px);
               border-bottom:1px solid #1e293b; padding:14px 26px; }}
  .controls h1 {{ color:#f8fafc; font-size:20px; margin:0 0 10px; }}
  .controls .sub {{ color:#64748b; font-size:12px; margin-bottom:12px; }}
  .row {{ display:flex; flex-wrap:wrap; align-items:center; gap:8px; margin-bottom:6px; }}
  .lbl {{ font-size:11px; color:#94a3b8; text-transform:uppercase; letter-spacing:.5px; margin-right:4px; }}
  #search {{ background:#0f172a; color:#e2e8f0; border:1px solid #334155; border-radius:6px;
            padding:7px 11px; font:14px system-ui; width:260px; outline:none; }}
  #search:focus {{ border-color:#22d3ee; }}
  .chip {{ background:#0f172a; border:1px solid #334155; color:#cbd5e1; border-radius:20px;
          padding:5px 12px; font:12px system-ui; cursor:pointer; }}
  .chip:hover {{ border-color:#64748b; }}
  .chip.on {{ background:#164e63; border-color:#22d3ee; color:#a5f3fc; }}
  .chip.all {{ font-weight:600; }}
  .box {{ display:flex; align-items:center; gap:6px; font-size:12px; color:#cbd5e1; cursor:pointer; }}
  .box input {{ accent-color:#22d3ee; }}
  .btn {{ background:#0f172a; border:1px solid #334155; color:#e2e8f0; border-radius:6px;
         padding:6px 12px; font:12px system-ui; cursor:pointer; }}
  .btn:hover {{ background:#1e293b; }}
  .hint {{ font-size:11px; color:#475569; margin-top:4px; }}
  main {{ padding:18px 26px 40px; overflow-x:auto; }}
  svg {{ width:100%; height:auto; display:block; min-width:1600px; }}
  svg .node {{ cursor:pointer; transition:opacity .15s; }}
  svg .node.off {{ display:none; }}
  svg .node.hide {{ opacity:0.05; pointer-events:none; }}
  svg .node.hl2 {{ opacity:1; }}
  svg .node:hover rect:nth-of-type(2) {{ stroke-width:3; }}
  svg .edge {{ transition:opacity .15s; }}
  svg .edge.off {{ display:none; }}
  svg .edge.hide {{ opacity:0.03; }}
  #tip {{ position:fixed; display:none; background:#0f172a; border:1px solid #334155; color:#e2e8f0;
         font:12px monospace; padding:10px 12px; border-radius:6px; pointer-events:none; max-width:340px; z-index:99;
         box-shadow:0 8px 30px rgba(0,0,0,.5); }}
  #tip b {{ color:#7dd3fc; }}
  .dlegend {{ font-size:11px; }}
  .dlegend .dot {{ display:inline-block; width:10px; height:10px; border-radius:2px; margin:0 5px 0 0; vertical-align:middle; }}
  .dlegend span {{ margin:2px 10px 2px 0; display:inline-block; }}
  @media print {{ body {{ background:#fff; }} .controls {{ position:static; }} }}
</style></head>
<body>
<div class="controls">
  <h1>Adastrea — Crafting &amp; Building Tree</h1>
  <div class="sub">Tiers 1–6 · {len(recipes)} recipes · generated from Content/Data/CraftingTree.json ·
     search an item, filter by research lab, or collapse Mk upgrades.</div>
  <div class="row">
    <input id="search" type="text" list="items" placeholder="Search an item…" autocomplete="off">
    <datalist id="items">{''.join(f"<option value='{i}'>" for i in item_list)}</datalist>
    <span class="lbl">Lab:</span>
    <button id="dall" class="chip all on">All</button>
    <button class="chip dchip" data-d="propulsion">Physics</button>
    <button class="chip dchip" data-d="materials">Materials</button>
    <button class="chip dchip" data-d="computing">Electronics</button>
    <button class="chip dchip" data-d="defence">Weapons</button>
    <button class="chip dchip" data-d="bio">Biology</button>
    <span class="lbl" style="margin-left:10px;">Show:</span>
    <label class="box"><input type="checkbox" id="mk" checked> Mk 2/3</label>
    <button id="reset" class="btn">Reset</button>
  </div>
  <div class="row">
    <span class="lbl">Tier:</span>
    <label class="box"><input type="checkbox" class="tier" value="1" checked> T1</label>
    <label class="box"><input type="checkbox" class="tier" value="2" checked> T2</label>
    <label class="box"><input type="checkbox" class="tier" value="3" checked> T3</label>
    <label class="box"><input type="checkbox" class="tier" value="4" checked> T4</label>
    <label class="box"><input type="checkbox" class="tier" value="5" checked> T5</label>
    <label class="box"><input type="checkbox" class="tier" value="6" checked> T6</label>
    <span class="dlegend">
      {''.join(f"<span><span class='dot' style='background:{DOMAIN_COLOR[d]}'></span>{DOMAIN_LABEL[d]}</span>" for d in ("propulsion","materials","computing","defence","bio","base"))}
    </span>
  </div>
  <div class="hint">Click a node to pin &amp; trace its full recipe chain · hover to see inputs/outputs · Esc clears.</div>
</div>
<main>{svg}</main>
<div id="tip"></div>
<script>
const CAT_LABEL = {J({
  "Mineral":"Raw mineral (ore)", "RefinedMineral":"Refined material", "Component":"Component",
  "Electronics":"Electronics / tech", "ConstructionPart":"Construction part", "ShipPart":"Ship part / weapon",
  "Food":"Food / consumable", "Research":"Research / data", "Organic":"Organic", "Synthetic":"Synthetic",
  "Other":"Other"})};
const DOMAIN_LABEL = {J(DOMAIN_LABEL)};
const DOMAIN_COLOR = {J(DOMAIN_COLOR)};
const OUT = {J(out_adj)};
const STATS = {J(stats_dict)};

const nodes = [...document.querySelectorAll('.node')];
const edges = [...document.querySelectorAll('.edge')];
const ingMap = {{}};                 // item -> its ingredient items
nodes.forEach(n => {{ ingMap[n.dataset.item] = JSON.parse(n.dataset.ing); }});
const meta = {{}};
nodes.forEach(n => {{ meta[n.dataset.item] = {{tier:n.dataset.tier, mk:n.dataset.mk, cat:n.dataset.cat, domain:n.dataset.domain}}; }});

const search = document.getElementById('search');
let domFilter = null, focus = null, query = '', hover = null;
let showMk = true, tierF = new Set(['1','2','3','4','5','6']);

function near(item) {{ // 1-hop neighbours
  const s = new Set([item]); (ingMap[item]||[]).forEach(i => s.add(i)); (OUT[item]||[]).forEach(o => s.add(o));
  return s;
}}
function ingChain(item) {{ // transitive inputs up to raw
  const s = new Set(), q = [item];
  while (q.length) {{ const c = q.pop(); if (s.has(c)) continue; s.add(c); (ingMap[c]||[]).forEach(i => q.push(i)); }}
  return s;
}}
function activeSet() {{
  if (focus) {{ const s = ingChain(focus); (OUT[focus]||[]).forEach(o => s.add(o)); s.add(focus); return s; }}
  if (query) {{
    const q = query.toLowerCase(), s = new Set();
    Object.keys(meta).filter(i => i.toLowerCase().includes(q)).forEach(m => {{
      ingChain(m).forEach(x => s.add(x)); (OUT[m]||[]).forEach(x => s.add(x)); s.add(m);
    }});
    return s;
  }}
  if (domFilter) {{ const s = new Set(); Object.keys(meta).forEach(i => {{ if (meta[i].domain === domFilter) s.add(i); }}); return s; }}
  return null; // everything active
}}
function offNode(n) {{
  return ( !showMk && n.dataset.mk === '1' ) || ( tierF.size && !tierF.has(n.dataset.tier) );
}}
function apply() {{
  const active = activeSet();                 // emphasis set (null = all)
  const em = hover ? near(hover) : active;
  nodes.forEach(n => {{
    const off = offNode(n);
    const on = em === null || em.has(n.dataset.item);
    n.classList.toggle('off', off);
    n.classList.toggle('hide', !off && !on);
  }});
  edges.forEach(e => {{
    const a = meta[e.dataset.src], b = meta[e.dataset.dst];
    const off = !a || !b || offNode({{dataset:{{mk:a.mk, tier:a.tier}}}}) || offNode({{dataset:{{mk:b.mk, tier:b.tier}}}});
    const on = em === null || (em.has(e.dataset.src) && em.has(e.dataset.dst));
    e.classList.toggle('off', off);
    e.classList.toggle('hide', !off && !on);
  }});
}}
function refresh() {{ apply(); }}

// ---- controls ----
document.querySelectorAll('.dchip').forEach(b => b.addEventListener('click', () => {{
  document.querySelectorAll('.dchip').forEach(x => x.classList.remove('on'));
  document.querySelector('#dall').classList.remove('on');
  if (domFilter === b.dataset.d) {{ domFilter = null; document.querySelector('#dall').classList.add('on'); }}
  else {{ domFilter = b.dataset.d; b.classList.add('on'); }}
  refresh();
}}));
document.getElementById('dall').addEventListener('click', () => {{
  domFilter = null; focus = null; query = ''; search.value = '';
  document.querySelectorAll('.dchip').forEach(x => x.classList.remove('on'));
  document.getElementById('dall').classList.add('on'); refresh();
}});
document.querySelectorAll('.tier').forEach(c => c.addEventListener('change', () => {{
  tierF = new Set([...document.querySelectorAll('.tier')].filter(x => x.checked).map(x => x.value));
  refresh();
}}));
document.getElementById('mk').addEventListener('change', e => {{ showMk = e.target.checked; refresh(); }});
search.addEventListener('input', () => {{ query = search.value.trim(); focus = null; refresh(); }});
search.addEventListener('keydown', e => {{ if (e.key === 'Escape') {{ search.value=''; query=''; refresh(); }} }});
document.getElementById('reset').addEventListener('click', () => {{
  domFilter=null; focus=null; query=''; hover=null; search.value=''; showMk=true;
  document.querySelector('#mk').checked = true;
  document.querySelectorAll('.tier').forEach(c => c.checked = true); tierF = new Set(['1','2','3','4','5','6']);
  document.querySelectorAll('.dchip').forEach(x => x.classList.remove('on'));
  document.getElementById('dall').classList.add('on');
  refresh();
}});

// ---- node interactions ----
nodes.forEach(n => n.addEventListener('mouseenter', () => {{ hover = n.dataset.item; refresh(); }}));
nodes.forEach(n => n.addEventListener('mouseleave', () => {{ hover = null; refresh(); }}));
nodes.forEach(n => n.addEventListener('click', () => {{
  if (focus === n.dataset.item) {{ focus = null; document.getElementById('tip').style.display='none'; }}
  else {{ focus = n.dataset.item; const t = document.getElementById('tip');
    t.style.display='block'; t.style.left='50%'; t.style.top='120px';
    t.innerHTML = `<b>${{n.dataset.item}}</b> pinned — full recipe chain highlighted. Press Esc or click again to unpin.`; }}
  refresh();
}}));
window.addEventListener('keydown', e => {{ if (e.key === 'Escape') {{ focus=null; document.getElementById('tip').style.display='none'; refresh(); }} }});

// ---- tooltip ----
nodes.forEach(n => n.addEventListener('mousemove', e => {{
  const t = document.getElementById('tip');
  const itm = n.dataset.item;
  const ing = JSON.parse(n.dataset.ing);
  const ings = ing.length ? ing.join(', ') : '(raw extraction)';
  const dom = DOMAIN_LABEL[n.dataset.domain] || n.dataset.domain;
  const st = STATS[itm] || {{}};
  const star = {{Common:'★', Uncommon:'★★', Rare:'★★★', VeryRare:'★★★★', Legendary:'★★★★★'}}[st.r] || '';
  t.style.display = (focus === itm) ? 'none' : 'block';
  t.style.left = (e.clientX + 14) + 'px'; t.style.top = (e.clientY + 14) + 'px';
  t.innerHTML = `<b>${{st.n || itm}}</b> <span style="color:#fbbf24">${{star}}</span> · <span style="color:#94a3b8">${{st.r || ''}}</span> · Tier ${{n.dataset.tier}} · ${{CAT_LABEL[n.dataset.cat]||n.dataset.cat}}<br/>
    <span style="color:#64748b">${{st.d || ''}}</span><br/>
    <span style="color:${{DOMAIN_COLOR[n.dataset.domain]}}">■</span> <span style="color:#94a3b8">${{dom}}</span>
    <span style="color:#94a3b8">${{n.dataset.rl>1 ? '· Research Lv '+n.dataset.rl : ''}}</span><br/>
    <span style="color:#7dd3fc">${{st.w ? st.w+' kg' : ''}}</span> · <span style="color:#7dd3fc">${{st.m ? st.m+' m³' : ''}}</span> ·
    <span style="color:#94a3b8">${{st.s || ''}}</span> · <span style="color:#4ade80">${{st.b ? st.b+' cr' : ''}}</span><br/>
    Requires: ${{ings}}`;
}}));
nodes.forEach(n => n.addEventListener('mouseleave', () => {{ if (focus !== n.dataset.item) document.getElementById('tip').style.display='none'; }}));

refresh();
</script>
</body></html>"""

    os.makedirs(OUT_DIR, exist_ok=True)
    html_path = os.path.join(OUT_DIR, "CRAFTING_TREE_DIAGRAM.html")
    svg_path = os.path.join(OUT_DIR, "CRAFTING_TREE_DIAGRAM.svg")
    with open(html_path, "w", encoding="utf-8") as f:
        f.write(html)
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg)
    print(f"Wrote {html_path}")
    print(f"Wrote {svg_path}")
    print(f"Nodes: {len(positions)}  Edges: {len(edges)}  Canvas: {width}x{height}")


if __name__ == "__main__":
    main()