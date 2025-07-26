# AtomOS - Bare Metal İşletim Sistemi

AtomOS, x86_64 mimarisi için geliştirilmiş bare metal bir işletim sistemi projesidir.

## Özellikler

- x86_64 mimarisi desteği
- GRUB2 bootloader kullanımı
- Modüler yapı (Kernel + Standard Library)
- C/C++ ve Assembly dil desteği

## Gereksinimler

- x86_64-elf-gcc (Cross-compiler)
- NASM (Netwide Assembler)
- GRUB2 tools (grub-mkrescue)
- QEMU (Test için)
- Make

## Kurulum

1. Bağımlılıkları kurun:
```bash
make setup-deps