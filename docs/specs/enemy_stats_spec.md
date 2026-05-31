# Enemy Stat Sheet — Locked Spec

**Linear**: [DUN-15](https://linear.app/dungeon-crawler-game/issue/DUN-15)
**Status**: LOCKED. Values are *starting targets* — tune in vertical slice playtest.

---

## Summary

Relative stat tiers for all 5 enemies. Establishes shared vocabulary so the team can design encounters with consistent expectations. Final numbers come from playtesting; the tiers are the contract.

Tiers, not raw numbers, are the load-bearing decision here. "Grunt has Low HP" is the durable design intent; whether that maps to 1 hit or 25 HP at final tuning doesn't change encounter composition logic.

---

## Tier Framework

| Stat | Low | Medium | High | Very High |
| --- | --- | --- | --- | --- |
| **HP** | 1 hit | 2–3 hits | 4–6 hits | 8+ hits |
| **Damage** | Chip (<10% player HP) | Light (10–20%) | Heavy (25–35%) | Lethal (50%+) |
| **Move Speed** | Slow (~50% player) | Normal (~100% player) | Fast (~130% player) | Very Fast (~170% player) |
| **Attack Range** | Melee only | Short (~1 tile reach) | Medium (~3 tiles) | Long (~6+ tiles / room-spanning) |
| **Attack Cooldown** | Constant (continuous threat) | Short (1–2s) | Medium (3–5s) | Long (6–10s) |

**Player HP target**: 4 hits at base damage (i.e., a Light-damage enemy kills the player in ~10 hits, a Heavy enemy in 3–4, a Lethal enemy in 2). Tune in slice.

---

## Per-Enemy Tiers

### Enemy 1 — Melee Grunt

| Stat | Tier | Notes |
| --- | --- | --- |
| HP | Low | 1 hit by cutlass. Swarm threat, not damage sponge. |
| Damage | Light | Individually weak; danger comes from numbers. |
| Move Speed | Normal | Matches player. Can be outrun but not by much. |
| Attack Range | Melee only | Must close to threaten. |
| Attack Cooldown | Short | ~1.5s. Continuous pressure when in range. |

- **Spawn group**: 4–8 typical per encounter.
- **Parry tag**: parryable (white telegraph).
- **AI**: chase player → swing in melee range → recover → repeat.

### Enemy 2 — "Mo & Krill" Pair (Big Guy + Shooter)

#### Big Guy (front)
| Stat | Tier | Notes |
| --- | --- | --- |
| HP | High | 4–6 hits. Pressure-bullet sponge. |
| Damage | Heavy | Big telegraphed melee. Punishes greedy approaches. |
| Move Speed | Slow | Predictable, plodding closer. |
| Attack Range | Melee only | Forces player into commit-or-flank decision. |
| Attack Cooldown | Medium | ~3s. Gives player flanking windows. |

#### Back-Mounted Shooter
| Stat | Tier | Notes |
| --- | --- | --- |
| HP | Medium | 2–3 hits **when exposed.** Untargetable from the front. |
| Damage | Light | Sniper pressure, not lethal alone. |
| Move Speed | — | Bound to Big Guy. |
| Attack Range | Long | Fires across the room. |
| Attack Cooldown | Medium | ~3s. Predictable rhythm. |

- **Spawn**: counts as one encounter unit. Max 2 pairs per room.
- **Parry tags**: Big Guy melee = parryable; shooter projectiles = parryable.
- **Critical rule**: shooter is only damageable from behind, OR after the Big Guy dies. This is the puzzle.
- **AI**: Big Guy interposes between player and shooter; shooter holds position behind Big Guy.

### Enemy 3 — Disruptor (Flying Poison Bottle Thrower)

| Stat | Tier | Notes |
| --- | --- | --- |
| HP | Low-Medium | 2 hits. Squishy when reached. |
| Damage | Light on bottle hit, Chip per tick in puddle | Layered threat. |
| Move Speed | Fast (flying) | Kites the player; hard to corner. |
| Attack Range | Long (lobs bottles) | Indirect — bottles arc, leaving puddles. |
| Attack Cooldown | Medium | ~3s. Bottle lands, creates ~3s DOT puddle. |

- **Spawn rule**: only with another enemy type present. Complicator, not standalone.
- **Spawn timing**: mid-fight (after some other enemies are dead), not at room entry.
- **Max per room**: 2.
- **Parry tag**: bottles parryable. **Parried bottle hits only the flying enemy** (no aim = no AoE redirect).
- **AI**: kite away from player → lob bottle → relocate → repeat.

### Enemy 4 — Shaman (Priority Target)

| Stat | Tier | Notes |
| --- | --- | --- |
| HP | Medium | 2–3 hits. Must be killable in cast duration. |
| Damage | None directly | The buff is the danger. |
| Move Speed | Slow | Positions and stays put. |
| Attack Range | N/A | Doesn't attack. Buffs nearby allies. |
| Attack Cooldown | Cast (one-shot) | Self-destructs on completion. |

- **Spawn rule**: at most 1 per encounter. Spawns at room entry (player must read threat immediately). Always with at least 2 other enemies to buff.
- **Cast duration**: 3–5 seconds, telegraphed (audio + visual aura buildup).
- **Buff**: **Enrage** — speed + damage boost to nearby allies.
- **Resolution**: self-destructs on completing the ritual. Buff remains on enemies for the rest of the encounter.
- **Parry tag**: cast is not parryable (it's a ritual, not an attack). The only counter is killing the shaman before completion.

### Enemy 5 — Corrupted Parrot (Boss-Spawned)

| Stat | Tier | Notes |
| --- | --- | --- |
| HP | Low | 1–2 hits. Pressure, not sustained threat. |
| Damage | Light on contact | Single hit per encounter; despawns after contact. |
| Move Speed | Fast (flying, homing) | Erratic, void-touched trajectory. |
| Attack Range | Contact-melee | Collides with player. |
| Attack Cooldown | N/A | Single homing approach until killed or contact. |

- **Spawn rule**: **boss-spawned only.** Silas summons in his Phase 1.
- **Concurrency**: tune so Silas has 1–2 active at any time during Phase 1. More overwhelms the player while they also dodge gatling fire.
- **Parry tag**: contact-parry deflects the bird back, briefly stuns it.
- **AI**: home toward player at constant speed; on contact, deal damage and despawn; on parry, deflect and stun ~1s.

---

## Tuning Order (Vertical Slice)

When tuning during slice, do it in this order. Each stage builds on the previous.

1. **Player HP & damage first.** Lock the baseline — how many hits does the player take, how many does the player give? Everything else is relative to this.
2. **Grunt next.** Grunt is the most-fought enemy; its difficulty calibrates the player's sense of base combat.
3. **Big Guy pair.** Tune Big Guy HP so flanking is the obvious correct play, not focus-fire.
4. **Boss tuning last.** Bosses scale to the player's mastery, which only exists after the player has fought enough regular enemies.
5. **Disruptor, Shaman, Parrot** — these are situational; tune them alongside the boss tuning since they appear in late or boss encounters.

---

## Implementation Notes

- Stats live in **per-enemy data structs / config files**, not hardcoded into AI classes. Tuning during playtest must not require recompiling.
- HP, damage, speed, cooldowns are all `float` fields exposed via a simple JSON or constant table. Tier labels (Low/Medium/High) live in comments for design clarity, not in code.
- "Tier" mappings to numbers can shift during tuning. The *tier* is the contract; the *number* is the dial.

---

## Dependencies

- **DUN-26 (Parry)** must be functional before parryable-vs-unparryable tagging can be tested.
- **Feeds into DUN-16 (Encounter Composition)** — encounter design uses these tiers to compose rooms.
- **Feeds into DUN-17 (Interaction Matrix)** — tiers inform which enemies remain dangerous in which modifier rooms.
- **DUN-27 (Grunt + Health Component)** uses placeholder tiers until tuning; this spec is the post-tuning target.
