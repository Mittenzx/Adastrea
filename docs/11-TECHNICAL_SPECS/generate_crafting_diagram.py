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
    # output item -> recipe
    produced = {}
    for r in recipes:
        produced[r["OutputItem"]] = r
    return recipes, produced


def main():
    recipes, produced = parse()

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
                   5:"STATION CONSTRUCTION PARTS",6:"STATION MODULES (ASSEMBLY)",
                   7:"STATION ASSEMBLY"}
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
        '<path d="M 0 0 L 10 5 L 0 10 z" fill="#64748b"/></marker></defs>')

    # tier columns (vertical divider + label)
    for t in TIER_RANGE:
        cx = col_x[t]
        svg_parts.append(
            f'<text x="{cx+NODE_W+18}" y="{MARGIN_TOP-6}" fill="#475569" font-family="monospace" font-size="13" font-weight="bold" letter-spacing="1">{TIER_LABELS.get(t, f"TIER {t}")}  ▸ T{t}</text>')
        svg_parts.append(
            f'<line x1="{cx-6}" y1="{MARGIN_TOP-28}" x2="{cx-6}" y2="{height-10}" stroke="#1e293b" stroke-width="1" stroke-dasharray="3,4"/>')

    # edges (draw before nodes)
    for (src, dst, x1, y1, x2, y2, qty) in edges:
        # cubic bezier control points
        mx = (x1 + x2) / 2
        svg_parts.append(
            f'<path d="M {x1} {y1} C {mx} {y1}, {mx} {y2}, {x2} {y2}" fill="none" stroke="#475569" stroke-width="1" marker-end="url(#arrow)" opacity="0.7"/>')
    # qty labels ~ midpoint (optional, small)

    # nodes
    for item, (nx, ny) in positions.items():
        r = produced[item]
        ck = cat_key(r)
        stroke = CAT_STROKE.get(ck, DEFAULT_STROKE)
        fill = CAT_FILL.get(ck, DEFAULT_FILL)
        # node
        svg_parts.append(
            f'<g class="node" data-item="{item}" data-tier="{r["Tier"]}" data-cat="{ck}" data-rl="{r.get("ResearchLevel",1)}" data-ing="{json.dumps([i["ItemID"] for i in r["Ingredients"]])}">'
            f'<rect x="{nx}" y="{ny}" width="{NODE_W}" height="{NODE_H}" rx="6" fill="#0f172a"/>'
            f'<rect x="{nx}" y="{ny}" width="{NODE_W}" height="{NODE_H}" rx="6" fill="{fill}" stroke="{stroke}" stroke-width="1.5"/>'
            f'<text x="{nx+8}" y="{ny+17}" fill="#e2e8f0" font-family="monospace" font-size="12" font-weight="bold">{item}</text>'
            f'<text x="{nx+8}" y="{ny+34}" fill="#94a3b8" font-family="monospace" font-size="9">{r["ProducedIn"]} · ×{r["OutputQty"]}</text>'
            f'</g>')

    svg_parts.append('</svg>')
    svg = "".join(svg_parts)

    # ---- html wrapper -----------------------------------------------------
    html = f"""<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Adastrea — Crafting & Building Tree</title>
<style>
  body {{ background:#020617; color:#cbd5e1; font-family:system-ui,Segoe UI,Roboto; margin:0; padding:24px; }}
  h1 {{ color:#f8fafc; font-size:22px; }}
  .sub {{ color:#64748b; font-size:13px; margin-bottom:20px; }}
  .card {{ background:#0f172a; border:1px solid #1e293b; border-radius:10px; padding:18px; margin-bottom:18px; }}
  .legend span.dot {{ display:inline-block; width:12px; height:12px; border-radius:3px; margin-right:6px; vertical-align:middle; }}
  .legend {{ font-size:12px; }}
  .legend div {{ margin:4px 8px; display:inline-block; }}
  svg {{ width:100%; height:auto; display:block; }}
  svg .node {{ cursor:pointer; }}
  svg .node:hover rect {{ stroke-width:3; }}
  #tip {{ position:fixed; display:none; background:#0f172a; border:1px solid #334155; color:#e2e8f0;
         font:12px monospace; padding:10px 12px; border-radius:6px; pointer-events:none; max-width:320px; z-index:99; }}
  #tip b {{ color:#7dd3fc; }}
  @media print {{ body {{ padding:0; background:#fff; }} .card {{ border:none; }} }}
</style></head>
<body>
<div class="card">
  <h1>Adastrea — Crafting &amp; Building Tree</h1>
  <div class="sub">Tiers 1–7 · generated from Content/Data/CraftingTree.json ({len(recipes)} recipes) · acyclic · hover any node for its recipe</div>
  <div class="legend" id="legend"></div>
</div>
<div class="card"> {svg} </div>
<div id="tip"></div>
<script>
const CAT_LABEL = {{
  Mineral:'Raw mineral (ore)', RefinedMineral:'Refined material', Component:'Component',
  Electronics:'Electronics / tech', ConstructionPart:'Construction part', ShipPart:'Ship part / weapon',
  Food:'Food / consumable', Research:'Research / data', Organic:'Organic', Synthetic:'Synthetic', Other:'Other'
}};
document.querySelectorAll('.node').forEach(n=>{{
  n.addEventListener('mousemove',e=>{{
    const tip=document.getElementById('tip');
    const item=n.dataset.item, tier=n.dataset.tier, cat=n.dataset.cat, ing=n.dataset.ing;
    const ings = JSON.parse(ing).length ? JSON.parse(ing).join(', ') : '(raw extraction)';
    tip.style.display='block';
    const mx=e.clientX+14, my=e.clientY+14;
    tip.style.left=mx+'px'; tip.style.top=my+'px';
    tip.innerHTML = `<b>${{item}}</b> · Tier ${{tier}} · ${{CAT_LABEL[cat]||cat}}<br/><span style="color:#94a3b8">${{n.dataset.rl ? 'Research Lv '+n.dataset.rl : ''}}</span><br/>Requires: ${{ings}}`;
  }});
  n.addEventListener('mouseleave',()=>document.getElementById('tip').style.display='none');
}});
// legend
const colors = {CAT_STROKE};
const order = ['Mineral','RefinedMineral','Component','Electronics','ConstructionPart','ShipPart','Food','Research','Organic','Synthetic','Other'];
document.getElementById('legend').innerHTML = order.map(c=>
  `<div><span class="dot" style="background:${{colors[c]||'#94a3b8'}}"></span>${{CAT_LABEL[c]||c}}</div>`).join('');
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