# Art Direction Reference

**Palette:** Endesga 32 (assets/aseprite/endesga32.gpl)
**Version:** 1.0

---

## Tone by area

**Level 1 — swashbuckling**
Warm mids, saturated accents. Torchlight golds, mossy greens. Sprites pop against stone. Bright pickups read instantly.

**Levels 2–3 — dim and perilous**
Desaturated base, deep purples and cool grays. Accent colors hit harder against the muted field. Danger telegraphs own the screen.

---

## Accent signal roles

These four colors are reserved. Do not use them on non-signal geometry.

| Role | Hex | Usage |
|---|---|---|
| Pickup / reward | #feae34 | Coins, health orbs, key items only. Never decorative. |
| Red telegraph | #e43b44 | Enemy attack wind-up and AoE zones. Flashes the frame before the hitbox activates. |
| White telegraph | #ffffff | Unparryable attack indicator. Distinguishes from parryable (red). Same wind-up frame. |
| Parry spark | #2ce8f5 | Cyan flash on successful parry. Only cyan use in combat. 3-4 frames max. |

---

## Shading philosophy

- 2 shades minimum, 3 maximum per hue
- Dither at boundaries between shades, no hard gradients
- Light source is top-left on all sprites without exception
- Highlights land top-left, deepest shadow bottom-right

---

## Silhouette priority

- Every sprite must be legible in silhouette alone
- Draw the outline first, add interior detail second
- Player silhouette is warm-toned (ambers, tan)
- Enemy silhouettes are cool-toned (slate, purple)
- They must never read as the same character at a glance
- No black outlines inside a sprite, the darkest shade of each hue does the work

---

## No-go examples

- Colors outside Endesga 32
- 4 or more shades on a single tile
- Red telegraphs on non-danger sprites
- White highlights anywhere except parry sparks
- Warm player palette on enemy sprites
- Pickup gold used for decoration
- Sub-pixel anti-aliasing
- Gradients, dither instead
