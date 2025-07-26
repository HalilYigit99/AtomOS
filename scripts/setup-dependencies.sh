#!/bin/bash
# AtomOS Bağımlılık Kurulum Scripti

echo "AtomOS için gerekli bağımlılıklar kuruluyor..."

# İşletim sistemi kontrolü
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux (Ubuntu/Debian)
    echo "Linux sistemi tespit edildi."
    sudo apt update
    sudo apt install -y build-essential nasm xorriso grub-pc-bin grub-common qemu-system-x86 gdb
    
    # Cross-compiler kurulum kontrol
    if ! command -v x86_64-elf-gcc &> /dev/null; then
        echo "x86_64-elf-gcc bulunamadı. Cross-compiler kurulumu gerekli."
        echo "Lütfen şu adımları takip edin:"
        echo "1. https://wiki.osdev.org/GCC_Cross-Compiler"
        echo "2. x86_64-elf-gcc ve x86_64-elf-binutils kurun"
    fi
    
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "macOS sistemi tespit edildi."
    if command -v brew &> /dev/null; then
        brew install nasm xorriso qemu gdb
        echo "Cross-compiler için Homebrew'dan x86_64-elf-gcc kurmanız gerekebilir."
    else
        echo "Homebrew bulunamadı. Lütfen önce Homebrew kurun."
    fi
    
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    # Windows (MSYS2/Cygwin)
    echo "Windows sistemi tespit edildi."
    echo "Lütfen MSYS2 üzerinden şu paketleri kurun:"
    echo "pacman -S mingw-w64-x86_64-toolchain nasm grub qemu"
    
else
    echo "Desteklenmeyen işletim sistemi: $OSTYPE"
    echo "Lütfen manuel olarak gerekli araçları kurun:"
    echo "- x86_64-elf-gcc"
    echo "- nasm"
    echo "- grub-mkrescue"
    echo "- qemu-system-x86_64"
fi

echo "Bağımlılık kurulum scripti tamamlandı."