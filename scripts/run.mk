# Çalıştırma ve test kuralları

# QEMU ayarları
QEMU_FLAGS = -m 512M -serial stdio -vga std -no-reboot -no-shutdown
QEMU_DEBUG_FLAGS = -s -S
OVMF_PATH = /usr/share/ovmf/OVMF.fd

# Platform algılama
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    QEMU_CMD = qemu-system-x86_64
    # Ubuntu/Debian OVMF yolu
    ifeq ($(wildcard /usr/share/ovmf/OVMF.fd),)
        ifeq ($(wildcard /usr/share/OVMF/OVMF_CODE.fd),)
            OVMF_PATH = /usr/share/edk2-ovmf/x64/OVMF.fd
        else
            OVMF_PATH = /usr/share/OVMF/OVMF_CODE.fd
        endif
    endif
else ifeq ($(UNAME_S),Darwin)
    QEMU_CMD = qemu-system-x86_64
    # macOS Homebrew OVMF yolu
    OVMF_PATH = /opt/homebrew/share/qemu/edk2-x86_64-code.fd
    ifeq ($(wildcard $(OVMF_PATH)),)
        OVMF_PATH = /usr/local/share/qemu/edk2-x86_64-code.fd
    endif
else
    QEMU_CMD = qemu-system-x86_64
    OVMF_PATH = OVMF.fd
endif

# BIOS modda çalıştır
run: iso
	@echo "AtomOS BIOS modunda başlatılıyor..."
	@echo "Çıkmak için: Ctrl+A, X"
	@$(QEMU_CMD) $(QEMU_FLAGS) -cdrom AtomOS.iso

# UEFI modunda çalıştır
run-uefi: iso
	@echo "AtomOS UEFI modunda başlatılıyor..."
	@echo "Çıkmak için: Ctrl+A, X"
	@if [ -f "$(OVMF_PATH)" ]; then \
		echo "OVMF kullanılıyor: $(OVMF_PATH)"; \
		$(QEMU_CMD) $(QEMU_FLAGS) -bios $(OVMF_PATH) -cdrom AtomOS.iso; \
	else \
		echo "UYARI: OVMF bulunamadı: $(OVMF_PATH)"; \
		echo "UEFI test için OVMF kurmanız gerekiyor:"; \
		echo "  Ubuntu/Debian: sudo apt install ovmf"; \
		echo "  CentOS/RHEL:   sudo yum install edk2-ovmf"; \
		echo "  macOS:         brew install qemu (OVMF dahil)"; \
		echo ""; \
		echo "Şimdilik BIOS modunda çalıştırılıyor..."; \
		$(QEMU_CMD) $(QEMU_FLAGS) -cdrom AtomOS.iso; \
	fi

# Güvenli mod (KVM devre dışı)
run-safe: iso
	@echo "AtomOS güvenli modda başlatılıyor (KVM devre dışı)..."
	@$(QEMU_CMD) $(QEMU_FLAGS) -no-kvm -cdrom AtomOS.iso

# UEFI güvenli mod
run-uefi-safe: iso
	@echo "AtomOS UEFI güvenli modda başlatılıyor..."
	@if [ -f "$(OVMF_PATH)" ]; then \
		$(QEMU_CMD) $(QEMU_FLAGS) -no-kvm -bios $(OVMF_PATH) -cdrom AtomOS.iso; \
	else \
		echo "OVMF bulunamadı, BIOS modunda çalıştırılıyor..."; \
		$(QEMU_CMD) $(QEMU_FLAGS) -no-kvm -cdrom AtomOS.iso; \
	fi

# USB test (ISO'yu USB gibi mount ederek)
run-usb: iso
	@echo "AtomOS USB boot simülasyonu..."
	@$(QEMU_CMD) $(QEMU_FLAGS) -drive format=raw,file=AtomOS.iso,if=ide

# UEFI USB test
run-uefi-usb: iso
	@echo "AtomOS UEFI USB boot simülasyonu..."
	@if [ -f "$(OVMF_PATH)" ]; then \
		$(QEMU_CMD) $(QEMU_FLAGS) -bios $(OVMF_PATH) -drive format=raw,file=AtomOS.iso,if=ide; \
	else \
		echo "OVMF bulunamadı, BIOS USB modunda çalıştırılıyor..."; \
		$(QEMU_CMD) $(QEMU_FLAGS) -drive format=raw,file=AtomOS.iso,if=ide; \
	fi

# Debug modunda çalıştır (BIOS) - debug.mk dosyasında tanımlanmış
# run-debug hedefi debug.mk dosyasında mevcut

# Debug modunda çalıştır (UEFI) - debug.mk dosyasında tanımlanmış  
# run-uefi-debug hedefi debug.mk dosyasında mevcut

# Sistem bilgilerini göster
show-qemu-info:
	@echo "===== QEMU Sistem Bilgileri ====="
	@echo "Platform: $(UNAME_S)"
	@echo "QEMU Komutu: $(QEMU_CMD)"
	@echo "OVMF Yolu: $(OVMF_PATH)"
	@echo "OVMF Durumu: $$(if [ -f '$(OVMF_PATH)' ]; then echo 'Mevcut ✓'; else echo 'Bulunamadı ✗'; fi)"
	@echo ""
	@echo "Kullanılabilir test komutları:"
	@echo "  make run          - BIOS modunda çalıştır"
	@echo "  make run-uefi     - UEFI modunda çalıştır"
	@echo "  make run-safe     - BIOS güvenli mod"
	@echo "  make run-uefi-safe - UEFI güvenli mod"
	@echo "  make run-usb      - USB boot simülasyonu"
	@echo "  make run-uefi-usb - UEFI USB boot simülasyonu"
	@echo ""

# Hızlı test (hem BIOS hem UEFI)
test-both: iso
	@echo "Her iki modda da hızlı test yapılıyor..."
	@echo ""
	@echo "=== BIOS TEST ==="
	@timeout 10 $(QEMU_CMD) $(QEMU_FLAGS) -cdrom AtomOS.iso || echo "BIOS test tamamlandı"
	@echo ""
	@echo "=== UEFI TEST ==="
	@if [ -f "$(OVMF_PATH)" ]; then \
		timeout 10 $(QEMU_CMD) $(QEMU_FLAGS) -bios $(OVMF_PATH) -cdrom AtomOS.iso || echo "UEFI test tamamlandı"; \
	else \
		echo "OVMF bulunamadı, UEFI test atlandı"; \
	fi

.PHONY: run run-uefi run-safe run-uefi-safe run-usb run-uefi-usb show-qemu-info test-both