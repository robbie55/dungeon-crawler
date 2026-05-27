# TECH.md

Technical foundation for the game. This is the source of truth for stack, architecture, and build decisions. If you disagree with something here, raise it with the Technical Lead — don't quietly deviate.

For design (mechanics, levels, enemies, bosses, art/audio), see `DESIGN.md`. For deep-dive implementation specs on individual systems, see `specs/`.

---

## Stack

| Area | Choice | Notes |
| --- | --- | --- |
| Language | **C++20** | Set in `CMakeLists.txt` and pinned in `.clang-format` / `.clang-tidy`. |
| Build system | CMake | Single top-level `CMakeLists.txt`. |
| Graphics / input / audio | Raylib 6.0 | Pinned to a specific release tag — not `main`. |
| JSON | `nlohmann/json` | Save files and any future config. |
| Unit testing | doctest | Header-only, drop-in. Tests live in `tests/`. |
| Logging | Custom, in-house | Lightweight. See "Logging" below. |
| Asset hot-reload | Custom (Raylib file-watch + reload) | No paid tooling. |
| Code hot-reload | None | Restart-on-change. Out of scope for v1. |
| Platform target | Windows only | Single target. Re-evaluate post-launch. |
| Formatting | clang-format | Google base, see `.clang-format`. CI-enforced. |
| Linting | clang-tidy (strict) | See `.clang-tidy`. CI-enforced; warnings = errors. |
| Editor integration | clangd | See `.clangd`. |

---

## Architecture

### Entity Model

**Simple OOP.** Concrete classes for `Player`, `Enemy`, `Projectile`, `Boss`, etc. No ECS — overkill for a 3-month project with a known, small entity count and a tarpit if anyone is learning it for the first time.

Default to **composition over inheritance** where it's natural (e.g., `Entity` has a `Health` component, a `Hitbox` component) but don't build a framework for the sake of it. If you find yourself writing base-class machinery in week 2, stop and ask why.

### Scene / State Management

**Single FSM with a state stack.** One state machine governs all game states. States push and pop. The current state is always the top of the stack.

States to start with:
- `MainMenu`
- `Hub` (central island hub where the player selects unlocked levels)
- `Level` (one instance per active level, holds rooms / enemies / player)
- `Pause` (pushed on top of `Level`, doesn't destroy it)
- `GameOver`
- `Victory`

Rules:
- Only the top state ticks and renders by default. If a state wants the state beneath it to render (e.g., `Pause` showing the frozen level behind a dim overlay), it explicitly asks for it.
- States own their resources. On pop, the state's destructor releases everything it allocated.
- Transitions go through the state machine — no direct state-to-state pointers.

### Player State Machine (Locked from `specs/parry.md`)

Distinct from the scene FSM. Governs player combat states:

`Idle`, `Moving`, `Attacking`, `Parrying`, `ParryRecovery`, `ParrySuccess`, `Hurt`.

- Movement is locked during `Parrying`.
- Transitions tracked in frame counts (60fps fixed simulation tick).
- All other combat state (stamina-like timers, attack cooldowns) live here when added.

See `specs/parry.md` for full timing and feedback details.

### Input Handling

**Thin wrapper around Raylib input.** Don't call `IsKeyPressed` directly from gameplay code. Wrap once, then everything else calls `Input::IsPressed(Action::Attack)` or similar.

Why: lets us rebind keys without grepping the codebase, and lets us add a debug-only "input replay" or "input override" later if we want.

Keep the wrapper boring — no event queue, no observer pattern. A static-ish module that translates raw Raylib state into named actions per frame.

### Asset Loading

**Per-level loading.** Load on `Level::Enter`, free on `Level::Exit`. With three levels and modest asset counts, this keeps memory low and load times trivial.

A small **shared core asset pool** (player sprites, UI, common SFX) loads at startup and persists across levels.

Don't preload everything at startup "to be safe" — it'll bite when art doubles in size mid-project.

### Projectile System

Four decoupled components:

1. **Spawner** — per-weapon. Each weapon has its own spawner that knows how to construct its projectiles (cutlass arc, flintlock shot). Enemies and modifiers can also spawn projectiles via their own spawners. Spawners are the only thing that calls into the pool to claim a projectile.
2. **Projectile** — the data + behavior struct. Position, velocity, lifetime, damage, owner, sprite. Includes a `speed_multiplier` field on day one, plus `owner` (preserved through portal traversal — see `specs/portals.md`), and an `AttackType` tag (`LightMelee`, `HeavyMelee`, `Projectile`, `Unparryable` — see parry spec).
3. **Collision** — handles impact: damage application, hit-stop trigger, knockback, hurt flash, particle spawn, projectile despawn.
4. **Pool** — fixed-size object pool. Allocated once at level load. If the pool is exhausted, the oldest active projectile is recycled (loud warning logged in debug builds — exhaustion means we under-budgeted).

#### Modifier integration

Room modifiers that affect projectiles do so by **mutating a multiplier**, not by special-casing per-room logic:

```cpp
struct Projectile {
    float speed_multiplier = 1.0f;  // per-projectile
    // ... position, velocity, owner, attack_type, etc.
};

struct Room {
    float projectile_speed_multiplier = 1.0f;  // room-wide modifier
    // ... other modifier fields
};
```

Effective speed per frame = `base_speed * projectile.speed_multiplier * room.projectile_speed_multiplier`.

This is the hook for any projectile-affecting modifier. **Do not skip this on day one** — retrofitting it is painful.

### Portal System (Locked from `specs/portals.md`)

- Portals are paired locations with **atomic snap teleport on center-point overlap**.
- **Anchor-based random placement**: designers place 5–6 anchor slots per room; at room load, the system picks an active pair. Seedable for debug reproducibility.
- **Binary direction per room**: horizontal (east/west) or vertical (north/south). Traversal is doorway-mirrored.
- **Per-entity 0.5s cooldown** after use.
- **Player and projectiles only** — enemies do not enter portals.
- **Boss-controlled portals** (Level 3 boss) are a separate system sharing the renderer.

See `specs/portals.md` for the full spec and implementation order.

### Simulation Tick

- **60 Hz fixed simulation tick.** All gameplay timing — parry windows, animation frames, AI cooldowns — is expressed in frames at 60fps.
- Render rate may vary; simulation does not.

### Coordinate System

- **World space, float positions.**
- **1 unit = 1 pixel at base resolution (384×216).**
- All gameplay code uses world units. Rendering scales up to the player's monitor as a separate concern.
- Don't mix int and float positions. Pick float, stay float.

---

## File Layout

```
/
├── CMakeLists.txt
├── README.md
├── .clang-format
├── .clang-tidy
├── .clangd
├── .gitattributes
├── .gitignore
├── docs/
│   ├── DESIGN.md         # Master design doc (mechanics, levels, enemies, bosses, art/audio)
│   ├── TECH.md           # This file
│   └── specs/
│       ├── parry.md      # Parry system implementation spec
│       └── portals.md    # Portals system implementation spec
├── src/
│   ├── main.cpp
│   ├── core/             # FSM, input, logger, math helpers
│   ├── render/           # camera, sprite batching, screen shake, post-fx
│   ├── entity/           # player, enemy, boss, projectile
│   ├── world/            # room, level, modifier
│   └── ui/               # menus, HUD
├── assets/
│   ├── aseprite/         # source .ase files (LFS-tracked)
│   └── audio/            # music, SFX (LFS-tracked)
├── build/                # cmake output, gitignored
├── vendor/               # raylib, nlohmann/json, doctest — submodules or pinned
└── tests/
```

PNGs are **not committed** — they're exported from `.ase` files at build time via Aseprite CLI invoked from CMake.

---

## Repo & Source Control

### Mono-repo
Code + Aseprite source files + audio all live in one repo.

### Git LFS
**Required for the repo.** Configure for: `*.ase`, `*.aseprite`, `*.wav`, `*.ogg`, `*.mp3`, and `*.png` as a safety net.

- Every contributor runs `git lfs install` once on their machine before cloning. Add to README setup.
- `.gitattributes` is the source of truth for what's LFS-tracked. Don't add binary types ad-hoc.
- `.gitignore` covers `build/`, IDE files, OS junk, and any generated PNGs.

### Branching
Short-lived feature branches off `main`. No direct pushes to `main`.

### PR Review Policy
- **PR required for every merge to `main`.** No exceptions.
- **Architectural changes**: Tech Lead review **mandatory and non-negotiable** during the early phase.
- Teammates earn broader review authority as they level up in the codebase.
- **Live-review sessions** scheduled periodically — Tech Lead walks teammates through their reviews for mentorship.

---

## Code Style & Linting

Configs are in the repo and CI-enforced. Don't fight them — adjust the config (with a PR + discussion) if a rule is genuinely wrong.

### `.clang-format`

- Base: **Google**.
- Standard: **C++20**.
- Indent: 2 spaces, no tabs.
- Column limit: 100.
- Pointer/reference alignment: left (`int* p`, not `int *p`).
- Namespace indentation: all (every namespace level indents).
- `AlignConsecutiveAssignments` and `AlignConsecutiveDeclarations` are **off** — they create churn in diffs when variables are added or renamed.

### `.clang-tidy`

Strict, opinionated, educational. The philosophy: a learning project benefits more from the linter yelling about a real C++ idiom than from silence about a pattern that "works."

**Enabled groups**: `bugprone-*`, `clang-analyzer-*`, `concurrency-*`, `cppcoreguidelines-*`, `misc-*`, `modernize-*`, `performance-*`, `portability-*`, `readability-*`.

**Notable disables** (with reasoning — see `.clang-tidy` for the full list):
- `*-magic-numbers` — too noisy in a game with lots of coordinate/timing constants.
- `bugprone-easily-swappable-parameters` — fires on every `f(int x, int y)`.
- `*-bounds-pointer-arithmetic` and `*-bounds-array-to-pointer-decay` — false-positive heavy without GSL.
- `misc-no-recursion` — recursion is a normal tool.
- `modernize-use-trailing-return-type` — stylistic, not consensus.
- `readability-identifier-length` — `for (int i = 0; ...)` is fine.

**WarningsAsErrors**: `*`. Any clang-tidy hit fails CI. Suppress with `// NOLINTNEXTLINE(check-name)  -- reason` at the call site rather than disabling globally.

### Naming Conventions

| Kind | Style | Example |
| --- | --- | --- |
| Types (class, struct, enum, alias) | `CamelCase` | `PlayerState` |
| Functions, methods | `CamelCase` | `TryParry()` |
| Variables, parameters | `lower_case` | `parry_window_frames` |
| Member variables (private/protected) | `lower_case_` (trailing underscore) | `health_` |
| Constants, constexpr | `kCamelCase` | `kParryWindowFrames` |
| Enum constants | `kCamelCase` | `AttackType::kUnparryable` |
| Namespaces | `lower_case` | `core::input` |
| Macros | `UPPER_CASE` | `DEBUG_ASSERT` |
| Template parameters | `CamelCase` | `template <typename T>` |

### Complexity Bounds (clang-tidy enforced)

| Metric | Threshold |
| --- | --- |
| Cognitive complexity per function | 20 |
| Lines per function | 80 |
| Statements per function | 50 |
| Branches per function | 10 |
| Parameters per function | 5 |
| Nesting depth | 4 |
| Variables per function | 15 |

Functions that bust these limits split. No exceptions without discussion.

### `.clangd`

Editor-side configuration. Points `clangd` at the CMake compilation database at `build/debug`. Ensures editor diagnostics match what CI sees.

---

## CI / Automation

- **Format + lint pipeline is live.** Every PR runs `clang-format --dry-run` and `clang-tidy`. Both must pass.
- **No automated build/test pipeline yet** — deferred until after vertical slice ships. Revisit week 5.
- Pre-commit hooks **not** required (CI catches it). Optional for individual contributors.

---

## Asset Pipeline

- Artists work in Aseprite. Source `.ase` files live in `assets/aseprite/` and are LFS-tracked.
- A CMake build step invokes the Aseprite CLI to export `.ase` → `.png` into a generated output directory (e.g., `build/assets/sprites/`).
- Generated PNGs are **never committed**.
- Every dev needs Aseprite installed and on `PATH`. Document this in the README's setup section.
- **Coordinate with the Art & Audio Director** on:
  - Folder structure inside `assets/aseprite/`
  - Naming conventions (`player_idle.ase`, `enemy_grunt_walk.ase`)
  - Sprite sheet layout vs. individual frames
  - Whether tagged animation frames are exported as one sheet or many

---

## Save & Persistence

- **Format**: JSON via `nlohmann/json`.
- **Location**: `%APPDATA%\Local\<GameName>\save.json` — exact `<GameName>` folder name set when the game is named.
- **Top-level versioning** — every save file has a `"version": 1` field as the first key.
- **Load behavior** — on load, check version. If unrecognized (newer than current build) or older with no migration path, prompt user before overwriting; silently start fresh in debug builds.
- **Atomicity** — write to `save.json.tmp` then rename, so a crash mid-write doesn't corrupt the existing save.
- **v1 save contents**: unlocked weapons (cutlass always; flintlock after L1 boss), unlocked levels, settings (volume, keybinds). Add room-checkpoint state if escape-hatch checkpointing is needed post-playtest.

---

## Logging

In-house, minimal. No third-party dependency.

- Severity levels: `TRACE`, `DEBUG`, `INFO`, `WARN`, `ERROR`.
- Compile-time minimum level — `DEBUG` in debug builds, `INFO` in release.
- Output to stdout + a rotating file in `%APPDATA%\Local\<GameName>\logs\`.
- One-line format: `[timestamp] [level] [file:line] message`.
- Keep it boring — no fancy structured logging, no async log threads. Grow it if we need to.

---

## Debug & Dev Tooling

Build all of this **in week 1**. It pays for itself every day after.

| Tool | Trigger | Effect |
| --- | --- | --- |
| God mode | Debug-only key (e.g., F1) | Player takes no damage |
| Super speed | Debug-only key (e.g., F2) | Player movement speed ×3 |
| Level skip | Debug-only key (e.g., F3 / F4) | Jump to next/previous level |
| Modifier preview | Debug-only key (e.g., F5) | Cycle through room modifiers in current room |
| FPS / frame time | Always on in debug, off in release | Top-left corner overlay |
| Hitbox visualization | Debug-only toggle | Draw entity + projectile hitboxes |
| Spawn-anywhere | Debug-only menu | Spawn any enemy at cursor |
| Portal seed display | Debug-only overlay | Show current portal-randomization seed for reproducibility |
| Parry frame counter | Debug-only overlay | Show parry-state frame timing for tuning |

All debug features compile out of release builds via `#ifdef DEBUG` or a `constexpr bool kDebug` flag. Release binary has zero debug overhead.

---

## Hot-Reload

**Asset hot-reload only.** No code hot-reload — restart on code change.

Implementation: a thin file-watcher (poll mtime on a background thread, or check on a timer in the main loop) over `build/assets/`. When a PNG changes, the renderer reloads the affected texture.

Scope:
- ✅ Sprite reload
- ✅ Audio reload (nice-to-have)
- ❌ Shader reload (only if/when we add Tier-3 post-processing)
- ❌ Code, JSON config, anything else

---

## Testing

doctest. Tests live in `tests/` and build into a separate binary.

What to test:
- **Pure logic**: math helpers, FSM transitions (player + scene), save serialization round-trips, modifier-multiplier arithmetic, parry-frame window logic, portal anchor pair selection.
- **Don't test**: rendering, input, anything requiring a window or audio device. Not worth the harness cost for a 3-month project.

Aim for "tests catch the dumb regressions," not coverage targets.

---

## Vertical Slice Definition

**Owner**: Tech Lead (Robbie).
**Target**: End of Week 3-4. Playable end-to-end:

- [ ] Player moves with cutlass attack (precision combat, no auto-attack)
- [ ] Parry system functional (8-frame window, 3-frame perfect tier, locked movement during parry)
- [ ] One enemy type that threatens the player (recommend Grunt — see DUN-15 for stats)
- [ ] One room with one modifier active (recommend Wreckage Cover or Water Pool)
- [ ] One miniboss with 2 phases (Phase 2 amplifies the modifier)
- [ ] Tier 1 game feel: hit-stop, screen shake, hurt flash, knockback
- [ ] v1 parry feedback: hit-stop, ring audio, blue sparks, parried-entity flash, star-burst stun, parry-ready UI
- [ ] Player FSM and scene FSM both working
- [ ] Programmer art (colored rectangles fine)
- [ ] Asset pipeline + hot-reload functional
- [ ] Save/load works for unlocks
- [ ] Debug tooling listed above is in place

If we hit Week 5 without a slice, **stop adding features and finish it.**

---

## System Specs

Living implementation specs for the systems with enough detail to warrant their own document. Update these as systems are built and tuned.

- **`specs/parry.md`** — full parry system spec (FSM states, timing constants, perfect-parry behavior, feedback bundle, scope cuts, ~9.5 days estimated).
- **`specs/portals.md`** — full portals system spec (anchor placement, traversal, edge cases, boss-controlled separation, ~13 days estimated).

Future specs likely needed:
- `specs/enemies.md` (when DUN-15 lands)
- `specs/encounters.md` (when DUN-16 lands)
- `specs/telegraphing.md` (when DUN-18 lands)

---

## Open / Deferred Decisions

| Item | Owner | Notes |
| --- | --- | --- |
| Per-weapon projectile pool max counts | Tech Lead | Tune during vertical slice. Initial guess: 64 cutlass swings, 256 flintlock shots, 256 enemy projectiles. |
| Aseprite sub-folder structure & naming | Art & Audio Director | Lock before any real art lands. |
| Save file game name (`<GameName>` folder) | Design Lead | Tied to when we name the game (deadline: end of Week 2). |
| Full CI build/test pipeline | Tech Lead | Format + lint live; build/test deferred until vertical slice ships. |
| Audio engine specifics (mixing, ducking) | Art & Audio Director + Tech Lead | Raylib audio is enough for v1; revisit if it isn't. |
| README VSCode integration docs | Tech Lead | Configs in repo and CI enforces; user-facing setup docs still pending. |
| `.clang-tidy` `HeaderFilterRegex` path | Tech Lead | Currently references `debug-plus/` from a previous project. Update when game has a name. |

---

## Mentor Note

The boring, unsexy stuff in this doc — asset pipeline, save versioning, debug tooling, format/lint configs — is what separates a project that ships from one that doesn't. **Build it in week 1, even though it's not fun.** Three months goes faster than you think.
