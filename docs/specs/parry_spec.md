# Parry System — Locked Spec

**Author**: Robbie (Tech Lead)
**Linear**: [DUN-14](https://linear.app/dungeon-crawler-game/issue/DUN-14)
**Status**: DECISIONS LOCKED. This document is the spec; implementation can begin.

---

## Summary

Parry is core combat. Player FSM-based, frame-based at 60fps fixed tick, hybrid melee/projectile detection. Parried projectiles deflect in player's facing direction. Perfect parry tier exists with 3-frame window — same defensive behavior, upgraded reward: melee perfect parry stuns enemy with a reusable star-burst visual, projectile perfect parry returns the projectile to the original sender (not facing-direction). Movement locked during parry. v1 feedback ships hit-stop, blue sparks, ring audio, missed-parry visual, parry-ready UI indicator. **Total estimated cost: ~9.5 days.**

---

## Locked Decisions

### Timing
- **Frame-based at 60fps fixed simulation tick.** Render rate may vary.
- **Parry window**: 8 frames (tune in slice).
- **Perfect parry window**: first 3 frames of the parry window. Adjust based on playtest feel — easy fix in tuning.
- **Parry recovery on miss**: 12 frames.
- **Parry recovery on hit**: 5 frames.
- **Enemy stun on normal parry**: 24 frames (counter window).
- **Enemy stun on perfect parry**: enemy enters a *stunned state* with a longer recovery — extended counter window plus the star-burst visual telegraphs to the player that they've earned the punish.

### State Management
- **PlayerState FSM**. States: `Idle`, `Moving`, `Attacking`, `Parrying`, `ParryRecovery`, `ParrySuccess`, `Hurt`.
- Movement is **locked** during `Parrying`.

### What Can Be Parried
- **Everything except attacks tagged unparryable.**
- Tagging via `AttackType` enum: `LightMelee`, `HeavyMelee`, `Projectile`, `Unparryable`.
- Melee parry: hitbox-overlap detection.
- Projectile parry: contact-during-window detection (separate system sharing infrastructure).

### Boss Parry Rules
- **Parry mechanics are identical against bosses.** No tightened window, no stagger gauge, no boss-unique parry system.
- **Bosses are differentiated by attack composition**: a much higher proportion of unparryable (red) attacks per phase, with phase 2 / phase 3 introducing new unparryable patterns.
- This means difficulty scales via *content design*, not *system override*. Reinforces consistent muscle memory across the game.

### Unparryable Attack Standard
- **Red flash on the attacker during wind-up.** Non-negotiable, universal.
- If an attack is unparryable, it's red. If it's red, it's unparryable.

### Cost / Resource
- **No stamina or focus cost.**
- Implicit cost: recovery frames on miss (12 frames) and successful hit (5 frames).
- "Parry everything" mindset preserved.

### Parry Effects

**Normal Parry — Melee:**
- Brief opening on the enemy (24-frame stun window).
- Player can attack or reposition during the window.

**Normal Parry — Projectile:**
- Projectile deflects in the **direction the player is facing**.
- No aim input; facing is set by movement.

**Perfect Parry — Melee:**
- Enemy enters stunned state with extended recovery window.
- **Reusable star-burst visual** appears above the enemy's head. Asset is sized up/down per enemy and boss tier (small for grunts, large for bosses).
- Distinct audio sting separate from the normal parry ring.

**Perfect Parry — Projectile:**
- Projectile is sent **straight back to the original sender** (owner reference is already tracked in the projectile struct).
- Distinct audio sting.

### Feedback

| Element | v1 (Slice) | Cost | Owner |
| --- | --- | --- | --- |
| Hit-stop on successful parry (50-80ms) | ✅ | 0 days (Tier 1 game feel) | Tech Lead |
| Ring audio on normal parry success | ✅ | 0.5 days | Art & Audio Director (Ryker) |
| Distinct sting on perfect parry success | ✅ | 0.25 days | Art & Audio Director |
| Blue sparks on successful parry | ✅ | 0.5 days | Art & Audio Director |
| Visual indicator on missed parry (whiff effect) | ✅ | 0.25 days | Art & Audio Director |
| Brief white flash on parried entity | ✅ | 0.25 days (reuses hurt flash) | Tech Lead |
| Star-burst stun asset (reusable, scalable) | ✅ | 0.5 days | Art & Audio Director |
| Parry-ready UI indicator (visible state of parry availability) | ✅ | 0.5 days | Art & Audio Director + Tech Lead |
| Screen shake on parry | ❌ Deferred | n/a | — |
| Time-slow on perfect parry | ❌ Cut | n/a | — |
| Auto-counter attack | ❌ Cut | n/a | — |

**Note on parry-ready UI indicator:** small visible cue on or near the player that signals whether parry is available (ready) or in cooldown (dim/desaturated during recovery frames). Teaches cooldown to new players without tooltips. Exact form is the Art & Audio Director's call.

**Ownership note:** Per DUN-6, audio is the Art & Audio Director's domain. SFX selection for parry (ring, perfect-parry sting, whiff) is Ryker's call with input from Nick on the *feel* the sounds need to convey.

### Scope Cuts (Explicit)
- No directional parry. Single button.
- No parry combo / chain bonuses.
- No upgradeable parry (longer window, bigger stun via items).
- No environmental parry (spike traps, falling debris).
- No time-slow on perfect parry.
- No auto-counter after parry.
- No stagger gauge for bosses.
- No tightened parry window for bosses.

---

## Reference Implementation Sketch

```cpp
// Player.h
enum class PlayerState {
    Idle,
    Moving,
    Attacking,
    Parrying,        // parry window active
    ParryRecovery,   // post-miss cooldown
    ParrySuccess,    // post-hit recovery (much shorter)
    Hurt,
};

struct ParryConfig {
    int windowFrames           = 8;
    int perfectWindowFrames    = 3;   // first N frames of the window
    int recoveryOnMissFrames   = 12;
    int recoveryOnHitFrames    = 5;
    int enemyStunNormalFrames  = 24;
    int enemyStunPerfectFrames = 40;  // extended for perfect-parry punish
};

// Attack.h
enum class AttackType {
    LightMelee,    // parryable
    HeavyMelee,    // parryable
    Projectile,    // parryable (handled in projectile system)
    Unparryable,   // RED — must be dodged
};

struct Attack {
    AttackType  type;
    Rectangle   hitbox;
    int         damage;
    int         activeFrames;
    Entity*     owner;
};

// Parry check (pseudocode)
ParryResult tryParry(Player& player, std::vector<Attack>& activeAttacks) {
    if (player.state != PlayerState::Parrying) return ParryResult::None;

    int framesIntoWindow = player.framesInState;
    bool isPerfectWindow = framesIntoWindow < ParryConfig::perfectWindowFrames;

    for (auto& attack : activeAttacks) {
        if (attack.type == AttackType::Unparryable) continue;
        if (!parryZoneOverlaps(player, attack)) continue;
        if (attack.framesActive > attack.activeFrames) continue;

        if (isPerfectWindow) {
            triggerPerfectParry(player, attack);
            return ParryResult::Perfect;
        } else {
            triggerNormalParry(player, attack);
            return ParryResult::Normal;
        }
    }
    return ParryResult::None;
}

// Projectile parry — separate path
void onProjectileContactDuringParryWindow(Player& player, Projectile& proj) {
    int framesIntoWindow = player.framesInState;
    bool isPerfectWindow = framesIntoWindow < ParryConfig::perfectWindowFrames;

    if (isPerfectWindow) {
        // Return to sender
        proj.velocity = directionTo(proj.owner) * proj.speed;
        proj.owner = &player; // friendly fire flip
        triggerPerfectParryFX(player, proj.position);
    } else {
        // Deflect in player facing direction
        proj.velocity = player.facing * proj.speed;
        proj.owner = &player;
        triggerNormalParryFX(player, proj.position);
    }
}
```

---

## Implementation Cost Summary

| Component | Days |
| --- | --- |
| Player FSM refactor | 1.0 |
| Parry state + timing windows (normal + perfect tiers) | 0.75 |
| Melee parry detection (hitbox overlap + frame tagging) | 2.0 |
| Projectile parry handling (in projectile system, normal + perfect behaviors) | 2.0 |
| Enemy attack tagging (`AttackType` enum + per-attack data) | 0.5 |
| Red-attack visual convention (shader flash or sprite swap) | 0.5 |
| v1 feedback bundle: hit-stop, ring audio hookup, sparks, whiff, parried-entity flash | 1.0 |
| Star-burst stun asset + scaling system | 0.5 |
| Parry-ready UI indicator | 0.5 |
| Tuning pass during slice | 1.0 |
| **Total** | **~9.5 days** |

**Calendar time**: ~2 weeks for one person focused; ~1 week for two parallelizing (one on player FSM + melee, one on projectile + feedback).

---

## Implementation Order

Recommended sequence so each piece unblocks the next:

1. **Player FSM refactor** (1 day) — foundation for everything else.
2. **Parry state + timing** (0.75 days) — gets parry "working" with placeholder feedback.
3. **`AttackType` enum + attack tagging** (0.5 days) — required before melee detection has anything to test against.
4. **Melee parry detection** (2 days) — first complete parry loop.
5. **Red-attack visual** (0.5 days) — communicates unparryable to the player.
6. **v1 feedback bundle** (1 day) — parry now *feels* like parry.
7. **Star-burst + parry-ready UI** (1 day) — polish the perfect-parry payoff.
8. **Projectile parry handling** (2 days) — extend system to projectiles.
9. **Tuning** (1 day) — final pass during slice playtest.

---

## Dependencies Unblocked

- **DUN-15** (enemy stat sheet — needs parryable tagging on each enemy attack)
- **DUN-16** (encounter composition — parry-counter dynamics inform spawn rules)
- **DUN-18** (telegraphing standards — red-attack convention now spec'd)
- **DUN-11** (audio — parry SFX inventory is now defined: ring, perfect sting, whiff, sparks)
- **DUN-9** (boss design — per-boss attack tagging follows the standard above)
