# AMTOOL

> A terminal calculator for the **Adopt Me!** trading community.
> Cost out pet stacks, estimate grind time, and work out box-drop probability —
> all without leaving your terminal.

```
░█████╗░███╗░░░███╗████████╗░█████╗░░█████╗░██╗░░░░░
██╔══██╗████╗░████║╚══██╔══╝██╔══██╗██╔══██╗██║░░░░░
███████║██╔████╔██║░░░██║░░░██║░░██║██║░░██║██║░░░░░
██╔══██║██║╚██╔╝██║░░░██║░░░██║░░██║██║░░██║██║░░░░░
██║░░██║██║░╚═╝░██║░░░██║░░░╚█████╔╝╚█████╔╝███████╗
╚═╝░░╚═╝╚═╝░░░░░╚═╝░░░╚═╝░░░░╚════╝░░╚════╝░╚══════╝
```

---

## Features

- **Cost calculator** — add up prices of any number of pets, with NEON (×4) and MEGA (×16) bundle math, plus custom normal / NEON / MEGA counts.
- **Time calculator** — estimate how long it takes to reach a currency goal, using either a preset event rate or your own measured rate (either typed in, or derived from a past play session).
- **Drop probability** — work out how many boxes you need on average to obtain a given drop at a given % chance, and optionally what that would cost.

---

## Requirements

You need a **C++17 compiler** and a **truecolor-capable terminal**.

| Platform | Compiler | Terminal |
|----------|----------|----------|
| **Windows** | [MSYS2](https://www.msys2.org/) `g++` *or* [Visual Studio](https://visualstudio.microsoft.com/) with "Desktop development with C++" | **Windows Terminal** (recommended) or the classic Console — both support ANSI + truecolor on Windows 10 1511+ |
| **macOS** | Xcode Command Line Tools (`xcode-select --install`) | Terminal.app or iTerm2 |
| **Linux** | `g++` from your package manager | Any truecolor terminal (GNOME Terminal, Konsole, Alacritty, kitty, …) |

To check your compiler:

```bash
g++ --version
```

If it prints `g++ (…) 7.0` or higher, you're good.

---

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/wtvsad/amtool.git
cd amtool
```

### 2. Compile

**Linux / macOS:**

```bash
g++ -std=c++17 -O2 amtool.cpp -o amtool
```

**Windows (MSYS2 / MinGW):**

```bash
g++ -std=c++17 -O2 amtool.cpp -o amtool.exe
```

**Windows (Visual Studio `cl`):**

```cmd
cl /std:c++17 /EHsc /O2 amtool.cpp /Fe:amtool.exe
```

### 3. Run

**Linux / macOS:**

```bash
./amtool
```

**Windows:**

```cmd
amtool.exe
```

That's it — no config files, no installers, no external libraries.

---

## Usage

The app is fully **arrow-key driven**. You never need to type a menu letter unless you want to (single-letter shortcuts work too).

| Key | Action |
|-----|--------|
| `↑` `↓` | Move selection |
| `Enter` | Confirm / submit |
| `Esc` | Go back one level · from the main menu, quit the app |
| Type + `Enter` | Enter a value at a prompt |
| Blank + `Enter` | Skip / use default (each prompt tells you the default) |
| `Esc` at a prompt | Cancel the current tool, return to its menu |

### Sections

**`[a]` Cost calculator**
Two modes:

- **flexible** — add any number of pets, one cost × quantity each, then see a grand total.
- **NEON / MEGA** — enter a unit cost and see the reference table for 1 pet, 1 NEON (×4), 1 MEGA (×16), a NEON + MEGA pair, and all three at once. Then enter how many normals / NEONs / MEGAs of that pet you want, optionally repeat for more pets, and see a **grand total for 1 of each pet** plus a **custom total** for what you actually entered.

**`[b]` Time calculator**

- **quick** — uses a preset event rate that the tool maintainer updates per event (event name + last-updated date shown).
- **custom** — either type your rate directly, or enter how long you played and how much you earned, and the tool derives the rate for you.

You then enter your current and target amounts, and it shows **remaining**, **rate**, and **estimated time** as `Xd Yh Zm`.

**`[c]` Drop probability**

Enter the drop % per box, how many of that drop you want (default 1), and optionally the cost per box. You get:

- **estimated boxes** (expected value: `quantity ÷ p`)
- **total cost** (if you gave a box cost)

**`[i]` About dev**

Project info and links to the creator's other work. Arrow keys select, `Enter` opens a link in your default browser.

---

## Updating the preset event rate

If you're the maintainer and want the **quick time calculator** to reflect the current Adopt Me! event, edit the `PRESET_RATE` block at the top of `amtool.cpp`:

```cpp
const PresetRate PRESET_RATE = {
    "Summer Festival 2025",   // event name
    "Candy",                  // currency name
    "2025-07-01",             // last updated (yyyy-mm-dd)
    1500.0                    // amount earned per hour
};
```

Recompile, done. Everyone who pulls the new binary sees the new preset.

> **Optional (not implemented yet):** if you'd rather update the rate **without recompiling**, replace `PRESET_RATE` with a file loader (`std::ifstream` reading a small `preset.txt`). The About section in the code explains where to hook it in.

---

## For maintainers — shipping a new version

When you change `amtool.cpp`, follow these steps in order.

### 1. Bump the version string

Search the source for the current version (e.g. `v2.15`) and update **both** occurrences:

- **About screen header** — inside `toolAbout()`, find:
  ```cpp
  borderTop("wtvsad@amtool · v2.15");
  ```
  change to the new version.

- **Anywhere else** — sweep the file:
  ```bash
  grep -n "v2\." amtool.cpp
  ```
  Bump every hit.

### 2. Rebuild and smoke-test

Before pushing, at minimum compile cleanly with warnings on, and run through every menu once:

```bash
g++ -std=c++17 -O2 -Wall -Wextra amtool.cpp -o amtool
./amtool
```

Verify while running:

- main menu → each tool → back to main → each tool again
- About: `↑↓` moves highlight, `Enter` opens browser, `Esc` returns
- prompts: `Enter` alone skips, `Esc` cancels, backspace works
- width guard fires cleanly at ≤75 cols (resize your terminal narrow)

If you have access to macOS or Windows, build there too:

```bash
# macOS
clang++ -std=c++17 -O2 -Wall -Wextra amtool.cpp -o amtool

# Windows (MSYS2 / MinGW)
g++ -std=c++17 -O2 -Wall -Wextra amtool.cpp -o amtool.exe
```

### 3. Commit with a version tag

Use a commit body that describes what changed, then tag it:

```bash
git add amtool.cpp README.md
git commit -m "$(cat <<'EOF'
v2.16: fit in 80 columns, honest error on narrow terminals

- INDENT 3→2, BAR_W 58→56, About INNER 74→70
- width guard prints clear message instead of garbling under 76 cols
- About: shortened app blurb, whoami bio, dropped topic list
- About menu label now "About dev"
- fixed RGB/string bug in width guard (RED_C → fg(RED_C))
EOF
)"
git tag -a v2.16 -m "v2.16"
git push origin main --tags
```

Tag naming: `vMAJOR.MINOR` (e.g. `v2.16`). Bump **minor** for anything user-visible. Skip a version bump entirely for pure refactors / comment cleanup that users won't notice.

### 4. Publish a GitHub Release

Releases are what most users download. On GitHub:

1. **Releases → Draft a new release**
2. **Choose a tag** → the `v2.16` tag you just pushed
3. **Title** → `v2.16`
4. **Description** → one or two lines describing what changed
5. **Attach binaries** built on each OS (see below)
6. **Publish release**

**Building release binaries** — from a clean checkout of the tagged commit:

```bash
# Linux
g++ -std=c++17 -O2 -s amtool.cpp -o amtool-linux-x64

# macOS (Apple Silicon)
clang++ -std=c++17 -O2 -s amtool.cpp -o amtool-macos-arm64

# macOS (Intel)
clang++ -std=c++17 -O2 -s -arch x86_64 amtool.cpp -o amtool-macos-x64

# Windows (MSYS2)
g++ -std=c++17 -O2 -s amtool.cpp -o amtool-windows-x64.exe
```

`-s` strips debug symbols, roughly halving binary size. Attach all four to the release so users on any platform can `curl` + `chmod +x` + run, no compiler needed.

### 5. Update the README version reference (optional)

The README doesn't hardcode a version — keep it that way. Only touch the README if the release changes *how users install or run* the tool (new flag, new dependency, renamed binary).

---

## What counts as a version bump

| Change | Bump | Example |
|---|---|---|
| New tool, menu option, or screen | **minor** | `v2.15 → v2.16` |
| New theme, animation, or layout change | **minor** | `v2.16 → v2.17` |
| Bug fix that changes behaviour | **minor** | `v2.17 → v2.18` |
| `PRESET_RATE` update only (event / date / rate) | **minor** | `v2.18 → v2.19` |
| Pure refactor, comment removal, no user-visible effect | **skip** version bump, no release | — |

Rule of thumb: **if a user would notice, bump.** If not, don't.

---

## Rolling back

If a release is broken:

```bash
git revert HEAD
git commit -m "v2.16.1: revert broken v2.16"
git tag -a v2.16.1 -m "v2.16.1"
git push origin main --tags
```

Never re-tag a published tag. Users may have already downloaded `v2.16`, and release assets are pinned to it. Ship `v2.16.1` instead.

---

## Troubleshooting

### Colours look wrong / icons show as `?`

Your terminal isn't truecolor. Try:

- **Windows:** use **Windows Terminal** instead of the legacy console. AMTOOL auto-enables VT processing on Windows, but the legacy host struggles with 24-bit colour.
- **macOS/Linux:** check `echo $TERM` — it should be `xterm-256color` or similar. If it's `dumb` or missing:
  ```bash
  export TERM=xterm-256color
  ```

### The box-drawing characters render as garbage

Your terminal's font doesn't include them. Every modern monospace font does — *JetBrains Mono*, *Fira Code*, *Cascadia Code*, *Menlo*, *SF Mono*, *Hack*. On Windows Terminal the default Cascadia Mono is fine.

### Screen keeps repeating the same block over and over

Your terminal is narrower than 76 columns. Widen it (drag the window edge), or set the window to at least 80×24. The tool prints a clear error instead of the old behaviour (garbled redraws) once it detects a narrow terminal — if you're still seeing the loop, you're running an older binary. Rebuild.

### `g++: error: amtool.cpp: No such file or directory`

You're not in the repo directory. `cd amtool` first.

### `ld: unknown file type in 'amtool'`

You renamed the source file and lost the `.cpp` extension. The compiler decides language by extension — the file must end in `.cpp`.

### `'std::chrono' file not found` / `'auto' cannot be deduced`

Your compiler is old, or you're not passing `-std=c++17`. Add it: `g++ -std=c++17 …`.

### `Permission denied` when running `./amtool`

```bash
chmod +x amtool
```

### Windows: `'amtool' is not recognized as an internal or external command`

Use `amtool.exe`, or run with the full path, or add the folder to `PATH`.

---

## License

MIT — see [`LICENSE`](./LICENSE).

---

## Links

- **YouTube:** [youtube.com/@wtvsad](https://www.youtube.com/@wtvsad)
- **X / Twitter:** [x.com/wtvsad](https://x.com/wtvsad)
- **ArtStation:** [artstation.com/wtvsad](https://www.artstation.com/wtvsad)
- **GitHub:** [github.com/wtvsad](https://github.com/wtvsad)
- **Patreon:** [patreon.com/wtvsad](https://patreon.com/wtvsad)

> *"Maybe SAD stands for SomeAmazingDude, who knows."*
> *(actually, first 3 letters of my legal last name)*

---

**Made by [@wtvsad](https://github.com/wtvsad)**
