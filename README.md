# dungeon-crawler

A C++20 roguelike built on Raylib.

For design and architecture, see [`docs/`](docs/) — start with [`docs/DESIGN.md`](docs/DESIGN.md) and [`docs/TECH.md`](docs/TECH.md).

---

## Prerequisites

Supported dev platforms are **macOS** and **Windows**. Both are exercised in CI.

| Tool | Min version | Notes |
| --- | --- | --- |
| C++ compiler | Apple Clang 15 / MSVC 19.3+ | Must support C++20 |
| CMake | 3.25 | Needed for `CMakePresets.json` v6 |
| Ninja | any recent | Generator used by all presets |
| Git + Git LFS | any recent | Binary assets will be LFS-tracked |
| clang-format | 17+ | Style enforcement |
| clang-tidy | 17+ | Lint (run locally; CI lint is format-only today) |
| clangd | 17+ | Editor diagnostics |

Raylib and doctest are fetched automatically by CMake — no system install needed.

### macOS

```sh
brew install cmake ninja llvm git-lfs
# llvm provides clang-format, clang-tidy, and clangd. Put them on PATH:
echo 'export PATH="$(brew --prefix llvm)/bin:$PATH"' >> ~/.zshrc
```

### Windows

Install via [winget](https://learn.microsoft.com/en-us/windows/package-manager/winget/) from an admin PowerShell:

```powershell
# MSVC toolchain, Windows SDK, CMake, and Ninja all come with VS 2022.
# "Community" is free for individuals; "BuildTools" is headless.
winget install --id Microsoft.VisualStudio.2022.Community  `
  --override "--add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"

winget install --id LLVM.LLVM       # clang-format, clang-tidy, clangd
winget install --id Git.Git         # bundles Git for Windows + Git LFS
```

After install:

- Open the **x64 Native Tools Command Prompt for VS 2022** (or run `vcvarsall.bat x64` from any shell) before running CMake — that's how the MSVC toolchain ends up on `PATH`. If you're driving everything from VS Code, the **CMake Tools** extension picks the kit for you instead.
- Confirm `clang-format --version`, `clang-tidy --version`, and `clangd --version` resolve. The LLVM installer offers a "Add LLVM to the system PATH" checkbox that is **off by default** — if you skipped it, see [Putting tools on PATH](#putting-tools-on-path) below.

Raylib needs no extra setup on Windows — CMake links the bundled OS libraries automatically.

### Putting tools on PATH

Installers don't always add their binaries to `PATH`. If a command "isn't recognized" / "command not found" even though you installed it, the tool is on disk but the shell can't find it. First confirm what's missing — every tool should print a version:

```sh
cmake --version
ninja --version
git --version
clang-format --version   # from llvm
clang-tidy --version     # from llvm
clangd --version         # from llvm
```

Fix whichever ones fail.

#### macOS (zsh)

Homebrew puts `cmake`, `ninja`, and `git-lfs` on `PATH` automatically **as long as Homebrew itself is wired into your shell**. If even `cmake` isn't found, your shell never sourced `brew shellenv`:

```sh
# Apple Silicon installs to /opt/homebrew; Intel to /usr/local — this picks the right one.
echo 'eval "$('"$(command -v brew || echo /opt/homebrew/bin/brew)"' shellenv)"' >> ~/.zshrc
exec zsh   # reload the shell
```

`llvm` is the exception. Homebrew installs it **keg-only** — it deliberately does *not* symlink `clang-format`/`clang-tidy`/`clangd` onto `PATH`, to avoid shadowing Apple's toolchain. That's why it needs its own line (also shown in the macOS install block above):

```sh
echo 'export PATH="$(brew --prefix llvm)/bin:$PATH"' >> ~/.zshrc
exec zsh
```

Order matters: prepend (`...:$PATH`) so the Homebrew LLVM tools win over any older system copies. `git` ships with the Xcode Command Line Tools, so it's normally already found.

#### Windows

Each installer has its own PATH behavior, and "comes with VS 2022" tools only resolve inside the Native Tools prompt:

| Tool | Default installer location | On PATH automatically? |
| --- | --- | --- |
| Git | `C:\Program Files\Git\cmd` | Yes — the installer's default option adds it |
| LLVM | `C:\Program Files\LLVM\bin` | **No** — the PATH checkbox is off by default |
| CMake | `C:\Program Files\CMake\bin` | Only if you chose "Add CMake to the system PATH"; the VS-bundled copy is on PATH only inside the Native Tools prompt |
| Ninja | wherever you unzipped it (single `.exe`) | **No** — standalone Ninja has no installer; the VS-bundled copy is on PATH only inside the Native Tools prompt |

**Permanent fix (recommended) — System Settings GUI:**

1. Press Win, search **"Edit the system environment variables"**, open it.
2. Click **Environment Variables…**
3. Under **User variables**, select **Path** → **Edit** → **New**, and add the bin directory for each missing tool (e.g. `C:\Program Files\LLVM\bin`).
4. **OK** out of all dialogs, then **open a new terminal** — existing terminals keep the old `PATH`.

**Current-session-only fix (PowerShell)** — handy to test before committing to the change:

```powershell
$env:Path += ";C:\Program Files\LLVM\bin"
```

Avoid `setx PATH ...` from the command line: it truncates long values at 1024 chars and can clobber your existing `PATH`. Use the GUI for permanent changes.

After editing `PATH`, re-run the version checks above in a **fresh** terminal.

---

## First-time setup

### 1. Install Git LFS on your machine (one-time, per machine)

```sh
git lfs install
```

This adds Git LFS hooks to your global git config. You only need to run it once per machine, not per repo. Skipping it means a clone fetches **pointer files** instead of the real binaries, and the game won't have its assets.

### 2. Clone

```sh
git clone <repo-url> dungeon-crawler
cd dungeon-crawler
```

If you cloned before installing LFS, fix it with:

```sh
git lfs install
git lfs pull          # download the real blobs for the current checkout
```

### 3. Git LFS — what it is and how to use it

LFS replaces large binaries in git history with tiny text pointers; the real bytes live on a separate LFS store and are fetched on checkout. We use it for art and audio source files so the repo stays cheap to clone.

`.gitattributes` tracks `*.ase`, `*.aseprite`, `*.wav`, `*.ogg`, `*.mp3`, and `*.png` as LFS. As long as you ran `git lfs install` once on your machine (step 1 above), clones and checkouts will fetch the real binaries automatically.

**Day-to-day commands:**

| Task | Command |
| --- | --- |
| Track a new file type | `git lfs track "*.ase"` then `git add .gitattributes` and commit |
| See what's tracked in this repo | `cat .gitattributes` |
| See which files in HEAD are LFS pointers | `git lfs ls-files` |
| See LFS state for the working tree | `git lfs status` |
| Force-download blobs after a clone or branch switch | `git lfs pull` |
| Migrate already-committed binaries into LFS | `git lfs migrate import --include="*.png"` (rewrites history — coordinate first) |

**Per [`docs/TECH.md`](docs/TECH.md), the planned LFS types are:** `*.ase`, `*.aseprite`, `*.wav`, `*.ogg`, `*.mp3`, and `*.png` as a safety net. Don't add new binary types to git ad-hoc — track them in `.gitattributes` first, in the same commit that introduces the file.

**Common gotchas:**
- **Forgot `git lfs install` before cloning** → your binaries are 130-byte text files. Run `git lfs install` then `git lfs pull`.
- **`git lfs track` without committing `.gitattributes`** → only your machine knows the file should be LFS; teammates will commit it as a plain blob. Always stage and commit `.gitattributes` in the same change.
- **`.gitattributes` says LFS but the file already exists as a plain blob in history** → tracking only affects new commits. Use `git lfs migrate` to rewrite (history rewrite — needs team buy-in).

---

## Build, test, run

The project ships two presets, both single-config Ninja builds. They output to `build/debug/` and `build/release/`.

```sh
# Debug: -O0 -g, asserts on
cmake --preset debug
cmake --build build/debug
ctest --preset debug
./build/debug/dungeon-crawler

# Release: -O3, asserts off
cmake --preset release
cmake --build build/release
ctest --preset release
```

First configure downloads Raylib 6.0 and doctest via `FetchContent`; subsequent configures are fast.

---

## Editor setup — VS Code

`.vscode/` is gitignored, so each contributor wires this up locally.

### 1. Configure the debug preset once

`.clangd` points clangd at `build/debug/`, so you must run `cmake --preset debug` at least once before opening the project. clangd reads `build/debug/compile_commands.json` to mirror what compiles locally.

### 2. Install extensions

- **clangd** (`llvm-vs-code-extensions.vscode-clangd`) — diagnostics, completion, format, clang-tidy.
- Disable or uninstall **Microsoft C/C++** IntelliSense. The two providers fight over the same files; pick clangd.

Run `> clangd: Download language server` from the command palette if it doesn't find a system clangd.

### 3. Workspace settings

Create `.vscode/settings.json` with:

```json
{
  "editor.formatOnSave": true,
  "[cpp]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
  },
  "[c]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
  },
  "clangd.arguments": [
    "--clang-tidy",
    "--compile-commands-dir=build/debug",
    "--header-insertion=iwyu",
    "--background-index"
  ]
}
```

What this gives you:

- **Format on save** — clangd formats the file using the repo's `.clang-format`. No separate clang-format extension needed.
- **clang-tidy on the fly** — `--clang-tidy` makes clangd run the checks from `.clang-tidy` as you type. Findings appear as squiggles in the Problems panel. (Note: clangd shows them as warnings; CI promotes them to errors via `WarningsAsErrors: '*'`.)
- **Compile DB pinned to debug** — matches `.clangd` and ensures diagnostics use the right include paths and flags.

### 4. Verify

Open any `.cpp` file, save it, and confirm a re-indent / format pass runs. Introduce a deliberate lint violation (e.g., `int X = 0;` — wrong case for a local) and confirm the squiggle from `readability-identifier-naming`.

---

## Formatting and linting from the command line

CI and contributors without VS Code use the same configs:

```sh
# Format every tracked C/C++ file in place
git ls-files -z '*.cpp' '*.h' '*.hpp' | xargs -0 clang-format -i

# Check without modifying (this is what CI runs)
git ls-files -z '*.cpp' '*.h' '*.hpp' | xargs -0 clang-format --dry-run --Werror

# Lint a single file (uses build/debug's compile_commands.json)
clang-tidy src/dungeon/grid/tile_coord.cpp -p build/debug

# Lint the whole project
git ls-files '*.cpp' | xargs -I{} clang-tidy {} -p build/debug
```

Suppress a real false positive at the call site, never globally:

```cpp
// NOLINTNEXTLINE(check-name)  -- short reason
```

---

## Project layout

```
.
├── CMakeLists.txt         # top-level; fetches raylib, wires warnings, adds targets
├── CMakePresets.json      # debug / release presets
├── .clang-format          # formatter config
├── .clang-tidy            # linter config (strict)
├── .clangd                # editor-side compile-db pointer
├── src/
│   ├── main.cpp
│   └── dungeon/           # game code; built into dungeon_core static lib
├── tests/                 # doctest binary, discovered by CTest
├── docs/                  # design + tech docs, system specs
└── build/                 # gitignored CMake output
```

---

## CI

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) runs on every push to `main` and every PR:

- **Build Matrix** — configure / build / test on macOS + Windows, both presets.
- **Linter** — clang-format dry-run on Ubuntu. Must pass.

If a PR fails formatting, run the format command above and recommit.
