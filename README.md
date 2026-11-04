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

**`[i]` About**

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

## Troubleshooting

### Colours look wrong / icons show as `?`

Your terminal isn't a truecolor terminal. Try:

- **Windows:** use **Windows Terminal** (Microsoft Store) instead of the legacy console. AMTOOL auto-enables VT processing on Windows, but the legacy host still struggles with 24-bit colour.
- **macOS/Linux:** check `echo $TERM`. It should be `xterm-256color` or similar. If it's `dumb` or missing, set it:

  ```bash
  export TERM=xterm-256color
  ```

### The box-drawing characters (`╭ ╮ │ ╞`) render as garbage

Your terminal's font doesn't include them. Every modern monospace font does — *JetBrains Mono*, *Fira Code*, *Cascadia Code*, *Menlo*, *SF Mono*, *Hack* all work. On Windows Terminal the default **Cascadia Mono** is already fine.

### `g++: error: amtool.cpp: No such file or directory`

You're not in the repo directory. Run `cd amtool` first.

### `ld: unknown file type in 'amtool'`

You renamed the source file and lost the `.cpp` extension. The compiler decides the language from the extension; make sure the file is named `amtool.cpp`.

### `'std::chrono' file not found` or `'auto' cannot be deduced`

Your compiler is old or being invoked without `-std=c++17`. Pass `-std=c++17` to `g++`, or `cl /std:c++17` on MSVC.

### Menus don't react to arrow keys on macOS

Only happens if the source was modified to call `readKey()` outside a menu loop. The shipped code is correct — if you're editing it, make sure `toolAbout()` starts with `g_raw.enable();`.

### `Permission denied` when running `./amtool`

```bash
chmod +x amtool
```

### Windows: `'amtool' is not recognized as an internal or external command`

Either use `amtool.exe`, or run it with the full path, or add the folder to `PATH`.

---

## Building with CMake (optional)

If you'd rather use CMake:

```cmake
cmake_minimum_required(VERSION 3.15)
project(amtool LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(amtool amtool.cpp)
if (WIN32)
    target_compile_options(amtool PRIVATE /EHsc)
endif()
```

```bash
cmake -S . -B build
cmake --build build --config Release
```

Binary lands in `build/` (or `build/Release/` on multi-config generators).

---

## Project layout

```
amtool/
├── amtool.cpp      ← the entire application, single file
├── README.md       ← you are here
└── LICENSE         ← (MIT — see below)
```

AMTOOL is intentionally a single-file program. If you want to contribute, everything you'd want to touch — palette, animation, menu system, tools, presets — lives in one file with clear section headers.

---

## Contributing

1. Fork the repo.
2. Create a branch: `git checkout -b feature/my-thing`.
3. Edit `amtool.cpp`.
4. Verify on your platform: `g++ -std=c++17 -O2 -Wall -Wextra amtool.cpp -o amtool && ./amtool`.
5. Open a PR with a short description + a screenshot if the UI changed.

Small asks:

- **Keep it single-file.** No libraries, no build system beyond the one-liner.
- **Stay C++17.** Nothing newer without discussion.
- **Keep colours themable.** Add to `THEME_*` rather than hardcoding RGB.
- **Match the geometry.** Use `INDENT` and `BAR_W` — never hardcode `3` or `58`.
- **Test on at least two of: macOS / Linux / Windows.**

---
