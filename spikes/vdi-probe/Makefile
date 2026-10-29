# Spike #11: controle virtual a partir de um payload.
#
#   export PS5_PAYLOAD_SDK=~/.local/share/ps5-payload-sdk
#   make            # constrói vdi_probe.elf
#   make deploy PS5_HOST=192.168.68.70

PS5_HOST ?= ps5
PS5_PORT ?= 9021

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error defina PS5_PAYLOAD_SDK)
endif

CFLAGS := -O2 -Wall -Wextra -std=c11

all: vdi_probe.elf

vdi_probe.elf: src/main.c
	$(CC) $(CFLAGS) -o $@ $< -lScePad

deploy: vdi_probe.elf
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(PS5_PORT) vdi_probe.elf

clean:
	rm -f vdi_probe.elf

.PHONY: all deploy clean
