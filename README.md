# Adopt Me! Neon & Mega Cost Calculator

A simple C++ terminal calculator for calculating pet costs in Adopt Me!

## Installation

### macOS

Open Terminal and run:

```bash
curl -fsSL https://raw.githubusercontent.com/wtvsad/amtool/main/install.sh -o /tmp/nmcost-install.sh && bash /tmp/nmcost-install.sh
```

Then launch the calculator:

```bash
nmcost
```

### Windows

Open PowerShell and run:

```powershell
irm https://raw.githubusercontent.com/wtvsad/amtool/main/install.ps1 -OutFile "$env:TEMP\nmcost-install.ps1"
& "$env:TEMP\nmcost-install.ps1"
```

Then launch the calculator:

```powershell
nmcost
```

Windows requires a compatible C++ compiler to be installed.

## Cost Multipliers

| Pet type | Multiplier |
|---|---:|
| Regular | 1× |
| Neon | 4× |
| Mega Neon | 16× |
| Neon + Mega Neon | 20× |
| Regular + Neon + Mega Neon | 21× |

## Requirements

- macOS: Xcode Command Line Tools or another compatible C++ compiler.
- Windows: A compatible C++ compiler, such as LLVM/Clang or MinGW-w64.

## Source Code

- [Calculator](https://github.com/wtvsad/amtool/blob/main/nmcost.cpp)
- [Repository](https://github.com/wtvsad/amtool)

