# SPDX-License-Identifier: GPL-2.0-only
KVER ?= $(shell uname -r)
KDIR ?= /lib/modules/$(KVER)/build

.PHONY: all modules clean test check install uninstall
all: modules
modules:
	$(MAKE) -C $(KDIR) M=$(CURDIR) W=1 modules
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
test:
	python3 tests/test_install.py
check:
	./scripts/check.sh
install:
	./scripts/install.sh
uninstall:
	./scripts/uninstall.sh
