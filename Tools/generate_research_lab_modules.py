#!/usr/bin/env python3
"""Generate the 12 research-lab module classes (5 base + 7 niche), Phase 5."""
import os
from pathlib import Path

SRC = Path(__file__).resolve().parent.parent / "Source" / "Adastrea"
PUB = SRC / "Public" / "Stations"
PRIV = SRC / "Private" / "Stations"

# class name, domain, breakthroughs, prerequisite (None = base lab)
LABS = [
    ("PhysicsLabModule",    "Propulsion",      ["PropulsionResearch", "AdvancedPropulsionResearch"], None),
    ("MaterialsLabModule",  "Materials",       ["MaterialsResearch", "NanoMaterialsResearch"], None),
    ("ElectronicsLabModule","Computing",       ["ComputingResearch", "QuantumComputingResearch"], None),
    ("WeaponsLabModule",    "Weapons",         ["DefenceResearch", "AdvancedDefenceResearch"], None),
    ("BiologyLabModule",    "Biotech",         ["BioResearch", "AdvancedBioResearch"], None),
    ("ProjectileWeaponsLab","Kinetic Weapons", ["KineticWeaponResearch"], "WeaponsLabModule"),
    ("BeamWeaponsLab",      "Beam Weapons",    ["BeamWeaponResearch"], "WeaponsLabModule"),
    ("IonPropulsionLab",    "Ion Propulsion",  ["IonPropulsionResearch"], "PhysicsLabModule"),
    ("GravMaterialsLab",    "Grav Materials",  ["GravMaterialsResearch"], "MaterialsLabModule"),
    ("EncryptionLab",       "Encryption",      ["EncryptionResearch"], "ElectronicsLabModule"),
    ("OptronicsLab",        "Optronics",       ["OptronicsResearch"], "ElectronicsLabModule"),
    ("CyberneticsLab",      "Cybernetics",     ["CyberneticsResearch"], "BiologyLabModule"),
]

def hdr(name, domain, breakthroughs, prereq):
    kind = "an upgraded (niche)" if prereq else "a base"
    upgrade_note = f" *\n * Upgraded from {prereq}.\n" if prereq else ""
    return (
        f"""// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#pragma once

#include "CoreMinimal.h"
#include "ResearchLabModule.h"
#include "{name}.generated.h"

/**
 * {domain} research lab module — {kind} domain-specialized research facility.
 *
 * Produces the {domain} domain's breakthrough Data items (see CraftingTree):
 * {', '.join(breakthroughs)}. Craftable as a station module.
"""
        + upgrade_note
        + f""" *
 * Module Group: Processing
 */
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API A{name} : public AResearchLabModule
{{
    GENERATED_BODY()

public:
    A{name}();
}};
"""
    )

def cpp(name, domain, breakthroughs, prereq):
    b = ", ".join(f'"{x}"' for x in breakthroughs)
    prereq_arg = f"A{prereq}::StaticClass()" if prereq else "nullptr"
    prereq_include = f'\n#include "Stations/{prereq}.h"' if prereq else ""
    return f"""// Copyright (c) 2025 Mittenzx. Licensed under MIT.

#include "Stations/{name}.h"{prereq_include}

A{name}::A{name}()
{{
    InitLab(TEXT("{domain}"), {{ {b} }}, {prereq_arg});
    ModulePower = 90.0f;  // research labs draw notable power
    ModuleType = TEXT("{domain} Research Lab");
}}
"""

def main():
    count = 0
    for name, domain, breakthroughs, prereq in LABS:
        (PUB / f"{name}.h").write_text(hdr(name, domain, breakthroughs, prereq), encoding="utf-8")
        (PRIV / f"{name}.cpp").write_text(cpp(name, domain, breakthroughs, prereq), encoding="utf-8")
        count += 1
    print(f"Wrote {count} research-lab module classes ({count} .h + {count} .cpp)")

if __name__ == "__main__":
    main()