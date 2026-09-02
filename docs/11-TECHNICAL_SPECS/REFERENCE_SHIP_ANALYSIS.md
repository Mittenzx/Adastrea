# Reference Spaceship Composition Analysis

Analysis of free (OpenGameArt) reference ship models — distilled design lessons to
inform the Adastrea procedural generator + hand-authored hulls. Models kept LOCAL
in `ref_ships/` for inspection only; licenses vary (CC0, CC-BY, CC-BY-SA) so they
are NOT shipped / committed into game assets.

## Models analyzed (2026-09-02)
| Model | License | Verts/Faces | Kind |
|---|---|---|---|
| civilian-spaceship (model.obj) | CC-BY-SA 4.0 | 456 / 908 | rocket-fuselage multi-engine |
| cargo.zip | **CC0** | 2547 / 2150 | bulk cargo hauler |
| lowpoly spaceship | **CC0** | 148 / 141 | sharp fighter/interceptor |
| rebellion + scifi_ver10 (.blend) | CC-BY / CC-BY-SA | — | spare refs |

## Composition lessons (what makes a ship read as *coherent*, not stacked boxes)

### 1. Unify the form language across all parts
Every analyzed ship that reads well uses the **same geometric vocabulary everywhere**
(consistent sharp facets, consistent angular language, consistent material). Parts are
visually distinct but share a common design grammar — that's what prevents "stack of boxes."
→ *Applies to us*: our kitbash pieces should share angular/planar language with the hull.

### 2. One dominant primary mass + subordinate supports
Coherent ships have a **clear central core/primary volume** that anchors everything,
with wings/pods/engines as *secondary* supporting elements. Size hierarchy matters:
main hull dominates, sub-parts are clearly smaller/scaled to it.
→ *Applies*: our carcass must visibly dominate; add-ons read subordinate in scale.

### 3. Integration beats stacking — flow/taper is the cohesion cue
The low-poly fighter reads coherent because the **central spire rises out of the hull**
and the wings *flow* from the main mass with a clear taper. When parts visibly grow out
of / taper from the primary body (not bolted on with hard orthogonal steps), it reads as
one ship. The cargo hauler reads as modular-but-coherent via **repetition** (repeated
container prisms) + **hierarchical scaling**.
→ *Applies*: favor taper / growth-from-hull over box-on-box; use repetition as a unifier.

### 4. Functional grouping sells the identity
The cargo hauler = bulk/containers (central blocky bays), a **docking/loading gantry**
(upper open frame), landing support (angular base). Each region maps to a function, so
the silhouette advertises its role (hauler vs fighter). Fighters read via *sharpness +
streamline*, haulers via *bulk + repetition*.
→ *Applies*: let role define silhouette — cargo = blocky bulk + container repetition;
fighters = sharp swept taper.

### 5. Directionality & implied motion
Clear forward direction (tapered nose / sweep) tells the eye which way it flies. Ships
that lack a strong fore/aft read as static or confused.
→ *Applies*: keep a strong nose/rear taper like our current X4-style hulls.

### 6. Greeble detail density sets the "realism" bar
The most realistic ships achieve credibility via **surface detail density** (panel seams,
modular segments, engine ports) even at low poly. It's the density + consistency of
detail, not raw poly count, that sells believability.

## Concrete generator takeaways
- Keep the **tapered silhouette** (we already have this) — it's the #1 cohesion cue.
- Ensure a **single dominant carcass** that add-on parts visually scale *from*, not float on.
- Use **repetition** (repeated container/panel forms) to unify modular ships.
- Reinforce **role silhouette**: cargo = bulk + repeat; warship = sharp + sweep.
- Greelble/panel **density + consistency** is what sells realism (supports our Phase-3
  weathering + kitbash direction).

## Honest caveat
These are low-poly / simplified reference ships. Their biggest lesson is *compositional*
(unified form language, integration, hierarchy, functional grouping) — not mesh resolution.
Our procedural kitbash + tapered hull already follows much of this; the gap is intentional
(we generate, they hand-author). Models are reference-only; not committed.