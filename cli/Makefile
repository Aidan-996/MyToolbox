# MyBox 命令行版（Linux / macOS）
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
PREFIX ?= /usr/local
DATADIR ?= $(PREFIX)/share/mybox

all: mybox

mybox: mybox.cpp
	$(CXX) $(CXXFLAGS) -o $@ mybox.cpp

# macOS 通用二进制（Apple Silicon + Intel）：
#   make CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -arch arm64 -arch x86_64"

# 安装二进制 + 共用的 prompts/skills 数据文件（与 Windows 版同一套，位于 ../shared/）
# 用 install -d + install -m（不用 GNU 专属的 -D），macOS/BSD install 也能跑
install: mybox
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m755 mybox $(DESTDIR)$(PREFIX)/bin/mybox
	install -d $(DESTDIR)$(DATADIR)
	cp -R ../shared/prompts $(DESTDIR)$(DATADIR)/
	cp -R ../shared/skills $(DESTDIR)$(DATADIR)/
	@echo "installed: $(DESTDIR)$(PREFIX)/bin/mybox, data: $(DESTDIR)$(DATADIR)"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/mybox
	rm -rf $(DESTDIR)$(DATADIR)

clean:
	rm -f mybox

.PHONY: all install uninstall clean
