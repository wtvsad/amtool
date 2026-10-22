#!/bin/bash
set -e

REPO="https://raw.githubusercontent.com/wtvsad/amtool/main/nmcost.cpp"
DIR="$HOME/.local/bin"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

command -v clang++ >/dev/null || {
    echo "Install Apple's tools first: xcode-select --install"
    exit 1
}

curl -fsSL "$REPO" -o "$TMP/nmcost.cpp"
clang++ -std=c++17 -O2 "$TMP/nmcost.cpp" -o "$TMP/nmcost"

mkdir -p "$DIR"
install -m 755 "$TMP/nmcost" "$DIR/nmcost"

LINE='export PATH="$HOME/.local/bin:$PATH"'
touch "$HOME/.zshrc"
grep -Fqx "$LINE" "$HOME/.zshrc" || echo "$LINE" >> "$HOME/.zshrc"

echo "Installed! Run: nmcost"
echo "If needed, run: source ~/.zshrc"
