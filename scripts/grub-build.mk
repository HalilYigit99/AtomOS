# GRUB ve ISO oluşturma kuralları

# Tüm object dosyalarını topla
ALL_OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS) $(ASM_OBJECTS)

# Kernel ELF dosyasını oluştur
kernel.elf: $(ALL_OBJECTS)
	ld -m elf_i386 -T $(SCRIPTS_DIR)/linker.ld -o $@ $(ALL_OBJECTS)
	@echo "Kernel ELF oluşturuldu: $@"

# ISO oluştur
iso: kernel.elf
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp kernel.elf $(ISO_DIR)/boot/
	@cp $(SCRIPTS_DIR)/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o AtomOS.iso $(ISO_DIR)
	@echo "ISO oluşturuldu: AtomOS.iso"