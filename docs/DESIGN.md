# DESIGN.md

Single source of truth for the design of *Untitled Dungeon Crawler* (working title — see TODOs). This document supersedes prior design drafts.

For technical decisions, see `TECH.md`. For Linear ticket history, see the Draft Requirements project.

---

## 1. Core Identity

### Pitch

> *A top-down precision combat dungeon crawler where you parry your way through corrupted versions of your former pirate crew, learning each room's modifier before it kills you.*

### Story

The player is a regular crew member — nobody special — washed up on a mythical island after their ship wrecks. The captain and crew are missing: either dead at the wreck site or somewhere on the island. The player descends through the island to find them.

The island corrupts the people on it. The crew the player finds has been transformed into the bosses they fight. The final boss is the captain.

The tone is scared, angry, and confused — a regular person trying to find their friends, not a hero on a quest.

### References

- **Combat feel**: Nine Sols (parry-centric, color-coded unparryable attacks).
- **Dual-weapon system**: Akane (cutlass + flintlock, both usable in tandem).
- **Top-down perspective**: Enter the Gungeon (camera, sprite style, encounter design).
- **Level structure**: Old Zelda (deliberate, hand-designed rooms; central hub).

### Player Fantasy

A resourceful survivor reading each room's modifier and adapting. Combat is reactive and precise — not a power fantasy, a thinking-person's action game.

### Core Loop

1. Enter room
2. Read the room's modifier
3. Survive the encounter with cutlass + flintlock + parry
4. Progress
5. Boss at end of each level → unlocks next level → hub access expands

---

## 2. Combat

### Combat Verb

**Precision combat.** Player manually attacks, manually parries, manually positions. No auto-attack.

### Weapons (Two-Weapon Dual-Wield Kit)

- **Cutlass** — melee. Player starts with this.
- **Flintlock** — ranged. Acquired from the Level 1 boss mid-fight (boss drops it, player picks it up).
- **No third weapon.** This is the entire combat kit for the whole game.

Both weapons are usable in tandem (Akane-style) — player can switch fluidly between them, not weapon-swap-with-cooldown.

### Parry System (Locked — see `parry_spec.md` for full implementation spec)

**Scoped Nine Sols.** Parry as primary defense, color-coded unparryable attacks, satisfying audio/visual feedback. **Includes a perfect-parry tier in v1.**

#### Timing (frame-based at 60fps fixed simulation tick)
- **Parry window**: 8 frames. Tune in vertical slice.
- **Perfect parry window**: first **3 frames** of the parry window. Adjust to feel — easy fix in tuning.
- **Parry recovery on miss**: 12 frames.
- **Parry recovery on hit**: 5 frames.
- **Enemy stun on normal parry**: 24 frames (counter window).
- **Enemy stun on perfect parry**: extended (~40 frames target) with star-burst visual telegraphing the punish.

#### State Management
- **PlayerState FSM**. States: `Idle`, `Moving`, `Attacking`, `Parrying`, `ParryRecovery`, `ParrySuccess`, `Hurt`.
- Movement is **locked** during `Parrying`.

#### What Can Be Parried
- **Everything except attacks tagged unparryable.**
- Tagging via `AttackType` enum: `LightMelee`, `HeavyMelee`, `Projectile`, `Unparryable`.
- Melee parry: hitbox-overlap detection.
- Projectile parry: contact-during-window detection (separate system, sharing infrastructure).
- AoE attacks: unparryable by convention.

#### Unparryable Attack Standard
Red flash on the attacker during wind-up = unparryable. **Universal rule, no exceptions.** If an attack is unparryable, it's red. If it's red, it's unparryable.

#### Cost
No stamina or focus cost. Implicit cost: recovery frames on miss (12) and on hit (5). "Parry everything" mindset.

#### Parry Effects

**Normal Parry — Melee:** brief opening on the enemy (24-frame stun); player can attack or reposition.

**Normal Parry — Projectile:** projectile deflects in the **direction the player is facing** (facing set by movement, no aim input).

**Perfect Parry — Melee:** enemy enters stunned state with extended recovery window. Reusable **star-burst visual** appears above the enemy's head, sized per enemy/boss tier. Distinct audio sting separate from the normal parry ring.

**Perfect Parry — Projectile:** projectile is sent **straight back to the original sender** (owner reference is tracked in the projectile struct). Distinct audio sting.

#### Boss Parry Rules
- **Parry mechanics are identical against bosses.** No tightened window, no stagger gauge, no boss-unique parry system.
- Bosses are differentiated by **attack composition**: a much higher proportion of unparryable (red) attacks per phase. Difficulty scales via *content design*, not *system override*.

#### Feedback (v1, all committed)
- **Hit-stop** on successful parry (50–80 ms freeze).
- **Ring audio** on normal parry success; **distinct sting** on perfect parry success.
- **Blue sparks** on successful parry.
- **Visual indicator** (whiff effect) on missed parry.
- **Brief white flash** on parried entity (reuses hurt-flash code).
- **Star-burst stun asset** (reusable, scalable per enemy/boss tier).
- **Parry-ready UI indicator** — visible cue near player; dim/desaturated during recovery frames. Teaches cooldown without tooltips.

**Audio ownership** per DUN-6: Art & Audio Director (Ryker) owns parry SFX selection, with Design Lead (Nick) providing feel direction.

#### Scope Cuts (Explicit)
- No directional parry. Single button.
- No parry combo / chain bonuses.
- No upgradeable parry (longer window, bigger stun via items).
- No environmental parry (spike traps, falling debris).
- No time-slow on perfect parry.
- No auto-counter after parry. Skill expression stays in the player's hands.
- No stagger gauge for bosses.
- No tightened parry window for bosses.
- No partial-parry / "parryable for reduced damage" tier — binary outcomes only.

### Game Feel — Tier 1 (Committed)

- **Screen shake**: 2–4 px on hits, 8–12 px on explosions / phase transitions. Single tunable config.
- **Hit-stop**: 50–80 ms freeze when an attack connects.
- **Hurt flash**: 50 ms pure-white render on damaged entities.
- **Knockback**: per-weapon tunable.

### Game Feel — Tier 2 (Pick during slice)

- Pixel-art particle system (discrete chunks, not gradients).
- Camera lerp (10–15% smooth follow).
- Damage numbers (open question — see TODO in Art section).
- SFX on everything.

### Game Feel — Tier 3 (Only if time)

- Post-process shader (CRT / scanlines / chromatic aberration).
- Trauma-based camera shake (Squirrel Eiserloh's system).

---

## 3. Progression & Run Structure

### Progression Model

- **Progression-based with permanent unlocks.**
- **Central hub** where the player can revisit unlocked levels.
- New level unlocks **only after defeating that level's boss**.
- **Easter egg collectibles** are LAST PRIORITY — not part of vertical slice or core scope.

### Death Handling

- **On death: restart the current level from the beginning.**
- All rooms must be replayed.
- Permanent unlocks (weapons, level access) are retained across deaths.

**Escape hatch** (not built now): if Level 3 retrace proves too punishing in playtest, add a mid-level checkpoint at the boss door. Do not build pre-emptively.

---

## 4. Levels & Room Modifiers

### Level Themes

| Level | Theme | Identity | Aesthetic |
| --- | --- | --- | --- |
| 1 | Surface / Shipwreck | Natural forces. Player learns *what a modifier is*. | Lighter, brighter, swashbuckling. |
| 2 | Underground Catacombs | Architectural decay. Player learns *modifiers complicate combat*. | Dim, wet, claustrophobic. |
| 3 | Occult Temple | Supernatural and wrong. Player synthesizes everything. | Dim, bioluminescent, otherworldly. |

### Modifier Roster (9 total)

#### Level 1 — Surface
1. **Quicksand / Water Pools** — visual reskin of one mechanic. Slow movement + visible drain timer if player stays still. Affects enemies too.
2. **Wind Gusts** — periodic gusts in a specific direction. Telegraphed by visual particles + audio. Direction (east/west, etc.) lockable as scope permits.
3. **Destructible Cover** — broken crates, barrels, masts. Static cover that depletes with damage.

#### Level 2 — Catacombs
4. **Dim Light** — player-attached torch radius + room torches. Some torches start lit, some unlit (player interacts to light). Catacombs-exclusive; does not combine with Level 1 or 3 modifiers.
5. **Falling Roof** — dust particles + rumbling audio telegraph → delayed rock chunks fall in zone. Random with player-bias targeting. Telegraph delay starting value: 1.0 seconds.
6. **Dart Traps / Pressure Plates** — visible plates. Player can bait enemies onto them. Excluded from boss room.

#### Level 3 — Temple
7. **Portals** — deferred to dedicated scoping session (see DUN-13).
8. **Blood Altars** — passive heal aura for enemies in range. Destroyable. Explode on destruction.
9. **Gravity Well** — fixed point in room, constant pull on player + enemies + projectiles.

### Modifier Design Rules

- Modifiers change **the space** or **enemy behavior** — not how the player's weapons work.
- Per modifier, the team must answer:
  1. **Mechanic** — what does it change?
  2. **Tactic** — what's the optimal player response?
  3. **Variety** — how does it interact with each weapon and parry?
  4. **Scope** — how expensive to build?
- If "Tactic" can't be answered crisply, the modifier isn't designed — it's just randomness.

### Boss Room Modifier Rule

Boss rooms use only the level's **passive** modifier. Active hazards are excluded.

| Level | Boss-Room Modifier | Excluded from Boss Room |
| --- | --- | --- |
| 1 | Wind Gusts | Water Pools, Destructible Cover |
| 2 | Dim Light | Falling Roof, Dart Traps |
| 3 | Gravity Well | Portals (note: boss-controlled portals are boss mechanics, not room modifiers), Blood Altars |

### Level Structure

- **5–6 combat rooms per level + 1 boss room.**
- Total: 15–18 combat rooms + 3 boss rooms across the game.
- **One active modifier per combat room.** Combination rooms may stack modifiers from the **same level only**. Never cross-level.

### Teaching Plan (Framework — Room-by-Room Homework Deferred)

For each level, modifier rooms break into four categories:

- **Introduction rooms** (start of level): one modifier, forgiving encounter. Player notices the mechanic.
- **Complication rooms** (middle): modifier combines with denser encounters or a second same-level modifier. Player applies under pressure.
- **Combination rooms** (pre-boss): multiple same-level modifiers active, full enemy mix. Player synthesizes.
- **Boss room**: passive modifier only (see rule above).

**Owner**: Design Lead. Room-by-room sketches as homework before next meeting.

---

## 5. Enemies

### Design Philosophy

- Enemies stay functionally independent of room modifiers (reusable across rooms).
- Every enemy should *function* in every room — verified via DUN-17 interaction matrix.
- Enemies are the alphabet; encounters are the words.
- Every enemy has parry-tagged attacks (parryable / unparryable).

### Roster (4 enemies — final, no 5th)

#### Enemy 1 — Melee Grunt
- **Identity**: Zombie / thrall corrupted pirates.
- **Role**: Swarm threat.
- **Stats**: HP Low (1 hit), Damage Light, Speed Normal, Range Melee, Cooldown Short.
- **Parryable**: yes.
- **Spawn group**: 4–8 typical.

#### Enemy 2 — "Mo & Krill" Pair (Big Guy + Back-Mounted Shooter)
- **Role**: Positioning puzzle. Simplified evolution of the original archer/tank concept.
- **Big Guy**: HP High, Damage Heavy, Speed Slow, Melee.
- **Shooter**: separate hitbox on Big Guy's back. **Only damageable from behind, OR after killing Big Guy.**
- **Parryable**: Big Guy melee yes; shooter projectiles yes.
- **Spawn**: counts as one encounter unit. Max 2 pairs per room.

#### Enemy 3 — Disruptor (Flying Poison Bottle Thrower)
- **Role**: Environmental complicator.
- **Behavior**: Flies, kites the player, lobs poison bottles. Bottles create AoE poison puddles that DOT players standing in them.
- **Stats**: HP Low-Medium, Speed Fast (flying), Range Long, Cooldown Medium.
- **Parry**: bottle parryable. **Parried bottle hits only the flying enemy** (no aim = no AoE redirect, by design).
- **Spawn rule**: only spawns when at least one other enemy type is present. Complicator, not standalone.

#### Enemy 4 — Shaman (Priority Target)
- **Role**: Priority target — must be killed before completing ritual.
- **Behavior**: Slow, stays put, casts a buff ritual. Buff = **Enrage** (speed + damage to nearby enemies). **Self-destructs** on completing the ritual.
- **Stats**: HP Medium, Damage none directly, Speed Slow.
- **Cast duration**: 3–5 seconds, telegraphed.
- **Cancellation**: only by killing the shaman.

### Stat Sheet, Composition Rules, Interaction Matrix (Locked — see `docs/specs/`)

Locked specs (see `docs/specs/`):
- Stat tiers: DUN-15 — `enemy_stats_spec.md`
- Encounter composition: DUN-16 — `encounter_composition_spec.md`
- Modifier interaction matrix: DUN-17 — `interaction_matrix_spec.md`

---

## 6. Bosses

### Structure
- 2 minibosses + 1 final boss.
- Minibosses: 2 phases each.
- Final boss: 3 phases.

### Design Thesis
Each boss is the thesis statement of its level. The level teaches the modifier; the boss interrogates whether the player has learned it. The level is the boss's de facto first phase.

### Boss Identities

The bosses are the player's former crewmates, corrupted by the island.

| Level | Boss | Original Role |
| --- | --- | --- |
| 1 | **Silas the Wind Bitten** | Former Gunner |
| 2 | **Dr. Carver** | Ship's Doctor |
| 3 (Final) | **Captain Crane the Many Handed** | Captain |

### Phase Structure Rule
- **Phase 1**: standard fight under the level's passive modifier. Establishes rules.
- **Phase 2 (minibosses) at 50% HP**: modifier amplified to extreme.
- **Phase 2 (final boss)**: modifier amplified.
- **Phase 3 (final boss only)**: new attacks layered on the established modifier. Familiar test → punishing test → something never seen.

### HP Architecture
- **Minibosses**: 1 HP bar, 50% threshold triggers Phase 2.
- **Final boss**: 3 segmented bars. Phase transitions at 66% and 33%.

### Boss-Room Modifier Rule (Confirmed Per Boss)

| Boss | Arena Modifier | Excluded |
| --- | --- | --- |
| Silas | Wind Gusts only | Quicksand/Water, Destructible Cover |
| Dr. Carver | Dim Light only | Falling Roof, Dart Traps |
| Captain Crane (Phase 1-2) | Gravity Well only | Room-modifier portals, Blood Altars |
| Captain Crane (Phase 3) | TBD — see gaps | — |

Boss-controlled portals are *boss attacks*, not room modifiers. They are unaffected by this rule.

---

### Mini-Boss 1 — Silas the Wind Bitten (Former Gunner)

**Arena modifier**: Wind Gusts.

**Phase 1**
- Parryable flintlock shots fired in quick succession (prevents the player from camping a single parry stance).
- Additional Phase 1 attacks: **TBD — see gaps**.

**Phase 2 (50% HP)**
- Silas builds up to a **red (unparryable) bullet** — visible charge-up telegraph during the build. Player must dodge. This is the headline punishing attack of the amplified phase.

**Phase Transition Theater**
- TBD — see gaps.

**Parry Tags**
- Standard bullets: parryable.
- Charged red bullet: unparryable (must dodge).

---

### Mini-Boss 2 — Dr. Carver (Ship's Doctor)

**Arena modifier**: Dim Light.

**Phase 1**
- Phase 1 takes place inside or around a large summoning bulge. Carver remains inside; periodic blood attacks emerge.
- Additional Phase 1 attacks: **TBD — see gaps**.

**Phase 2 (50% HP) — Bubble Burst**
- The bulge ruptures in a **wet ripping sound** (style: water balloon tearing).
- Dr. Carver curls on the ground, summoning his rage.
- Screen shakes, minimal red aura builds at screen edges.
- Once full, Carver **screams in agony/rage** — Phase 2 begins.

**Phase 2 Attacks**
- **Blood spikes ripping through the ground** — parryable. Designed to be *hard to parry consistently* (fast, multi-hit, tight windows) rather than partially negated. Encounter design carries the punishment, not the parry system.
- **Blood wave / blood-goop AoE** — periodic, large coverage, unparryable. Telegraphed with enough lead time for the player to relocate.

**Parry Tags**
- Blood spikes: parryable (but designed to punish lazy parrying).
- Blood waves / AoE: unparryable (must dodge).

---

### Final Boss — Captain Crane the Many Handed

**Arena modifier (Phases 1-2)**: Gravity Well. **Phase 3 arena modifier: TBD — see gaps.**

**Entrance Sequence**
- Player walks an eerie, dark, open cave on a thin rock walkway leading to the temple. Moonlight glow. Water droplets / falling pebbles into an endless pit.
- At the temple gates: screen hums and shakes; giant doors open.
- Inside: the old, tired captain on his throne. He rises. Camera pans to him stepping through a portal. Camera pans back to the player. Fight begins.

**Phase 1 — Fists and Anchor**
- **Giant fists** punch in from offscreen at walls/pillars. All fist impacts **synced** to a single screen shake per instance (avoids overwhelming the player with multiple shakes).
- **Fists are unparryable.** Forces dodging.
- **Anchor-and-chains attack** (anchor replaces lance — better fits the pirate theme).
  - Anchor head impact deals heavy damage; a direct trajectory hit can be lethal.
  - Chain trail deals chip damage on contact, not parryable. Communicates "don't touch the chain."
  - Distinct audio: loud bell or anchor-clang on wall impact, with a less violent screen shake than the fists.
  - **Implementation note**: Phase 1 ships with a static or pre-animated rigid chain. Articulated chain segments are a stretch upgrade — Ryker scopes the art cost and decides whether v1 ships with chain or just the anchor head.

**Phase 2 — Finger-Flick and Arena Changes**
- **Giant finger pokes through a portal** and flicks a single object (rock or orb — final art TBD; orb is easier to animate consistently).
- Flicked object is **parryable**. Player who fails to parry takes heavy damage plus a violent screen shake (no stun — see Phase 3 note).
- Arena reconfigures between stun cycles (3-4 layout variants).

**Phase 3 — Portal Dimension, Giant Hands**
- Player follows boss into portal dimension. New map.
- **Giant hand slaps and slams**: unparryable. On hit:
  - Screen flashes white, then dark vignette for ~0.3 seconds.
  - Player is movement-locked briefly (~0.5s stagger).
  - Player remains controllable post-flash; the brief lock is the stun.
- Phase 3 is "dodge the hands; parry only as a last resort to nothing-effects" — but since hands are unparryable, parry is genuinely not an option here. The phase tests dodge-reading exclusively.

**Boss Release**
- v1 (ships in slice): single particle burst + distinct sound cue when Crane is defeated.
- **Stretch goal** (final two weeks if ahead of schedule): cinematic slow-motion release sequence — captain freed from corruption, dramatic music swell, fade.
- Mini-bosses get the same treatment scaled down: single particle burst + sound cue per defeat.

**Parry Tags**
- Fists (Phase 1): unparryable.
- Anchor head (Phase 1): unparryable.
- Chain trail (Phase 1): contact damage, not parryable.
- Finger-flicked object (Phase 2): parryable.
- Giant hands (Phase 3): unparryable.

---

### Stun Timer Mechanic (Phase 1 Final Boss)

- Boss is invulnerable except during stun windows triggered by hitting glowing fists/lances.
- Stun duration: **5-7 seconds per stun** (starting target — tune in vertical slice).
- Player damage-output during stun determines whether Phase 1 takes 2-4 cycles.
- Final values calibrated once weapon damage and boss HP tiers are locked.

---

### Non-Negotiable Polish (Reaffirmed)

- **Telegraphs**: every attack must have a clear wind-up (see DUN-18).
- **Phase transitions are visible events**: distinct audio stinger, visual change, brief boss invulnerability/immobility (1-2s).
- **Red flash convention**: all unparryable attacks flash red during wind-up.

---

### Resolved TODOs (from DUN-9)
- ✅ Crew-identity reconciliation (Silas / Carver / Crane)
- ✅ Boss HP architecture
- ✅ Boss-room modifier confirmation (Phases 1-2)
- ✅ Telegraph specs for major attacks (parryable/unparryable tagging)
- ✅ Phase transition theater for Carver and Crane

### Remaining Gaps (Nick to Fill)
- ❗ **Phase 1 attack lists for Silas and Dr. Carver** — 3-4 attacks each minimum.
- ❗ **Phase 3 arena modifier for Captain Crane** — gravity well persists, or clean arena?
- ❗ **Silas Phase 1 → Phase 2 transition theater** — currently unspecified.

### Remaining Gaps (Ryker to Scope)
- 🎨 **Anchor-and-chains art**: decide v1 ships with anchor only, static chain, or articulated chain segments.
- 🎵 **Per-boss audio palette**:
  - Silas: wind + flintlock cracks.
  - Carver: wet / squelch / agony screams.
  - Crane: deep bass + anchor-on-stone + eerie bell.

### Explicit Rejections (Scope Discipline)
- ❌ **Partial parry / "parryable for reduced damage"** — rejected. Contradicts DUN-14's binary parry system. Hard-to-parry attacks are handled via encounter design (fast cadence, tight windows), not via a new partial-damage tier.
- ❌ **Coin-mix unparryable bullets** for Silas — rejected. Build-up unparryable bullet is the single Phase 2 punishing attack.
- ❌ **Full black screen on giant hand hit** — rejected. Soft flash + vignette instead.
- ❌ **Cinematic boss release in v1** — deferred to stretch.

---

## 7. Art Direction

### Aesthetic

- Top-down (Gungeon-style), not isometric.
- **Level 1**: lighter, brighter, swashbuckling.
- **Level 2 & 3**: dim, wet, bioluminescent, otherworldly.
- No jumping. No verticality. No platforming.

### Locked Constraints

| Constraint | Value |
| --- | --- |
| Base resolution | 384×216 |
| Character sprite size | **32×32** (aligns to 2×2 tiles on the 16×16 grid) |
| Enemy sprite size | 32×32 |
| Boss sprite size | 32×32 normal / 48×48 large / 72×72 if huge |
| Tile size | **16×16** (smaller than characters for spacious layouts + combat readability) |
| UI icons | 24×24 |
| Item pickups | 24×24 |
| Projectiles | 8×8 or 12×12 |
| Effects / VFX | 24×24 or 48×48 |
| Palette | **Lospec Endesga 32** — 32 colors flat, no more no less |
| Shading | 2–3 shades per color |
| Art direction | Less-detailed / charming style (chosen for scope + asset production speed) |

**Sprite-to-grid note**: 32×32 characters were chosen specifically so a character occupies exactly 2×2 tiles on the 16×16 grid. (Earlier the spec said 24×24; this was revised to fix grid misalignment — 24px characters span an awkward 1.5 tiles.)

### Animation Frame Budgets

#### Player
- Idle: 4 frames
- Run: 6 frames (single run animation; no walk/run toggle, no stamina system in v1)
- Attack (cutlass): 4–6 frames
- Attack (flintlock): 4–6 frames
- Parry: 2–4 frames
- Hurt: 2 frames
- Death: 6 frames

#### Enemy
- Idle: 2–4 frames
- Move: 4–6 frames
- Attack telegraph: 2–3 frames
- Attack: 4 frames
- Hurt flash: color swap (no new frames)
- Death: 4–6 frames

#### Boss
- 2–3× per-enemy budget.

#### Effects
- Slash: 4 frames
- Explosion: 6–8 frames
- Smoke / Dust: 4–6 frames
- Sparks (parry): TBD

### Animation Speeds
- Idle: 5 fps
- Walk: 10 fps
- Attack: 14 fps
- Effects: 16 fps

### Content Scope

- Tiles: 50–80
- Effects: 5–10
- Total sprite frames: 150–250

### Asset Pipeline

- Aseprite (compiled from source — free).
- Workflow: Concept → Sketch → Pixel Asset Creation → Animation → Export → Engine Import.
- Export as PNG sprite sheets, transparent backgrounds, organized layers, consistent dimensions.
- `.ase` files committed via Git LFS; PNGs exported at build time via Aseprite CLI from CMake.

### Folder Structure

Lowercase naming convention globally.

```
assets/
├── sprites/      # exported/working sprite assets
├── audio/        # music + SFX
├── ui/           # UI art
├── shaders/      # shader files (if/when Tier 3 post-fx lands)
└── aseprite/     # .ase source files (LFS-tracked)
```

**Note**: `.ase` sources live in `assets/aseprite/` (LFS). Reconcile generated-PNG output location with TECH.md asset pipeline — see TODO.

### Player Character Design

- Pirate-themed visual identity: traditional pirate garb (Blackbeard-style), **red bandana for silhouette recognition**.
- Distinct, readable silhouette is the priority.

### Resolved (DUN-10)
- ✅ Sprite size: 32×32 characters, 16×16 tiles (grid-aligned).
- ✅ Art style: less-detailed / charming (scope-driven).
- ✅ Run animation: single run, no walk/run toggle, no stamina.
- ✅ Tier 1 game feel confirmed in scope from Day 1 (screen shake, hit-stop, hurt flash, knockback) — acknowledged by Art Lead and for programming.
- ✅ Damage numbers: out of scope for v1.
- ✅ Palette: Lospec Endesga 32.
- ✅ Folder casing: lowercase globally.
- ✅ Player character: pirate garb + red bandana.

### Remaining TODOs
- [ ] **UI mockups** — HUD (health, parry-ready indicator, weapon icons), pause menu, settings, main menu, hub. To be drafted for internal team review. Art & Audio Director.
- [ ] **Pirate-themed visual identity pass** on enemies 2, 3, 4 (Big Guy pair, flying poison thrower, shaman) — redesign to fit corrupted-pirate/island-crew theme before final production. Art & Audio Director + Design Lead.
- [ ] **Asset folder reconciliation** — DESIGN.md art folders (`assets/sprites/`, `assets/ui/`, `assets/shaders/`) vs. TECH.md pipeline (PNGs generated at build time into `build/assets/sprites/`). Decide whether `assets/sprites/` holds committed working art or is purely generated output. Art & Audio Director + Tech Lead.

### Stretch Ideas (Not Committed)
- 🧢 **Cosmetic rewards from boss battles** (hats, coats, wearables). Charming and on-theme, but requires a wardrobe/equip system + player sprite layering/swapping + per-cosmetic art. **Post-vertical-slice consideration only.** Not a v1 commitment.

---

## 8. Audio

### Locked Constraints

#### SFX Durations (Target Ranges)
- Player jump: 0.10–0.20 s *(may not apply — no jump in game)*
- Attack swing: 0.10–0.25 s
- Hit / Hurt: 0.10–0.30 s
- Enemy death: 0.30–0.80 s
- Explosion: 0.50–1.20 s
- UI click / select: 0.05–0.12 s
- Pickup: 0.15–0.40 s
- Footsteps: 0.08–0.15 s

#### Music Specs
- Loop length: 30–90 s
- Format: `.ogg`
- Target size: < 5 MB per track
- **Menu**: calm / simple.
- **Gameplay**: faster rhythm, repetitive loop.
- **Boss**: heavier percussion + stronger bass.

#### Volume Defaults
- Master: 100%
- Music: 35–50%
- SFX: 70–90%
- UI: slightly quieter than combat.

#### Mixing Guidelines
- Cut muddy lows below 80 Hz.
- Boost clarity 2k–5k Hz.
- Peaks below -1 dB.
- Normalize exports to ~-3 dB.

#### SFX Style
Retro arcade / pixel-art: short, punchy, high contrast, minimal reverb.

#### Audacity Effect Chain
Normalize → light Compressor → EQ Filter Curve → Fade In/Out → light Reverb → Paulstretch only for ambient.

### Inventory Targets

- Player SFX: 8–12
- Enemy SFX: 10–20
- UI sounds: 5–8
- Ambient sounds: 3–5
- Music tracks: 3–6

### File Organization
```
assets/audio/
├── sfx/
│   ├── player/
│   ├── enemies/
│   ├── ui/
│   └── environment/
└── music/
    ├── menu/
    ├── gameplay/
    └── boss/
```

### Naming Convention
```
player_jump_01.wav
player_attack_01.wav
enemy_hit_02.wav
ui_click_01.wav
bgm_forest_loop.ogg
```

### Audio Direction & Production Decisions (Resolved — DUN-11)

#### Music Source Strategy
- **Hybrid**: original compositions + royalty-free tracks.
- In-house processing/editing in **Audacity**.

#### SFX Pipeline
- **Generation**: Bfxr (retro-styled generated SFX).
- **Editing/layering**: Audacity (trimming, volume balancing, layering, export formatting, cleanup).
- **Supplemental**: Freesound.org for environmental/sampled effects when generated sounds fall short.
- **Flow**: Bfxr → Audacity → Export → `assets/audio/sfx/`.

#### Music Ownership
- Art & Audio Director assembles a 15–20 track candidate playlist for review; team selects ~5–8 final.

#### Track Inventory & Reuse
- Tracks reused across scenes to keep scope manageable. Shared between hub / level ambience / miniboss / menu where appropriate.
- Boss phases use **layered variations or modified playback timing**, not entirely unique compositions.

#### Parry Sound Design
- **Successful parry**: sharp metallic ring, high-frequency emphasis, immediate readability.
- **Failed / mistimed parry**: muted thud / dull impact, clearly communicates failure.
- **Perfect parry**: distinct sting separate from the normal ring (per `specs/parry.md`).
- Goals: recognizable in high combat intensity, reinforces timing precision, minimal overlap with weapon-hit SFX.
- **Sync**: triggers on parry-confirmation frames alongside hit-stop, hurt flash, knockback, optional light screen shake. Timing must be immediate for responsiveness.

#### Red-Attack Audio Convention
- Shared warning signature across all enemies and bosses.
- Sharp warning stinger **before release** (during attack startup, not impact), distinct tonal identity, corrupted/unnatural texture, harsh scrape, consistent timing window.
- All unparryable attacks must trigger the shared cue. Bosses may layer additional sounds while preserving core recognizability.

#### Phase-Transition Stingers
- Every boss phase transition includes a dedicated stinger.
- ~1–3 seconds, dramatic, contrasts normal combat audio, avoids excessive interruption.
- Accompanied by temporary music ducking/pause + a brief gameplay-emphasis window.
- Palette: cursed pirate horn, distorted bell/chime, low-frequency swell.
- Syncs with cinematic pauses, animation locks, screen shake, boss transformation effects, arena changes.

#### Audio Engine
- **Raylib built-in audio confirmed sufficient for v1.**
- Included: music/SFX playback, basic mixing, simultaneous playback, looping, volume control.
- Out of scope: sidechaining, advanced ducking, spatial audio, adaptive music, external middleware.

#### Volume Defaults (carried from earlier)
- Music ~40%, SFX ~70% at first launch.

---

## 9. Open TODOs / Cross-Cutting

### Highest Priority
- [ ] **Game name** — currently `<GameName>`. Lock by end of Week 2.
- [ ] **Teaching plan room-by-room sketches** — Design Lead homework before next meeting.
- [ ] **Pirate-themed visual identity pass** on enemies 2-4 — Art & Audio Director + Design Lead (see Section 7).
- [ ] **UI mockups** — HUD, pause, settings, main menu, hub — Art & Audio Director (see Section 7).

### Medium Priority
- [ ] **Meeting cadence — specific recurring days/times.** Structure locked (2 standups + 1 playtest, 3–6pm, 1 buffer day); pick the actual days.
- [ ] **VSCode `clang-format` / `clang-tidy` integration in README** — configs and CI are in repo; user-facing setup docs still pending. Tech Lead.

### Low Priority (Config Cleanup)
- [ ] **`.clang-tidy` `HeaderFilterRegex`** references `debug-plus/` path — generalize or rename once the game has a name. Tech Lead.

### Deferred
- [ ] CI / automated builds beyond format/lint enforcement — post-vertical-slice.
- [ ] Easter egg collectibles — LAST priority.

### Resolved
- [x] **Crew-identity boss reconciliation** — Silas the Wind Bitten / Dr. Carver / Captain Crane the Many Handed. See Section 6.
- [x] **Portals scoping session (DUN-13)** — locked. See `portals_spec.md`.
- [x] **Parry system spec** — locked. See `parry_spec.md` and Section 2.
- [x] **C++ standard alignment** — C++20 locked across `TECH.md`, `.clang-format`, `.clang-tidy`.
- [x] **Art direction (DUN-10)** — sprite/tile sizes, palette (Endesga 32), art style, run animation, player design, folder casing. See Section 7.
- [x] **Audio direction (DUN-11)** — hybrid music, Bfxr→Audacity pipeline, parry/red/transition audio specs, Raylib engine confirmed. See Section 8.

---

## Mentor Notes

The biggest risk on a project like this isn't technical — it's **scope creep and unfinished verticals**. When in doubt, cut. Three months goes faster than you think.

When the team disagrees, domain owners have final say in their domains. Tech Lead has final say on scope across the board. This is a learning experience first, a shippable game second.

The vertical slice (one level, one enemy, one boss, parry working, hit-stop and screen shake working, programmer art) by end of Week 3–4 is the single most important early milestone. Everything in this document should serve that goal. If a feature listed here gets in the way of the slice, defer it.
