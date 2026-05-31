# Encounter Composition Rules — Locked Spec

**Linear**: [DUN-16](https://linear.app/dungeon-crawler-game/issue/DUN-16)
**Status**: LOCKED. Composition rules can be tuned in playtest; the architecture (density tiers, per-enemy spawn rules) is stable.

---

## Summary

Spawn rules: who appears with whom, in what counts, and under what conditions. Enemies are the alphabet; encounters are the words. This spec defines the words.

Three layers:
1. **Per-enemy spawn rules** — hard constraints on each enemy's appearance.
2. **Density tiers** — Light / Medium / Heavy, mapped to room purpose (intro / complication / pre-boss).
3. **Per-level bias** — which enemies and densities show up in L1 / L2 / L3.

---

## Per-Enemy Spawn Rules

### Grunt
- **Free spawn.** Any composition, any room (subject to per-level bias).
- **Min count when present**: 3. Below this, the swarm identity disappears.
- **Max per room**: 8. Above this, it becomes a soup, not an encounter.
- **Spawn timing**: at room entry.
- **No "infinite wave" mode.** Finite spawn count; killing all = room clear.

### Big Guy + Shooter Pair
- **Counts as one encounter unit** in composition logic.
- **Max 2 pairs per room.** Beyond that, positioning becomes unreadable.
- **Composition rules**:
  - Spawns solo (1 pair, no grunts).
  - Spawns with grunts (1 pair + grunts).
  - Never spawns with another pair in tight rooms (corridors < 6 tiles wide).
- **Spawn timing**: at room entry. Big Guy needs to position before combat begins.

### Disruptor (Flying Poison Bottle)
- **Complicator rule**: only spawns when at least one other enemy type is present. Never solo.
- **Max per room**: 2.
- **Spawn timing**: **mid-fight**, not at room entry. Trigger: 50% of starting enemies dead.
- **Why mid-fight**: complicates the *back half* of an encounter when the player has already settled into their pattern.

### Shaman (Priority Target)
- **Max per encounter**: 1.
- **Spawn timing**: at room entry. Player must see the threat immediately.
- **Required co-spawn**: at least 2 other enemies (the shaman's buff target — solo shaman is meaningless).
- **Positioning constraint**: shaman should spawn in a position that's *reachable but not trivially close*. The player must commit to closing.

### Parrot (Boss-Only)
- **Boss-spawned only.** Never appears in regular encounter rooms.
- Silas summons in his Phase 1; tune to 1–2 active at any moment.
- **Not part of any encounter composition rule below.** Boss-fight specific.

---

## Density Tiers

Every combat room maps to one of three density tiers. The tier dictates which enemies and how many.

### Light (Intro / Teaching Rooms)
Player learns the modifier and basic combat. Forgiving composition.

**Allowed compositions**:
- 2–3 Grunts only.
- 1 Big Guy pair, no grunts.

**Not allowed**: Disruptor, Shaman.

**Use for**: first room of a level. Goal — player notices the modifier without dying.

### Medium (Complication Rooms)
Player applies what they've learned under pressure. Mixed compositions.

**Allowed compositions**:
- 4–5 Grunts + 1 Disruptor (mid-fight).
- 1 Big Guy pair + 2–3 Grunts.
- 3–4 Grunts + 1 Shaman.

**Not allowed**: two complicators (Disruptor + Shaman) in the same room. Pick one.

**Use for**: middle of a level. Goal — combine modifier knowledge with tactical reading.

### Heavy (Combination / Pre-Boss Rooms)
Player synthesizes. All four roster enemies may appear. Modifiers stacked from the same level.

**Allowed compositions**:
- 1 Big Guy pair + 3 Grunts + 1 Disruptor + 1 Shaman (maximum density).
- 2 Big Guy pairs + 2 Grunts (positioning gauntlet).
- 6+ Grunts + 1 Shaman + 1 Disruptor (swarm + complicators).

**Use for**: 1–2 rooms before each boss. Goal — full test of player's mastery.

### Boss Rooms
No regular enemies in boss rooms. **Exception**: bosses may *summon* enemies as part of their attack pattern (Carver → Grunts; Silas → Parrots). Boss-summoned enemies follow boss-specific rules, not this composition layer.

---

## Per-Level Encounter Bias

### Level 1 — Surface / Shipwreck (Lighter density)
Player is learning the game. Errs toward simpler compositions.

- **Light rooms**: 2 (intro + after first modifier room).
- **Medium rooms**: 2–3 (mostly Grunt-driven).
- **Heavy rooms**: 1 (introduces Big Guy pair before the boss).
- **No Shamans in Level 1.** Save the priority-target archetype for Level 2.
- **Disruptor introduction**: late in Level 1, after the player has handled Big Guys.

### Level 2 — Catacombs (Medium density)
All non-Parrot enemies in rotation. Shaman introduced in mid-late rooms.

- **Light rooms**: 1 (modifier intro for dim light).
- **Medium rooms**: 3 (varied compositions).
- **Heavy rooms**: 1–2 (full mix, dense pre-boss).
- **Shaman introduction**: mid-Level 2.
- **Big Guy pairs more common** here.

### Level 3 — Temple (Heavy density)
Player is fully fluent. Dense compositions. Frequent shaman use.

- **Light rooms**: 0–1.
- **Medium rooms**: 2.
- **Heavy rooms**: 3+ (most rooms are Heavy by this point).
- **Maximum density encounters appear** here — this is the test.

---

## Mid-Fight Spawning Rules

- **Finite spawn count per room.** Every room has a known enemy count; killing all = room clear = doors open. No "infinite wave" rooms.
- **Mid-fight spawns are allowed for**:
  - Disruptors (triggered at 50% starting enemies dead).
  - Shaman replacements *only if explicitly designed for it* (default: shaman doesn't respawn).
- **Mid-fight spawns are NOT allowed for**:
  - Grunts (always at room entry).
  - Big Guy pairs (always at room entry).
- **Boss-summoned enemies** (Grunts from Carver, Parrots from Silas) are exempt — boss controls their own spawn logic.

---

## Encounter Composition Quick Reference

| Density | Grunts | Big Guy Pairs | Disruptor | Shaman |
| --- | --- | --- | --- | --- |
| Light | 2–3 OR 0 | 0 OR 1 | — | — |
| Medium | 2–5 | 0–1 | 0–1 (mid-fight) | 0–1 |
| Heavy | 3–8 | 0–2 | 0–2 (mid-fight) | 0–1 |
| Boss | — | — | — | — |

(Pick *one* complicator in Medium: Disruptor OR Shaman, not both. Heavy allows both.)

---

## Implementation Notes

- Encounters are **data, not code**. Each room has a config (JSON or constant table) listing: enemy types, counts, spawn positions, spawn triggers.
- The room/level container (DUN-28) reads the config and spawns accordingly.
- **No procedural generation.** Designers hand-author each room's composition during level design.
- Mid-fight spawn triggers are simple counters: `if (enemies_killed_this_room >= ceil(starting_count * 0.5)) spawn_complicator();`

---

## Open Decisions Deferred to Playtest

- **Exact grunt counts within tiers** — "4–8 typical" is the design intent; tune in slice.
- **Shaman cast duration tuning** — currently 3–5s. If too short, player can't cross room in time; if too long, trivial to kill.
- **Disruptor max-2-per-room** — may need to drop to 1 if the AoE puddles overlap too aggressively.
- **Mid-fight spawn trigger threshold** — 50% is the starting point; may shift to 33% or 66% based on pacing feel.

---

## Dependencies

- **Depends on DUN-15 (Stat Sheet)** — composition tiers reference enemy difficulty tiers.
- **Feeds into DUN-7 (Room-by-Room Teaching Plan)** — Nick uses this to author per-room compositions.
- **Feeds into DUN-28 (Room/Modifier implementation)** — config schema for encounter data lives here.
- **Cross-references DUN-17 (Interaction Matrix)** — composition rules forbid combinations the matrix marks as broken.
