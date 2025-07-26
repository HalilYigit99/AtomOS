# Debug kuralları

# GDB ile debug
debug: build
	qemu-system-x86_64 -cdrom AtomOS.iso -m 512M -s -S &
	@echo "QEMU GDB sunucusu başlatıldı (port 1234)"
	@echo "GDB'yi başlatmak için: gdb kernel.elf"
	@echo "GDB'de 'target remote :1234' komutu ile bağlanın"

# Debug bilgileri ile derle
debug-build: CFLAGS += -g -DDEBUG
debug-build: build

# Logs ile çalıştır
run-debug: build
	qemu-system-x86_64 -cdrom AtomOS.iso -m 512M -d int,cpu_reset -no-reboot -no-shutdown