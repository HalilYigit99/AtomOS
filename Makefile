# AtomOS Ana Makefile
# Derleyici ayarları
CC = x86_64-elf-gcc
ASM = nasm
CFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -m32
ASMFLAGS = -f elf32

# Dizinler
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
STD_DIR = std
ISO_DIR = iso
SCRIPTS_DIR = scripts

# Hedefler
.PHONY: all build run debug clean setup-deps std-build iso

all: build

# Script dosyalarını dahil et
include $(SCRIPTS_DIR)/c-files.mk
include $(SCRIPTS_DIR)/cpp-files.mk
include $(SCRIPTS_DIR)/asm-files.mk
include $(SCRIPTS_DIR)/std-build.mk
include $(SCRIPTS_DIR)/grub-build.mk
include $(SCRIPTS_DIR)/run.mk
include $(SCRIPTS_DIR)/debug.mk

# Ana build hedefi
build: setup-dirs std-build kernel.elf iso

# Gerekli dizinleri oluştur
setup-dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(STD_DIR)/build
	@mkdir -p $(ISO_DIR)/boot/grub

# Temizlik
clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(STD_DIR)/build
	@rm -rf $(ISO_DIR)
	@rm -f $(STD_DIR)/stdlib.so
	@rm -f kernel.elf
	@rm -f AtomOS.iso
	@echo "Temizlik tamamlandı."

# Bağımlılıkları kur
setup-deps:
	@chmod +x $(SCRIPTS_DIR)/setup-dependencies.sh
	@$(SCRIPTS_DIR)/setup-dependencies.sh