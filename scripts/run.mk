# Çalıştırma kuralları

# QEMU ile çalıştır
run: build
	qemu-system-x86_64 -cdrom AtomOS.iso -m 512M

# QEMU ile serial çıktı ile çalıştır
run-serial: build
	qemu-system-x86_64 -cdrom AtomOS.iso -m 512M -serial stdio

# VirtualBox ile çalıştır (eğer kuruluysa)
run-vbox: build
	@if command -v VBoxManage >/dev/null 2>&1; then \
		echo "VirtualBox ile çalıştırılıyor..."; \
		VBoxManage startvm "AtomOS" --type headless || echo "VM bulunamadı, manuel olarak oluşturun."; \
	else \
		echo "VirtualBox kurulu değil."; \
	fi