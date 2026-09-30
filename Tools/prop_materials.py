"""Prop material palette shared by Blender previews (Tools/build_interior_props.py)
and the Unreal import (Tools/import_interior_props.py). No Blender/UE imports.

slot -> (base colour (linear RGB), roughness, metallic)
Every slot becomes MI_Prop_<Name> (an instance of M_PropSurface: base colour
times a tiled micro-detail/grime layer on the mesh UVs, so it works at any
mesh scale).
"""
PROP_MATS = {
    "M_Prop_RedPaint":    ((0.55, 0.04, 0.03), 0.45, 0.10),
    "M_Prop_White":       ((0.78, 0.79, 0.80), 0.50, 0.00),
    "M_Prop_BlackPlastic": ((0.025, 0.026, 0.03), 0.40, 0.00),
    "M_Prop_Steel":       ((0.42, 0.44, 0.47), 0.40, 0.85),
    "M_Prop_Brushed":     ((0.70, 0.71, 0.73), 0.28, 1.00),
    "M_Prop_Rubber":      ((0.02, 0.02, 0.02), 0.85, 0.00),
    "M_Prop_Yellow":      ((0.75, 0.52, 0.02), 0.50, 0.05),
    "M_Prop_Olive":       ((0.20, 0.24, 0.11), 0.65, 0.05),
    "M_Prop_Blue":        ((0.04, 0.12, 0.42), 0.50, 0.05),
    "M_Prop_Orange":      ((0.80, 0.25, 0.02), 0.50, 0.05),
    "M_Prop_Paper":       ((0.80, 0.78, 0.70), 0.90, 0.00),
    "M_Prop_Cardboard":   ((0.45, 0.30, 0.15), 0.90, 0.00),
    "M_Prop_Ceramic":     ((0.72, 0.70, 0.66), 0.25, 0.00),
    "M_Prop_Fabric":      ((0.16, 0.18, 0.22), 0.95, 0.00),
    "M_Prop_Copper":      ((0.62, 0.30, 0.16), 0.35, 1.00),
    "M_Prop_Screen":      ((0.01, 0.012, 0.015), 0.08, 0.00),
    "M_Prop_Green":       ((0.06, 0.25, 0.05), 0.70, 0.00),
    "M_Prop_Wood":        ((0.36, 0.20, 0.09), 0.60, 0.00),
    "M_Prop_Hull":        ((0.55, 0.57, 0.60), 0.45, 0.35),     # exterior hull plate
}

# Exterior light lenses (nav lights, strobes, beacons, floods, signs): a flat
# emissive whose Color/Intensity UExteriorDressingComponent drives per light.
#   M_Prop_Beacon -> MI_Prop_Beacon (instance of M_PropBeacon)
BEACON_MATS = {
    "M_Prop_Beacon": ((1.0, 0.95, 0.85), 20.0),
}

# Render-target screens: 0..1 UVs, parameters set at runtime by the C++.
#   M_Prop_Live -> MI_Prop_Live   (exterior camera feed)
#   M_Prop_Term -> MI_Prop_Term   (console text canvas)
SCREEN_MATS = {
    "M_Prop_Live": ((0.35, 0.8, 1.0), 12.0),    # idle tint, emissive strength (readable in lit rooms)
    "M_Prop_Term": ((0.25, 1.0, 0.45), 25.0),
}
