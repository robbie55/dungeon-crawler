# Enemy × Modifier Interaction Matrix — Locked Spec

**Linear**: [DUN-17](https://linear.app/dungeon-crawler-game/issue/DUN-17)
**Status**: LOCKED. Each cell has a verdict and remediation plan. Verdicts can shift in playtest; the matrix is the contract for which combinations are safe to ship.

---

## Summary

5 enemies × 9 modifiers = 45 cells. For each cell, the verdict is one of:

- ✅ **Compatible** — enemy works as designed in this modifier room.
- 🔻 **Weaker** — enemy is easier than normal here; design encounters with more of them.
- 🔺 **Stronger** — enemy is harder than normal here; design with fewer of them.
- ⚠️ **Restricted** — flagged combination. Either fixed via AI behavior, or forbidden via encounter composition rules.
- ➖ **N/A** — enemy doesn't appear in this modifier room (boss-only).

**Design rule**: every enemy should *function* in every modifier room they're allowed to appear in. ⚠️ cells exist because either the modifier breaks the enemy's design intent, or the combination produces unfair gameplay.

---

## The Matrix

|  | Grunt | Big Guy + Shooter | Disruptor (Flying) | Shaman | Parrot (boss) |
| --- | --- | --- | --- | --- | --- |
| **L1: Quicksand/Water** | ✅ | 🔻 ⚠️ | ✅ (flies over) | 🔻 | ➖ |
| **L1: Wind Gusts** | ✅ | ✅ | 🔺 ⚠️ | ✅ | 🔺 ⚠️ |
| **L1: Destructible Cover** | ✅ | 🔺 | 🔻 ⚠️ | 🔺 | ➖ |
| **L2: Dim Light** | ✅ | ✅ | 🔺 | 🔺 ⚠️ | ➖ |
| **L2: Falling Roof** | ✅ | 🔻 | 🔺 ⚠️ | ✅ | ➖ |
| **L2: Dart Traps / Plates** | 🔺 | 🔻 | ✅ (flies over) | 🔻 | ➖ |
| **L3: Portals** | 🔺 ⚠️ | ⚠️ FORBIDDEN | 🔺 | 🔻 ⚠️ | ➖ |
| **L3: Blood Altars** | 🔺 | 🔺 | 🔺 | 🔺 ⚠️ | ➖ |
| **L3: Gravity Well** | 🔺 | 🔻 | 🔺 ⚠️ | ⚠️ FORBIDDEN | ➖ |

---

## Cell-by-Cell Resolution

### Level 1 — Surface

#### L1 Quicksand/Water × Grunt — ✅
Slowed in pools. Grunts in water are easy targets but still threatening through numbers. Player can herd Grunts into water as a tactical play. Works as designed.

#### L1 Quicksand/Water × Big Guy Pair — 🔻 ⚠️
**Issue**: Big Guy is already Slow; in water he becomes nearly stationary. Player trivially kites the pair, never closes, fight stalls.
**Resolution**: Big Guy AI treats water tiles as *uncrossable* (pathfind around them rather than through them). Preserves his "interpose" role; he can't get trapped in mud. Implementation: 0.5 day of AI adjustment in DUN-27. **Restrict** Big Guy Pair spawns to rooms where pathing around water is geometrically possible — level designers must verify.

#### L1 Quicksand/Water × Disruptor — ✅
Flies over water. Unaffected. Works.

#### L1 Quicksand/Water × Shaman — 🔻
Shaman is Slow; if spawned in water, never moves. Acceptable — shaman is meant to stay put. But level designers should avoid spawning the shaman *inside* a water tile (telegraph problem — player sees it but the visual is muddled).

#### L1 Wind Gusts × Grunt — ✅
Wind is a *projectile* modifier in its base form (no player-pull outside the Silas variant). Grunts are melee, unaffected. Works.

#### L1 Wind Gusts × Big Guy Pair — ✅
Big Guy melee unaffected. Shooter's projectiles get the wind treatment — curving / accelerating bullets that the player must read. Adds challenge but doesn't break the enemy.

#### L1 Wind Gusts × Disruptor — 🔺 ⚠️
**Issue**: Disruptor lobs bottles in an arc. Wind affects arc trajectories. Result: bottles land in unpredictable spots — could be cool, could feel unfair if puddles land *on the disruptor itself* (unfair to enemies) or *under the disruptor's flight path* (where the disruptor would normally fly through its own puddle).
**Resolution**: bottles ignore wind. Wind only affects *enemy shooter projectiles* (direct fire), not lobbed AoE projectiles. Implementation: in projectile system, wind modifier only applies to `Projectile.is_lobbed == false`. Single-line filter. 0.25 day in DUN-24's modifier integration.

#### L1 Wind Gusts × Shaman — ✅
Shaman doesn't move, doesn't fire projectiles. Wind has no interaction. Works.

#### L1 Wind Gusts × Parrot (boss only) — 🔺 ⚠️
**Issue**: Parrot has a homing trajectory toward the player. If wind modifies its velocity, it could miss the player entirely or be impossible to hit. Boss-fight-specific concern: Silas's Phase 1 has wind already pushing his bullets — adding wind to the parrot creates too many wind-driven things to track.
**Resolution**: parrot homing **ignores wind**. Parrot is a player-tracker, not an air-projectile. Implementation: parrot AI uses direct world-space targeting, doesn't read the wind multiplier. 0 days of additional work — just don't apply wind to parrot in DUN-29.

#### L1 Destructible Cover × Grunt — ✅
Grunts can break cover (or path around it). Adds tactical layer for the player. Works.

#### L1 Destructible Cover × Big Guy Pair — 🔺
Big Guy's slow approach is harder to avoid when cover is depleting. Shooter benefits from intact cover positions. Increases pair's effectiveness — design encounters with fewer of them (1 pair max in cover rooms).

#### L1 Destructible Cover × Disruptor — 🔻 ⚠️
**Issue**: bottles destroy cover on impact. Disruptor inadvertently demolishes the level designer's careful cover placement, neutralizing the modifier within seconds.
**Resolution**: bottles **don't damage destructible cover** — they pass through (or land harmlessly on top). Destructible cover only depletes from *player and Big-Guy-Shooter* projectiles. Implementation: collision filter in projectile system. 0.25 day.

#### L1 Destructible Cover × Shaman — 🔺
Shaman positioned behind cover is hard to reach. The buff completes before the player can break through. Forces the player to commit hard to closing.
**Encounter rule**: shaman spawn positions must have at least one clear line of approach (no full-cover walls between player entry and shaman). Level design constraint, not a code fix.

### Level 2 — Catacombs

#### L2 Dim Light × Grunt — ✅
Grunts emerge from darkness — atmospheric and threatening. Works.

#### L2 Dim Light × Big Guy Pair — ✅
The slow, hulking silhouette of the Big Guy in dim light is on-theme. Shooter's projectiles announce his position via muzzle flash. Works.

#### L2 Dim Light × Disruptor — 🔺
Flying enemies in dim light are harder to track (vertical movement + dim visuals). Increased difficulty, design with fewer (max 1 per dim-light room).

#### L2 Dim Light × Shaman — 🔺 ⚠️
**Issue**: shaman casts a buff ritual with a *visual aura* telegraph. In dim light, if the shaman is outside the player's torch radius, the player can't see the cast starting — they hear the audio but can't locate the source. Critical telegraph problem.
**Resolution**: shaman cast aura **emits its own light** (light-source on the enemy). Player can see the cast across the room even outside their torch radius. Implementation: shaman entity carries a light component during cast state. 0.5 day in DUN-27's Shaman behavior.

#### L2 Falling Roof × Grunt — ✅
Falling debris zones add chaos. Grunts may be killed by debris (player can use this tactically). Works.

#### L2 Falling Roof × Big Guy Pair — 🔻
Big Guy moves slowly through telegraph zones — player can bait him under falling debris. Becomes a "kill the boss with the modifier" puzzle. Acceptable in moderation; some rooms can do this, others should have geometry that prevents it.

#### L2 Falling Roof × Disruptor — 🔺 ⚠️
**Issue**: flying disruptor + falling debris zones = visual chaos. Telegraph indicators overlap with flying enemies, both moving. Player can't track everything.
**Resolution**: disruptors **don't co-spawn with active falling-roof zones**. Specifically — when a falling-roof zone is telegraphed (dust + rumble), no new disruptor projectiles can be fired during that window. Implementation: 0.25 day — modifier state pings disruptor AI to delay its next bottle. Alternatively: limit to 1 disruptor in falling-roof rooms instead of the normal max 2.

#### L2 Dart Traps / Plates × Grunt — 🔺
Grunts ignore plates (no AI for "step on plate") — they trip them randomly. Tactically useful for the player but unpredictable. Acceptable.
*Open design choice*: do Grunts intentionally avoid plates? If so, +1 day of AI work. Default: **no**, they trip them dumbly. Grunts are dumb enemies.

#### L2 Dart Traps / Plates × Big Guy Pair — 🔻
Big Guy's slow path makes him highly likely to step on plates. Player can predict his path and bait him onto traps. Becomes an exploitable interaction — acceptable but design rooms so plates aren't the *only* way to deal with Big Guy.

#### L2 Dart Traps / Plates × Disruptor — ✅
Flies over plates. Unaffected. Works.

#### L2 Dart Traps / Plates × Shaman — 🔻
If shaman spawns on a plate, it dies before completing the cast. Trivial counter.
**Encounter rule**: shaman spawn positions must avoid plate tiles. Level design constraint.

### Level 3 — Temple

#### L3 Portals × Grunt — 🔺 ⚠️
**Issue**: per DUN-13, enemies do NOT enter portals. But Grunts in a portal room can be *flanked through portals by the player*, which is fine and intended. However, encounter design needs to account for the player having a portal-flank option — placing Grunts in tight corridor positions that the portal renders trivial.
**Resolution**: no code fix needed. **Encounter design constraint**: portal-room Grunt placements must remain threatening even with the portal in use. Level designers verify this by playing each room with and without portal use.

#### L3 Portals × Big Guy Pair — ⚠️ FORBIDDEN
**Issue**: per DUN-16, Big Guy's interpose AI breaks if the player can portal-flank (he doesn't update positioning for portal-aware threats). Fixing this requires significant AI work (portal-aware pathfinding).
**Resolution**: **Big Guy pairs do not spawn in portal rooms.** Hard restriction in encounter composition. Already noted in DUN-16; reaffirmed here.

#### L3 Portals × Disruptor — 🔺
Disruptor flies; player can portal under/around it. Adds tactical layer. Works.

#### L3 Portals × Shaman — 🔻 ⚠️
**Issue**: portal-room Shamans become trivial — player portals next to the shaman and kills in seconds, completely circumventing the priority-target challenge.
**Resolution**: **Shaman spawn positions in portal rooms must require crossing the room *without* the portal route.** This means spawning shamans either far from any portal exit, or behind cover that portals can't bypass. Level design constraint, not a code fix. If level design can't satisfy this — restrict shaman to non-portal rooms.

#### L3 Blood Altars × Grunt — 🔺
Grunts within altar range heal — slows player's swarm-clearing rhythm. Forces the player to break the altar before clearing grunts. Works as designed (altar IS the priority target).

#### L3 Blood Altars × Big Guy Pair — 🔺
Healed Big Guy means longer fight, more shooter shots taken. High pressure. Reduce encounter density (1 pair max in altar rooms).

#### L3 Blood Altars × Disruptor — 🔺
Healed Disruptor is harder to kill before its puddles cover the floor. High pressure. Max 1 disruptor in altar rooms.

#### L3 Blood Altars × Shaman — 🔺 ⚠️
**Issue**: shaman in altar range heals during cast. Player must kill the shaman before the cast completes, *and* destroy the altar to even land hits. Stack of priority targets — may be too punishing as designed.
**Resolution**: **shaman is immune to altar healing during cast.** The cast already requires uninterrupted standing-still; rationalize as "the ritual prevents healing." Implementation: shaman ignores `health_aura` effects while in `Casting` state. 0.25 day.

#### L3 Gravity Well × Grunt — 🔺
Grunts get pulled toward the well center. Becomes a focal point of the encounter — player can use the well to cluster grunts for cutlass arcs. Works tactically.

#### L3 Gravity Well × Big Guy Pair — 🔻
Big Guy gets pulled toward the well, breaking his interpose positioning. The pair's puzzle (interpose between player and shooter) degrades — Big Guy ends up wherever the well puts him.
**Resolution**: Big Guy *resists* gravity well at a higher threshold than other entities (2x his weight constant, or however the pull is implemented). Preserves his interpose intent without ignoring the modifier entirely. 0.25 day in AI tuning.

#### L3 Gravity Well × Disruptor — 🔺 ⚠️
**Issue**: flying enemy + gravity pull = constant flight toward the well center. Disruptor becomes predictable but also stuck near the center, making bottle lobs erratic.
**Resolution**: disruptor's flight is **partially gravity-affected** — pulled toward well center but at a reduced rate (fights against pull, doesn't get sucked in). Implementation: 0.25 day, same pattern as Big Guy resistance.

#### L3 Gravity Well × Shaman — ⚠️ FORBIDDEN
**Issue**: shaman is meant to stay put for the entire cast. Gravity well pulls him toward the center, moving him mid-cast — telegraph breaks, cast position is unpredictable, player can't form a tactical plan.
**Resolution**: **Shamans do not spawn in gravity well rooms.** Hard restriction in encounter composition. Add to DUN-16's per-enemy spawn rules.

---

## Summary of Code Fixes Required

| Fix | Owner | Estimated Cost |
| --- | --- | --- |
| Big Guy AI treats water as uncrossable | Ryker (DUN-27) | 0.5d |
| Wind modifier filter (no effect on lobbed projectiles) | Robbie (DUN-24) | 0.25d |
| Parrot homing ignores wind | Nick (DUN-29) | 0d (just don't apply) |
| Bottles don't damage destructible cover | Robbie (DUN-24) collision filter | 0.25d |
| Shaman cast aura emits light | Ryker (DUN-27) | 0.5d |
| Disruptor delays fire during falling-roof telegraph | Ryker (DUN-27) | 0.25d |
| Shaman immune to altar heal during cast | Ryker (DUN-27) | 0.25d |
| Big Guy resists gravity well (2x weight) | Ryker (DUN-27) | 0.25d |
| Disruptor partial gravity resistance | Ryker (DUN-27) | 0.25d |
| **Total** | | **~2.5d added across DUN-24, 27, 29** |

These costs are small individually but real cumulatively. Worth flagging to Ryker that DUN-27 picks up ~1.75d of additional behavior work from this matrix.

## Summary of Encounter Composition Restrictions (Update DUN-16)

These are added to DUN-16's per-enemy spawn rules:

- **Big Guy pairs**: do not spawn in portal rooms. Do not spawn in water-pool rooms unless level geometry allows pathing around water.
- **Shaman**: do not spawn in gravity well rooms. In portal rooms, must spawn at positions requiring non-portal traversal. In altar rooms, immune to altar heal during cast. Spawn positions in dart-trap rooms must avoid plates. Spawn positions in cover rooms must have at least one clear line of approach.
- **Disruptor**: max 1 (not 2) in falling-roof rooms. Max 1 in altar rooms.
- **Parrot**: no general spawning. Wind doesn't affect homing (boss-fight rule).

## Summary of Level Design Constraints

Things that aren't code, but that level designers (Nick) need to verify per-room:

- Water-pool rooms: pathfinding around water is geometrically possible if Big Guy spawns.
- Portal rooms with Grunts: still threatening when player uses portals.
- Portal rooms with Shaman: shaman position requires non-portal approach.
- Dim-light rooms with Shaman: aura light visible across the room (this is mostly the code fix, but level designers should test it).
- Cover rooms with Shaman: at least one clear line of approach to the shaman.

---

## Dependencies

- **Depends on DUN-15 (Stat Sheet)** for tier interpretations.
- **Modifies DUN-16 (Composition Rules)** — restrictions above are appended to DUN-16's spawn rules.
- **Adds work to DUN-24, DUN-27, DUN-29** — the code fixes listed above.
- **Adds constraints to DUN-7 (Teaching Plan)** — level design must respect the level-design constraints above.
- **Feeds into DUN-33 (Integration & Tuning)** — playtest verifies the matrix verdicts hold up.
