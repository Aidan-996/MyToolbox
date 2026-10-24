# Ua Toolbox Linux 版
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
PREFIX ?= /usr/local
DATADIR ?= $(PREFIX)/share/ua

all: ua

ua: ua.cpp
	$(CXX) $(CXXFLAGS) -o $@ ua.cpp

# 安装二进制 + 共用的 prompts/skills 数据文件（与 Windows 版同一套，位于 ../shared/）
install: ua
	install -Dm755 ua $(DESTDIR)$(PREFIX)/bin/ua
	install -d $(DESTDIR)$(DATADIR)
	cp -r ../shared/prompts $(DESTDIR)$(DATADIR)/
	cp -r ../shared/skills $(DESTDIR)$(DATADIR)/
	@echo "installed: $(DESTDIR)$(PREFIX)/bin/ua, data: $(DESTDIR)$(DATADIR)"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/ua
	rm -rf $(DESTDIR)$(DATADIR)

clean:
	rm -f ua

.PHONY: all install uninstall clean
