# Debug kuralları (UEFI destekli)

# QEMU debug ayarları
QEMU_DEBUG_FLAGS = -s -S -d int,cpu_reset -monitor stdio
QEMU_DEBUG_SIMPLE = -s -S

# Debug modunda çalıştır (BIOS)
run-debug: iso
	@echo "AtomOS debug modunda başlatılıyor (BIOS)..."
	@echo "GDB bağlantısı: target remote localhost:1234"
	@echo "Çıkmak için: (qemu) quit"
	@$(QEMU_CMD) $(QEMU_FLAGS) $(QEMU_DEBUG_SIMPLE) -cdrom AtomOS.iso

# Debug modunda çalıştır (UEFI)
run-uefi-debug: iso
	@echo "AtomOS UEFI debug modunda başlatılıyor..."
	@echo "GDB bağlantısı: target remote localhost:1234"
	@echo "Çıkmak için: (qemu) quit"
	@if [ -f "$(OVMF_PATH)" ]; then \
		$(QEMU_CMD) $(QEMU_FLAGS) $(QEMU_DEBUG_SIMPLE) -bios $(OVMF_PATH) -cdrom AtomOS.iso; \
	else \
		echo "OVMF bulunamadı, BIOS debug modunda çalıştırılıyor..."; \
		$(QEMU_CMD) $(QEMU_FLAGS) $(QEMU_DEBUG_SIMPLE) -cdrom AtomOS.iso; \
	fi

# Detaylı debug (verbose)
run-debug-verbose: iso
	@echo "AtomOS detaylı debug modunda başlatılıyor..."
	@echo "Tüm CPU ve interrupt logları aktif"
	@$(QEMU_CMD) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS) -cdrom AtomOS.iso

# UEFI detaylı debug
run-uefi-debug-verbose: iso
	@echo "AtomOS UEFI detaylı debug modunda başlatılıyor..."
	@if [ -f "$(OVMF_PATH)" ]; then \
		$(QEMU_CMD) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS) -bios $(OVMF_PATH) -cdrom AtomOS.iso; \
	else \
		echo "OVMF bulunamadı, BIOS debug modunda çalıştırılıyor..."; \
		$(QEMU_CMD) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS) -cdrom AtomOS.iso; \
	fi

# GDB oturumu başlat
gdb:
	@echo "GDB oturumu başlatılıyor..."
	@echo "Önce 'make run-debug' veya 'make run-uefi-debug' çalıştırın"
	@gdb -ex "target remote localhost:1234" \
	     -ex "symbol-file kernel.elf" \
	     -ex "set architecture i386:x86-64"

# Debug bilgisi göster
debug-info:
	@echo "===== DEBUG BİLGİLERİ ====="
	@echo "QEMU: $(QEMU_CMD)"
	@echo "Debug Flags: $(QEMU_DEBUG_SIMPLE)"
	@echo "Verbose Flags: $(QEMU_DEBUG_FLAGS)"
	@echo "OVMF: $(OVMF_PATH)"
	@echo ""
	@echo "Debug komutları:"
	@echo "  make run-debug         - BIOS debug"
	@echo "  make run-uefi-debug    - UEFI debug"
	@echo "  make run-debug-verbose - Detaylı BIOS debug"
	@echo "  make run-uefi-debug-verbose - Detaylı UEFI debug"
	@echo "  make gdb               - GDB oturumu"
	@echo ""
	@echo "GDB kullanımı:"
	@echo "  1. Terminal 1: make run-debug"
	@echo "  2. Terminal 2: make gdb"
	@echo "  3. GDB'de: continue (veya c)"

.PHONY: run-debug run-uefi-debug run-debug-verbose run-uefi-debug-verbose gdb debug-info