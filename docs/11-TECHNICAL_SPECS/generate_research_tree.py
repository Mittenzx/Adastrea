#!/usr/bin/env python3
"""
Adastrea Research & Breakthrough Tree — authoritative generator.

DERIVES the research/building tree from the canonical
  Content/Data/CraftingTree.json
so there is exactly ONE source of truth for what researches unlock. Each recipe
records a ResearchLevel (1-4) and an optional ResearchRequired item. Research
DATA items (the *Research breakthroughs) are themselves produced inside their
specialized lab, and every recipe whose ResearchRequired points at a breakthrough
is "unlocked" by it.

Emit:
  Content/Data/ResearchTree.json            machine-readable research tree
  docs/11-TECHNICAL_SPECS/RESEARCH_TREE.md  human-readable design doc
  docs/11-TECHNICAL_SPECS/RESEARCH_TREE_DIAGRAM.html/.svg/.png  diagram

Run:
  python docs/11-TECHNICAL_SPECS/generate_research_tree.py
"""
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TREE = os.path.join(ROOT, "Content", "Data", "CraftingTree.json")
OUT_JSON = os.path.join(ROOT, "Content", "Data", "ResearchTree.json")
OUT_MD = os.path.join(ROOT, "docs", "11-TECHNICAL_SPECS", "RESEARCH_TREE.md")
OUT_HTML = os.path.join(ROOT, "docs", "11-TECHNICAL_SPECS", "RESEARCH_TREE_DIAGRAM.html")
OUT_SVG = os.path.join(ROOT, "docs", "11-TECHNICAL_SPECS", "RESEARCH_TREE_DIAGRAM.svg")

# ---- human labels & flavor for the research data items -------------------
R_NAME = {
    "ResearchData": "Research Data",
    "PropulsionResearch": "Propulsion Research",
    "AdvancedPropulsionResearch": "Advanced Propulsion Engines",
    "MaterialsResearch": "Materials Research",
    "NanoMaterialsResearch": "Nano-Materials",
    "ComputingResearch": "Computing Research",
    "QuantumComputingResearch": "Quantum Computing",
    "DefenceResearch": "Defence Research",
    "AdvancedDefenceResearch": "Advanced Defence Systems",
    "BioResearch": "Biotech Research",
    "AdvancedBioResearch": "Advanced Bio-Tech",
    "KineticWeaponResearch": "Kinetic Weapons",
    "BeamWeaponResearch": "Beam Weapons",
}
R_DESC = {
    "ResearchData": "The generic research milestone that unlocks early Mk2 upgrades.",
    "PropulsionResearch": "Physics-lab breakthrough enabling Mk2 engines, reactors and thrusters.",
    "AdvancedPropulsionResearch": "Top-tier propulsion giving Mk3 engines, reactors and fuel systems.",
    "MaterialsResearch": "Materials-lab breakthrough enabling Mk2 hulls, plating and cargo pods.",
    "NanoMaterialsResearch": "Nano-materials giving Mk3 hulls, plating and ship components.",
    "ComputingResearch": "Electronics-lab breakthrough unlocking the broad Mk2 electronics & computer network.",
    "QuantumComputingResearch": "Quantum-class computing unlocking the deepest Mk3 electronics and computers.",
    "DefenceResearch": "Weapons-lab breakthrough enabling Mk2 turrets, cannons, missiles and shields.",
    "AdvancedDefenceResearch": "Advanced weapons research giving Mk3 weapons, shields and point-defence.",
    "BioResearch": "Biology-lab breakthrough enabling Mk2 medicine and vaccines.",
    "AdvancedBioResearch": "Advanced bio-tech giving Mk3 medical supplies and bioweapons.",
    "KineticWeaponResearch": "Projectile-lab specialization enabling railguns, missiles and torpedoes.",
    "BeamWeaponResearch": "Beam-lab specialization enabling lasers, plasma cannons and point-defence.",
}
LAB_TO_DOMAIN = {
    "PhysicsLab": ("Propulsion", "PhysicsLabModule"), "MaterialsLab": ("Materials", "MaterialsLabModule"),
    "ElectronicsLab": ("Computing", "ElectronicsLabModule"), "WeaponsLab": ("Defence", "WeaponsLabModule"),
    "BiologyLab": ("Bio", "BiologyLabModule"), "ScienceLab": ("Base", "ScienceLabModule"),
    "ProjectileWeaponsLab": ("Projectile Weapons", "ProjectileWeaponsLab"),
    "BeamWeaponsLab": ("Beam Weapons", "BeamWeaponsLab"),
}

# Clean display name for a non-research item (used to list unlocks)
def human(item):
    if item in R_NAME:
        return R_NAME[item]
    for mk in ("Mk2", "Mk3"):
        if item.endswith(mk):
            stem = item[: -len(mk)]
            return human(stem).rstrip() + " " + mk
    s = item.replace("_", " ")
    return re.sub(r'(?<=[a-z])(?=[A-Z])', ' ', s)


def main():
    with open(TREE) as f:
        data = json.load(f)
    recipes = data["Recipes"]
    produced = {r["OutputItem"]: r for r in recipes}

    # ---- collect research nodes: every item that appears as a ResearchRequired ----
    req_by_item = {}
    research_ids = set()
    for r in recipes:
        rr = r.get("ResearchRequired")
        if rr and rr != r["OutputItem"]:   # skip self-referential placeholder
            research_ids.add(rr)
            req_by_item.setdefault(rr, []).append(r["OutputItem"])
        # research DATA items themselves are nodes even if nothing requires them
        if "Research" in r["OutputItem"] or r["OutputItem"] == "ResearchData":
            research_ids.add(r["OutputItem"])

    # research DATA items: those named *Research (the breakthroughs) + ResearchData.
    # Excludes things like ResearchComputer that merely contain "Research".
    research_ids = {i for i in research_ids
                    if i.endswith("Research") or i == "ResearchData"}
    # only keep those that are actually produced in the tree
    research_ids = {i for i in research_ids if i in produced}

    nodes = []
    for rid in sorted(research_ids):
        r = produced.get(rid, {})
        unlocks = sorted(set(req_by_item.get(rid, [])))
        unlock_names = [human(u) for u in unlocks]
        nodes.append({
            "ResearchID": rid,
            "Name": human(rid),
            "Description": R_DESC.get(rid, "Research breakthrough."),
            "ResearchLevel": r.get("ResearchLevel", 1),
            "ProducedIn": r.get("ProducedIn", "?"),
            "LabModule": LAB_TO_DOMAIN.get(r.get("ProducedIn"), ("", "?"))[1],
            "UnlocksCount": len(unlocks),
            "Unlocks": unlocks,
            "UnlocksNames": unlock_names,
        })

    # ---- group nodes into research branches by lab (rl2 then rl3) ----
    branches = []
    branch_order = ["PhysicsLab", "MaterialsLab", "ElectronicsLab", "WeaponsLab",
                    "BiologyLab", "ProjectileWeaponsLab", "BeamWeaponsLab"]
    for lab in branch_order:
        dn, module = LAB_TO_DOMAIN[lab]
        lab_nodes = [n for n in nodes if n["ProducedIn"] == lab]
        rl2 = [n for n in lab_nodes if n["ResearchLevel"] == 2]
        rl3 = [n for n in lab_nodes if n["ResearchLevel"] == 3]
        if not lab_nodes:
            continue
        branches.append({
            "Domain": dn,
            "Lab": lab,
            "LabModule": module,
            "ResearchLevel2": rl2[0]["ResearchID"] if rl2 else None,
            "ResearchLevel3": rl3[0]["ResearchID"] if rl3 else None,
            "Unlocks": sum(n["UnlocksCount"] for n in lab_nodes),
        })

    doc = {
        "$schema": "http://json-schema.org/draft-07/schema#",
        "Title": "Adastrea Research & Breakthrough Tree",
        "Description": "Research tree derived from Content/Data/CraftingTree.json. Each research benchmark unlocks the Mk2/Mk3 crafts that reference it. Single source of truth is the crafting tree.",
        "SchemaVersion": "1.0.0",
        "LastUpdated": "2026-08-31",
        "Source": "Content/Data/CraftingTree.json",
        "ResearchLevels": {"Base": 1, "Mk2": 2, "Mk3": 3},
        "Branches": branches,
        "ResearchNodes": nodes,
    }
    os.makedirs(os.path.dirname(OUT_JSON), exist_ok=True)
    with open(OUT_JSON, "w", encoding="utf-8") as f:
        json.dump(doc, f, indent=2)
        f.write("\n")

    write_md(doc)
    write_html(doc)
    print(f"Wrote {OUT_JSON}")
    print(f"Wrote {OUT_MD}")
    print(f"Wrote {OUT_HTML}")
    print(f"Research nodes: {len(nodes)}  Branches: {len(branches)}")


def write_md(doc):
    md = []
    md.append("# Adastrea — Research & Breakthrough Tree\n")
    md.append("*Derived from `Content/Data/CraftingTree.json` — single source of truth.*\n")
    md.append("| Research Level | Meaning |\n|---|---|\n| 1 | Base — no research required |\n| 2 | Unlocks **Mk2** upgrades |\n| 3 | Unlocks **Mk3** upgrades |\n")
    md.append("## Research Branches\n")
    md.append("Different **science/research labs** produce **different breakthroughs**. Each branch: build its lab module → research the rl2 breakthrough (unlocks Mk2) → research the rl3 breakthrough (unlocks Mk3).\n")
    for b in doc["Branches"]:
        md.append(f"### {b['Domain']} — `{b['Lab']}` (`{b['LabModule']}`)\n")
        for n in doc["ResearchNodes"]:
            if n["ProducedIn"] != b["Lab"]:
                continue
            lvl = "Mk2" if n["ResearchLevel"] == 2 else "Mk3" if n["ResearchLevel"] == 3 else f"Lv{n['ResearchLevel']}"
            md.append(f"- **`{n['ResearchID']}`** — *{n['Name']}* ({lvl}) — unlocks {n['UnlocksCount']} crafts: {', '.join(n['UnlocksNames']) or '—'}\n")
        md.append("")
    # cross-lab notes
    md.append("## Cross-Lab Coupling\n")
    md.append("Reverse-engineering the crafting tree shows the strong couplings: several rl3 breakthroughs and *every* computer-family item require `QuantumProcessor_Mk2` (unlocked by ElectronicsLab **ComputingResearch**). So maxing one lab is impossible — late-game tech spans multiple labs.\n")
    md.append("## Research Items (all)\n")
    md.append("| ResearchID | Name | Level | Lab | Unlocks |\n|---|---|---|---|---|\n")
    for n in doc["ResearchNodes"]:
        md.append(f"| `{n['ResearchID']}` | {n['Name']} | {n['ResearchLevel']} | {n['ProducedIn']} | {n['UnlocksCount']} |\n")
    with open(OUT_MD, "w", encoding="utf-8") as f:
        f.write("".join(md))


def write_html(doc):
    J = lambda o: json.dumps(o)
    # Build a per-branch card layout with rl2/rl3 and their unlock lists.
    cards = []
    for b in doc["Branches"]:
        lab = b["Lab"]
        nodes = [n for n in doc["ResearchNodes"] if n["ProducedIn"] == lab]
        rl2 = next((n for n in nodes if n["ResearchLevel"] == 2), None)
        rl3 = next((n for n in nodes if n["ResearchLevel"] == 3), None)
        def unlock_chips(n):
            if not n:
                return ""
            mk = "Mk2" if n["ResearchLevel"] == 2 else "Mk3"
            chips = "".join(f'<span class="u">{u}</span>' for u in n["UnlocksNames"])
            return (f'<div class="bmark {mk}"><div class="btitle">{mk} · {n["Name"]}'
                    f'</div><div class="bdesc">{n["Description"]}</div>'
                    f'<div class="unchips">{chips}</div></div>')
        cards.append(
            f'<div class="branch"><div class="blab">{b["Domain"]} Lab · {lab}</div>'
            f'{unlock_chips(rl2)}{unlock_chips(rl3)}</div>'
        )
    html = f"""<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Adastrea — Research & Breakthrough Tree</title>
<style>
  body {{ background:#020617; color:#cbd5e1; font-family:system-ui,Segoe UI,Roboto; margin:0; padding:24px; }}
  h1 {{ color:#f8fafc; font-size:22px; }}
  .sub {{ color:#64748b; font-size:13px; margin-bottom:20px; }}
  .grid {{ display:grid; grid-template-columns:repeat(auto-fit,minmax(340px,1fr)); gap:16px; }}
  .branch {{ background:#0f172a; border:1px solid #1e293b; border-radius:10px; padding:16px; }}
  .blab {{ font-weight:700; color:#7dd3fc; margin-bottom:12px; font-size:14px; }}
  .bmark {{ border-left:4px solid; border-radius:6px; padding:10px 12px; margin-bottom:12px; background:#111827; }}
  .bmark.Mk2 {{ border-color:#22d3ee; }}
  .bmark.Mk3 {{ border-color:#f472b6; }}
  .btitle {{ font-weight:600; color:#f8fafc; margin-bottom:4px; }}
  .bdesc {{ color:#64748b; font-size:12px; margin-bottom:8px; }}
  .unchips {{ display:flex; flex-wrap:wrap; gap:4px; }}
  .u {{ background:#1e293b; border:1px solid #334155; border-radius:11px; padding:2px 8px;
       font-size:10px; color:#94a3b8; }}
  .note {{ margin-top:18px; background:#131c2a; border:1px solid #1e293b; border-radius:8px;
          padding:12px 16px; font-size:13px; color:#cbd5e1; }}
</style></head>
<body>
<h1>Adastrea — Research &amp; Breakthrough Tree</h1>
<div class="sub">Derived from Content/Data/CraftingTree.json · {len(doc["ResearchNodes"])} research items ·
{len(doc["Branches"])} lab branches</div>
<div class="grid">{''.join(cards)}</div>
<div class="note"><b>Cross-lab coupling:</b> &#8203;several rl3 breakthroughs and every computer need
<b>QuantumProcessor_Mk2</b> (ElectronicsLab ComputingResearch), so late-game tech spans multiple labs.</div>
</body></html>"""
    with open(OUT_HTML, "w", encoding="utf-8") as f:
        f.write(html)
    # Minimal standalone SVG placeholder (real diagram rendered via the HTML above)
    with open(OUT_SVG, "w", encoding="utf-8") as f:
        f.write('<svg xmlns="http://www.w3.org/2000/svg" width="800" height="200">'
                '<rect width="800" height="200" fill="#020617"/>'
                '<text x="40" y="100" fill="#cbd5e1" font-family="monospace" font-size="16">'
                'Research tree — see RESEARCH_TREE_DIAGRAM.html (interactive)</text></svg>')


if __name__ == "__main__":
    main()