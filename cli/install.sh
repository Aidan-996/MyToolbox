#!/bin/sh
# MyBox CLI 一键安装器（Linux / macOS）
# 用法：
#   curl -fsSL https://raw.githubusercontent.com/Aidan-996/MyToolbox/main/cli/install.sh | sh
# 或下载后执行（可加 --system 装到 /usr/local）：
#   sh install.sh [--system]
#
# 默认免 root：二进制装到 ~/.local/bin，数据装到平台惯例目录。
# 仅依赖 curl 和 tar。

set -e

REPO="Aidan-996/MyToolbox"
BASE="${MYBOX_BASE_URL:-https://github.com/$REPO/releases/latest/download}"

OS="$(uname -s)"
case "$OS" in
    Linux*)
        BIN="mybox-linux-x64"
        DATA="$HOME/.local/share/mybox" ;;
    Darwin*)
        BIN="mybox-macos-universal"
        DATA="$HOME/Library/Application Support/MyBox" ;;
    *)
        echo "error: unsupported OS: $OS (this installer is for Linux / macOS)" >&2
        exit 1 ;;
esac

DEST="$HOME/.local/bin"
if [ "$1" = "--system" ]; then
    DEST="/usr/local/bin"
    if [ "$(id -u)" != "0" ]; then
        echo "error: --system needs root, run with sudo" >&2
        exit 1
    fi
fi

command -v curl >/dev/null 2>&1 || { echo "error: curl is required" >&2; exit 1; }
command -v tar  >/dev/null 2>&1 || { echo "error: tar is required"  >&2; exit 1; }

mkdir -p "$DEST" "$DATA"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo ">> downloading $BIN and data from latest release ..."
curl -fL "$BASE/$BIN" -o "$TMP/mybox"
curl -fL "$BASE/mybox-data.tar.gz" -o "$TMP/data.tgz"

echo ">> installing binary -> $DEST/mybox"
mv -f "$TMP/mybox" "$DEST/mybox"
chmod +x "$DEST/mybox"

echo ">> installing prompts & skills -> $DATA"
tar -xzf "$TMP/data.tgz" -C "$DATA"

case ":$PATH:" in
    *":$DEST:"*) ;;
    *) echo ">> note: $DEST is not in your PATH. Add it with:"
       echo "      export PATH=\"\$PATH:$DEST\"" ;;
esac

"$DEST/mybox" version
echo ">> done. Try: mybox list   (then: mybox install claude)"
