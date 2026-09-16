"""Generate or validate space-station placements within a sector, enforcing a
minimum distance between every pair of stations.

Motivation: TestLevel had 6 hand-placed BP_SpaceStation actors where 3 of them
sat within a tight 60-degree wedge only ~26,000 units apart while the rest of
the ring was 70,000-100,000 units apart - visually "clustered" even though
nothing was actually overlapping. This exists so future sector layouts (hand
or procedural) can just call generate_station_positions() instead of picking
coordinates by eye, and so an existing layout can be checked before it ships.

No bpy/unreal dependency - pure Python, so it can be imported by other
generator scripts or run standalone to print/validate coordinates that then
get applied via the usual unreal-mcp actor placement calls.

Usage:
    python Tools/generate_sector_layout.py generate --count 6 --min-distance 50000
    python Tools/generate_sector_layout.py validate --min-distance 50000 \\
        --positions "50000,0;0,0;15450.85,47552.83"
"""
from __future__ import annotations

import argparse
import math
import random
import sys
from dataclasses import dataclass
from typing import Sequence

# Matches ASpaceSectorMap::SectorSize (200km sector -> 100,000,000 unit half-size... but
# station clusters in practice sit far tighter than a full sector; this default radius
# matches the scale already used by hand-placed content in TestLevel, not the sector
# half-size itself.
DEFAULT_MIN_DISTANCE = 50000.0  # 500m between stations, matches the tightest *intended*
                                 # spacing already in TestLevel (hub-to-ring distance)
DEFAULT_RADIUS = 50000.0


@dataclass(frozen=True)
class Vec2:
    x: float
    y: float

    def distance_to(self, other: "Vec2") -> float:
        return math.hypot(self.x - other.x, self.y - other.y)


def generate_ring_positions(
    count: int,
    radius: float = DEFAULT_RADIUS,
    center: Vec2 = Vec2(0.0, 0.0),
    include_center: bool = True,
    start_angle_deg: float = 0.0,
) -> list[Vec2]:
    """Evenly distribute `count` stations around `center` at `radius`, optionally
    with one extra station AT the center (a hub). This is the right choice when
    you want guaranteed, predictable spacing (every neighbour pair is exactly
    2*radius*sin(pi/count) apart) rather than a randomized scatter.
    """
    if count < 1:
        raise ValueError("count must be >= 1")

    positions: list[Vec2] = []
    if include_center:
        positions.append(center)
        ring_count = count - 1
    else:
        ring_count = count

    if ring_count > 0:
        angle_step = 360.0 / ring_count
        for i in range(ring_count):
            angle = math.radians(start_angle_deg + i * angle_step)
            positions.append(Vec2(
                center.x + radius * math.cos(angle),
                center.y + radius * math.sin(angle),
            ))
    return positions


def generate_scattered_positions(
    count: int,
    min_distance: float = DEFAULT_MIN_DISTANCE,
    bounds_radius: float = DEFAULT_RADIUS * 3,
    center: Vec2 = Vec2(0.0, 0.0),
    max_attempts_per_point: int = 500,
    seed: int | None = None,
) -> list[Vec2]:
    """Poisson-disc-style rejection sampling: place stations at random points
    within a circular area, retrying any candidate that's too close to an
    already-placed station. Use this instead of generate_ring_positions() when
    you want an organic, non-uniform scatter (e.g. a naturally-occurring
    asteroid-field-adjacent cluster) that still guarantees no two stations end
    up closer than min_distance.

    Raises RuntimeError if a point can't be placed after max_attempts_per_point
    tries - that means bounds_radius is too small for `count` stations at this
    min_distance (increase bounds_radius or reduce count/min_distance).
    """
    rng = random.Random(seed)
    placed: list[Vec2] = []
    for _ in range(count):
        for _attempt in range(max_attempts_per_point):
            angle = rng.uniform(0, 2 * math.pi)
            r = bounds_radius * math.sqrt(rng.uniform(0, 1))
            candidate = Vec2(center.x + r * math.cos(angle), center.y + r * math.sin(angle))
            if all(candidate.distance_to(p) >= min_distance for p in placed):
                placed.append(candidate)
                break
        else:
            raise RuntimeError(
                f"Could not place station {len(placed) + 1}/{count} within "
                f"{max_attempts_per_point} attempts - bounds_radius={bounds_radius} is "
                f"too small for {count} stations at min_distance={min_distance}. "
                f"Increase bounds_radius or reduce count/min_distance."
            )
    return placed


def validate_min_distance(positions: Sequence[Vec2], min_distance: float) -> list[tuple[int, int, float]]:
    """Return a list of (index_a, index_b, actual_distance) for every pair that
    violates min_distance. An empty list means the layout is valid.
    """
    violations = []
    for i in range(len(positions)):
        for j in range(i + 1, len(positions)):
            d = positions[i].distance_to(positions[j])
            if d < min_distance:
                violations.append((i, j, d))
    return violations


def _parse_position(s: str) -> Vec2:
    x_str, y_str = s.split(",")
    return Vec2(float(x_str), float(y_str))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="mode", required=True)

    gen = sub.add_parser("generate", help="Print evenly-spaced ring positions")
    gen.add_argument("--count", type=int, required=True)
    gen.add_argument("--radius", type=float, default=DEFAULT_RADIUS)
    gen.add_argument("--no-center", action="store_true", help="Don't include a hub station at the center")
    gen.add_argument("--start-angle", type=float, default=0.0)

    scatter = sub.add_parser("scatter", help="Print randomized positions with a guaranteed minimum distance")
    scatter.add_argument("--count", type=int, required=True)
    scatter.add_argument("--min-distance", type=float, default=DEFAULT_MIN_DISTANCE)
    scatter.add_argument("--bounds-radius", type=float, default=DEFAULT_RADIUS * 3)
    scatter.add_argument("--seed", type=int, default=None)

    validate = sub.add_parser("validate", help="Check an existing layout for min-distance violations")
    validate.add_argument("--min-distance", type=float, default=DEFAULT_MIN_DISTANCE)
    validate.add_argument(
        "--positions", required=True,
        help='semicolon-separated "x,y" pairs, e.g. "50000,0;0,0;-40450.85,29389.26" '
             '(semicolon-separated, not space-separated, so negative coordinates '
             "don't get misread as extra flags by argparse)",
    )

    args = parser.parse_args()

    if args.mode == "generate":
        positions = generate_ring_positions(
            args.count, radius=args.radius,
            include_center=not args.no_center,
            start_angle_deg=args.start_angle,
        )
    elif args.mode == "scatter":
        positions = generate_scattered_positions(
            args.count, min_distance=args.min_distance,
            bounds_radius=args.bounds_radius, seed=args.seed,
        )
    else:  # validate
        positions = [_parse_position(p) for p in args.positions.split(";")]
        violations = validate_min_distance(positions, args.min_distance)
        if violations:
            print(f"FAIL: {len(violations)} pair(s) violate min_distance={args.min_distance}:")
            for i, j, d in violations:
                print(f"  [{i}] <-> [{j}]: {d:.1f} units apart (need >= {args.min_distance})")
            return 1
        print(f"OK: all {len(positions)} positions are >= {args.min_distance} units apart")
        return 0

    for i, p in enumerate(positions):
        print(f"[{i}] x={p.x:.2f}, y={p.y:.2f}")
    violations = validate_min_distance(positions, DEFAULT_MIN_DISTANCE)
    if violations:
        print(f"WARNING: {len(violations)} pair(s) below the default min_distance={DEFAULT_MIN_DISTANCE}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
