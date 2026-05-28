# Portals System — Locked Spec

**Author**: Robbie (Tech Lead)
**Linear**: [DUN-13](https://linear.app/dungeon-crawler-game/issue/DUN-13)
**Status**: DECISIONS LOCKED. This document is the spec; implementation can begin.

---

## Summary

Portals are simple teleport effects with light screen-distortion VFX. Anchor-based random placement using 5-6 designer-placed slots per room, with a binary horizontal/vertical traversal direction (doorway semantics: enter east → exit west, enter north → exit south). Player and projectiles use portals; **enemies do not**. Atomic snap on center-point overlap. Portal count escalates from 1 pair in the first room with portals to up to 3 pairs randomly in later rooms. Boss-controlled portals are a separate system reserved for the Level 3 boss — used exclusively for boss attacks and the phase-3 map transfer. **Total estimated cost: ~13 days.**

---

## Locked Decisions

### Visual & Audio
- **Simple teleport effect.** No portal-window rendering. No see-through.
- **Visual on traversal**: light screen distortion — "suck in" at entry, "suck out" at exit. Subtle, not theatrical.
- **Audio**: doorway/portal SFX on entry and exit (style reference: *Portal*).
- **Boss vs. modifier portal distinction**: size and color difference. No separate rendering pipeline.

### Who Uses Portals
- **Player**: yes.
- **Projectiles** (player and enemy): yes. Projectile owner reference is preserved across traversal.
- **Enemies**: **no**. Enemies pass through portal trigger zones without effect.
- **Boss-controlled portals (L3 only)**: separate system. The boss uses portals for *attacks* (projectiles, lance, hand-attacks) and the phase-3 map transition. Enemies in the L3 boss room *may* interact with boss-spawned portals; this is reserved design space and will be specified in the L3 boss design ticket (DUN-9).

### Edge Cases
- **Center-point overlap = teleport.** Atomic snap when the center of the entity's collision box enters the portal trigger zone. No half-in state, no partial transit.
- **Projectile enters as portal closes**: projectile is destroyed.
- **Two entities enter portal simultaneously**: queue, process in order.
- **Portal placement inside walls/environment**: prevented by anchor-based placement (designers only place anchors in valid positions).
- **Hit-during-traversal priority**: if a projectile/attack hits an entity that's mid-portal-trigger, the hit resolves first. Portal effect applies only if the entity survives.

### Cooldown
- **0.5 second buffer per entity** after using a portal. Prevents same-frame chaining and re-entry exploits.

### Pair Count & Placement
- **First room introducing portals**: 1 pair (teaching room — minimal complexity).
- **Subsequent portal rooms**: at least 1 pair, randomly up to 3 pairs.
- **Anchor-based random placement**:
  - Designers place 5-6 portal anchor slots per room during level authoring.
  - At room load, the system randomly selects which slots become active and which slots pair with which.
  - Each room has a **binary direction**: horizontal (east/west) or vertical (north/south).
  - Traversal direction is **mirrored doorway semantics**: enter the west portal → exit the east portal moving east. Enter the north portal → exit the south portal moving south.
- **Seedable randomness**: each generation includes a seed value visible in debug mode for reproducibility.

### Boss Room Rule
- **No portals in any boss room** *except* the Level 3 boss room.
- Level 3 boss room uses portals exclusively for **boss attacks** — the player and any enemies cannot traverse them. They are visual/mechanical extensions of the boss, not navigable.
- **Phase 3 transition**: the final boss phase uses portals to transfer the player (and the boss) to a new map. This is a scripted event, not a usable portal pair.

### Validation
- During development, every room with portals must be validated by the **screenshot tool** (described below): the tool runs through all valid anchor-pair combinations and captures each, so the Design Lead can review for broken-looking layouts.

---

## Reference Implementation Sketch

```cpp
// Portal.h
enum class PortalDirection { Horizontal, Vertical };

struct PortalAnchor {
    Vector2 position;
    int     compatibilityGroup; // determines which anchors can pair with which
};

struct PortalPair {
    Vector2          entryPos;
    Vector2          exitPos;
    PortalDirection  direction;
    float            cooldownPerEntity[MAX_ENTITIES];
    bool             active;
};

struct Room {
    std::vector<PortalAnchor> portalAnchors;   // 5-6 hand-placed
    PortalDirection           portalDirection; // horizontal or vertical for this room
    int                       pairCountMin = 1;
    int                       pairCountMax = 3;
};

// At room load
void initializePortals(Room& room, uint32_t seed) {
    int pairCount = randomRange(seed, room.pairCountMin, room.pairCountMax);
    auto pairs = selectValidPairs(room.portalAnchors, pairCount, room.portalDirection, seed);
    for (auto& pair : pairs) {
        spawnPortalPair(pair.entry, pair.exit, room.portalDirection);
    }
}

// Player or projectile overlap
void onEntityCenterOverlap(Entity& e, PortalPair& portal) {
    if (e.type == EntityType::Enemy) return;             // enemies skip portals
    if (portal.cooldownPerEntity[e.id] > 0) return;

    e.position = portal.exitPos;
    e.cooldownPerEntity[e.id] = 0.5f;

    spawnSuckInVFX(portal.entryPos);
    spawnSuckOutVFX(portal.exitPos);
    playPortalAudio();
}

// Projectile traversal preserves owner
void Projectile::onPortalEntry(PortalPair& portal) {
    if (cooldownPerEntity[this->id] > 0) return;
    position = portal.exitPos;
    cooldownPerEntity[this->id] = 0.5f;
    // velocity unchanged — direction stays consistent with doorway semantics
}
```

---

## Implementation Cost Summary

| Component | Days |
| --- | --- |
| Portal pair data structures + anchor system | 0.5 |
| Anchor-based random placement (pair selection + compatibility tagging) | 1.0 |
| Player teleport on center-point overlap (with per-entity cooldown) | 0.5 |
| Projectile teleport handling (owner preservation) | 1.5 |
| Enemy-skip logic (one if-statement, but needs testing across all enemies) | 0.25 |
| VFX (light screen distortion, suck-in/suck-out at entry/exit) | 1.0 |
| Audio cues (open, close, traversal) | 0.5 |
| Level designer tooling (placing anchors in rooms) | 1.0 |
| Screenshot validation tool (auto-captures all valid pair combinations per room) | 1.0 |
| Edge case handling (simultaneous-entry queue, mid-traversal hit, cooldown) | 1.0 |
| Boss-controlled portals (L3 only, dynamic creation for boss attacks) | 2.0 |
| L3 phase-3 map transition (scripted portal event) | 1.0 |
| Tuning + bug fixes during slice | 1.5 |
| **Total** | **~13 days** |

**Calendar time**: ~2.5 weeks for one person focused; ~1.5 weeks for two parallelizing (one on base portal system + anchor placement, one on boss-controlled system + validation tool).

---

## Implementation Order

Recommended sequence so each piece unblocks the next:

1. **Portal struct + anchor system** (0.5 days) — data foundation.
2. **Anchor-based random placement** (1 day) — generation works.
3. **Player teleport on center-point overlap + cooldown** (0.5 days) — first traversal works.
4. **Enemy-skip logic** (0.25 days) — closes the design rule.
5. **VFX + audio** (1.5 days) — traversal *feels* like a portal.
6. **Level designer tooling** (1 day) — designers can now build portal rooms.
7. **Screenshot validation tool** (1 day) — designers can verify all combinations.
8. **Projectile teleport handling** (1.5 days) — full traversal system complete.
9. **Edge case handling** (1 day) — robustness pass.
10. **Boss-controlled portals + phase-3 transition** (3 days) — L3 boss design support.
11. **Tuning** (1.5 days) — final pass during slice playtest.

---

## Scope Cuts (Explicit)

- No portal-window rendering (stretch goal for L3 boss only if meaningfully ahead post-slice).
- No four-orientation portals — binary horizontal/vertical per room, not per portal.
- No velocity rotation through portals — direction follows doorway semantics, not physical rotation.
- No portal-through-portal recursive rendering.
- No dynamic portal placement at runtime (anchors are designer-authored).
- No general portal-using enemy AI.
- No portal-redirects-projectile-trajectory (projectile direction is consistent with doorway).
- No infinite chaining (0.5s cooldown enforced per entity).

---

## Trapdoor Risks — Acknowledged

These are known concerns; locked rules above mitigate them. Documented so they don't resurface as "we should add this."

1. **Portals + Gravity Well**: projectiles are not gravity-affected during the atomic teleport moment (no in-portal state to be pulled).
2. **Portals + Big Guy**: not an issue — enemies don't enter portals.
3. **Exit obstruction**: not an issue — center-point overlap triggers atomic teleport; the exit is a position, not a sustained state.
4. **Player mid-portal mid-frame**: not an issue — atomic snap.
5. **Projectile rapid-fire through portal**: projectile pool sized at 256+ per TECH.md; verify during slice.

---

## Dependencies Unblocked

- **DUN-9** (Level 3 boss — boss-controlled portal behavior is now spec'd; final boss room design can proceed)
- **DUN-16** (encounter composition — no Big Guy exclusion needed since enemies don't enter portals; simpler than originally feared)
- **DUN-17** (interaction matrix — portal cells now have defined behavior: enemies skip, player + projectiles use)
- **DUN-7** (Level 3 room-by-room design — portal mechanics are now stable enough for level designers to build against)
