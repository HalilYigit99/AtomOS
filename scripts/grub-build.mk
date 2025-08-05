# GRUB ve ISO oluşturma kuralları (UEFI/EFI destekli)

# Gerekli dizinler
EFI_DIR = $(ISO_DIR)/EFI/BOOT
GRUB_EFI_DIR = $(ISO_DIR)/boot/grub/x86_64-efi

# Tüm object dosyalarını topla
ALL_OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS) $(ASM_OBJECTS)

# Sistem kontrolü ve modül ayarları
UNAME_S := $(shell uname -s)

# Minimal temel modüller (hem BIOS hem UEFI'de çalışır)
GRUB_MODULES_COMMON = normal multiboot2 configfile echo boot chain halt reboot \
                      ls cat test loadenv sleep font terminal part_gpt part_msdos \
                      fat ext2 iso9660

# BIOS için ek modüller
GRUB_MODULES_BIOS = $(GRUB_MODULES_COMMON) vbe vga video_bochs video_cirrus \
                    gfxterm gfxterm_background all_video

# UEFI için ek modüller (VBE olmadan)
GRUB_MODULES_EFI = $(GRUB_MODULES_COMMON) efi_gop efi_uga gfxterm \
                   gfxterm_background video_bochs video_cirrus

# Platform bazında güvenli ek modüller
ifeq ($(UNAME_S),Linux)
    GRUB_MODULES_BIOS += ntfs
    GRUB_MODULES_EFI += ntfs
endif

# Kernel ELF dosyasını oluştur
kernel.elf: $(ALL_OBJECTS)
	ld -m elf_i386 -T $(SCRIPTS_DIR)/linker.ld -o $@ $(ALL_OBJECTS)
	@echo "Kernel ELF oluşturuldu: $@"

# EFI boot dosyasını oluştur
bootx64.efi: prepare-iso-dirs
	@echo "BOOTX64.EFI oluşturuluyor..."
	
	# Platform kontrolü ve EFI dosyası oluşturma (UEFI modülleri kullan)
	@if command -v grub-mkimage >/dev/null 2>&1; then \
		grub-mkimage -O x86_64-efi -p /EFI/BOOT -o $(EFI_DIR)/BOOTX64.EFI $(GRUB_MODULES_EFI); \
	elif command -v grub2-mkimage >/dev/null 2>&1; then \
		grub2-mkimage -O x86_64-efi -p /EFI/BOOT -o $(EFI_DIR)/BOOTX64.EFI $(GRUB_MODULES_EFI); \
	else \
		echo "HATA: grub-mkimage veya grub2-mkimage bulunamadı!"; \
		echo "Lütfen GRUB2 EFI araçlarını kurun:"; \
		echo "  Ubuntu/Debian: sudo apt install grub-efi-amd64-bin"; \
		echo "  CentOS/RHEL:   sudo yum install grub2-efi-x64-modules"; \
		echo "  macOS:         brew install grub"; \
		exit 1; \
	fi
	@echo "BOOTX64.EFI oluşturuldu"

# ISO dizin yapısını hazırla
prepare-iso-dirs:
	@echo "ISO dizin yapısı hazırlanıyor..."
	@mkdir -p $(ISO_DIR)/boot/grub
	@mkdir -p $(EFI_DIR)
	@mkdir -p $(GRUB_EFI_DIR)

# GRUB konfigürasyonunu kopyala
copy-grub-config: prepare-iso-dirs
	@echo "GRUB konfigürasyonu kopyalanıyor..."
	@cp $(SCRIPTS_DIR)/grub.cfg $(ISO_DIR)/boot/grub/
	
	# EFI için de konfigürasyonu kopyala
	@mkdir -p $(EFI_DIR)/grub
	@cp $(SCRIPTS_DIR)/grub.cfg $(EFI_DIR)/grub/grub.cfg

# Kernel dosyasını kopyala
copy-kernel: kernel.elf prepare-iso-dirs
	@echo "Kernel dosyası kopyalanıyor..."
	@cp kernel.elf $(ISO_DIR)/boot/

# Hibrit ISO oluştur (BIOS + UEFI)
iso: prepare-iso-dirs copy-kernel copy-grub-config bootx64.efi
	@echo "Hibrit ISO oluşturuluyor (BIOS + UEFI desteği)..."
	
	# Basit hibrit ISO oluştur
	@if command -v grub-mkrescue >/dev/null 2>&1; then \
		grub-mkrescue -o AtomOS.iso $(ISO_DIR); \
	elif command -v grub2-mkrescue >/dev/null 2>&1; then \
		grub2-mkrescue -o AtomOS.iso $(ISO_DIR); \
	else \
		echo "HATA: grub-mkrescue veya grub2-mkrescue bulunamadı!"; \
		exit 1; \
	fi
	
	@echo ""
	@echo "=========================================="
	@echo "AtomOS.iso başarıyla oluşturuldu!"
	@echo "=========================================="
	@echo "ÖZELLIKLER:"
	@echo "  ✓ BIOS/Legacy boot desteği"
	@echo "  ✓ UEFI boot desteği (64-bit)"
	@echo "  ✓ Hibrit ISO (MBR + GPT)"
	@echo "  ✓ USB'ye yazılabilir"
	@echo ""
	@echo "TEST ETMEK IÇIN:"
	@echo "  BIOS: make run"
	@echo "  UEFI: make run-uefi"
	@echo ""
	@cp AtomOS.iso /mnt/c/users/halil/desktop 2>/dev/null || echo "Desktop kopyalama atlandı"

# EFI test hedefi
iso-efi-test: iso
	@echo "EFI ISO test dosyası oluşturuluyor..."
	@qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom AtomOS.iso -m 512

# ISO doğrulama
verify-iso: iso
	@echo "ISO dosyası doğrulanıyor..."
	@if command -v file >/dev/null 2>&1; then \
		file AtomOS.iso; \
	fi
	@if command -v fdisk >/dev/null 2>&1; then \
		echo "Bölüm tablosu:"; \
		fdisk -l AtomOS.iso 2>/dev/null | head -20; \
	fi

# Temizlik
clean-iso:
	@rm -rf $(ISO_DIR)
	@rm -f AtomOS.iso
	@echo "ISO dosyaları temizlendi"

.PHONY: bootx64.efi prepare-iso-dirs copy-grub-config copy-kernel iso iso-efi-test verify-iso clean-iso