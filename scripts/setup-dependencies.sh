#!/bin/bash
# AtomOS Bağımlılık Kurulum Scripti (UEFI Destekli)

echo "AtomOS için gerekli bağımlılıklar kuruluyor..."
echo "UEFI/EFI boot desteği dahil"

# İşletim sistemi kontrolü
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux (Ubuntu/Debian)
    echo "Linux sistemi tespit edildi."
    
    # Paket yöneticisi kontrolü
    if command -v apt >/dev/null 2>&1; then
        echo "APT paket yöneticisi kullanılıyor..."
        sudo apt update
        sudo apt install -y build-essential nasm xorriso grub-pc-bin grub-common \
                          grub-efi-amd64-bin grub-efi-amd64 ovmf qemu-system-x86 \
                          gdb mtools dosfstools
        
        # OVMF kurulum kontrolü
        if [ ! -f "/usr/share/ovmf/OVMF.fd" ] && [ ! -f "/usr/share/OVMF/OVMF_CODE.fd" ]; then
            echo "OVMF kurulumu kontrol ediliyor..."
            sudo apt install -y ovmf qemu-efi-amd64 || echo "OVMF kurulamadı, manuel kurulum gerekebilir"
        fi
        
    elif command -v yum >/dev/null 2>&1; then
        echo "YUM paket yöneticisi kullanılıyor..."
        sudo yum groupinstall -y "Development Tools"
        sudo yum install -y nasm xorriso grub2-tools grub2-efi-x64 grub2-efi-x64-modules \
                          edk2-ovmf qemu-system-x86 gdb mtools dosfstools
                          
    elif command -v dnf >/dev/null 2>&1; then
        echo "DNF paket yöneticisi kullanılıyor..."
        sudo dnf groupinstall -y "Development Tools"
        sudo dnf install -y nasm xorriso grub2-tools grub2-efi-x64 grub2-efi-x64-modules \
                          edk2-ovmf qemu-system-x86 gdb mtools dosfstools
                          
    elif command -v pacman >/dev/null 2>&1; then
        echo "Pacman paket yöneticisi kullanılıyor..."
        sudo pacman -Sy --needed base-devel nasm xorriso grub efi-shell ovmf \
                                 qemu-system-x86 gdb mtools dosfstools
    else
        echo "Desteklenmeyen paket yöneticisi. Manuel kurulum gerekli."
    fi
    
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
        echo "Homebrew kullanılarak paketler kuruluyor..."
        brew install nasm xorriso qemu gdb x86_64-elf-gcc x86_64-elf-binutils mtools
        
        # GRUB kurulumu (macOS için)
        if ! command -v grub-mkrescue &> /dev/null; then
            echo "GRUB kuruluyor..."
            brew install grub
        fi
        
        echo "macOS'ta UEFI test için QEMU otomatik olarak OVMF desteği ile gelir."
        
    else
        echo "Homebrew bulunamadı. Lütfen önce Homebrew kurun:"
        echo "https://brew.sh"
    fi
    
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    # Windows (MSYS2/Cygwin)
    echo "Windows sistemi tespit edildi."
    echo "Lütfen MSYS2 üzerinden şu paketleri kurun:"
    echo ""
    echo "pacman -S mingw-w64-x86_64-toolchain nasm"
    echo "pacman -S mingw-w64-x86_64-grub"
    echo "pacman -S mingw-w64-x86_64-qemu"
    echo "pacman -S mingw-w64-x86_64-gdb"
    echo ""
    echo "UEFI test için OVMF:"
    echo "https://www.kraxel.org/repos/jenkins/edk2/ adresinden OVMF dosyalarını indirin"
    
else
    echo "Desteklenmeyen işletim sistemi: $OSTYPE"
    echo "Lütfen manuel olarak gerekli araçları kurun:"
    echo "- x86_64-elf-gcc (Cross-compiler)"
    echo "- nasm (Assembler)"
    echo "- grub-mkrescue (GRUB2 rescue disk creator)"
    echo "- grub-mkimage (EFI image creator)"
    echo "- xorriso (ISO creation tool)"
    echo "- qemu-system-x86_64 (Emulator)"
    echo "- OVMF (UEFI firmware for QEMU)"
fi

echo ""
echo "===== KURULUM DOĞRULAMA ====="

# Gerekli araçların kontrolü
echo -n "Cross-compiler (x86_64-elf-gcc): "
if command -v x86_64-elf-gcc >/dev/null 2>&1; then
    echo "✓ Mevcut ($(x86_64-elf-gcc --version | head -n1))"
else
    echo "✗ Bulunamadı"
fi

echo -n "NASM: "
if command -v nasm >/dev/null 2>&1; then
    echo "✓ Mevcut ($(nasm -v))"
else
    echo "✗ Bulunamadı"
fi

echo -n "GRUB mkrescue: "
if command -v grub-mkrescue >/dev/null 2>&1; then
    echo "✓ Mevcut"
elif command -v grub2-mkrescue >/dev/null 2>&1; then
    echo "✓ Mevcut (grub2-mkrescue)"
else
    echo "✗ Bulunamadı"
fi

echo -n "GRUB mkimage: "
if command -v grub-mkimage >/dev/null 2>&1; then
    echo "✓ Mevcut"
elif command -v grub2-mkimage >/dev/null 2>&1; then
    echo "✓ Mevcut (grub2-mkimage)"
else
    echo "✗ Bulunamadı"
fi

echo -n "Xorriso: "
if command -v xorriso >/dev/null 2>&1; then
    echo "✓ Mevcut"
else
    echo "✗ Bulunamadı"
fi

echo -n "QEMU: "
if command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "✓ Mevcut"
else
    echo "✗ Bulunamadı"
fi

echo -n "OVMF (UEFI firmware): "
OVMF_FOUND=false
for path in "/usr/share/ovmf/OVMF.fd" "/usr/share/OVMF/OVMF_CODE.fd" "/usr/share/edk2-ovmf/x64/OVMF.fd" "/opt/homebrew/share/qemu/edk2-x86_64-code.fd" "/usr/local/share/qemu/edk2-x86_64-code.fd"; do
    if [ -f "$path" ]; then
        echo "✓ Mevcut ($path)"
        OVMF_FOUND=true
        break
    fi
done

if [ "$OVMF_FOUND" = false ]; then
    echo "✗ Bulunamadı"
    echo "  UEFI test için OVMF kurmanız önerilir."
fi

echo ""
echo "===== KULLANIM TALİMATLARI ====="
echo "Kurulum tamamlandı! AtomOS'u derlemek için:"
echo ""
echo "1. Projeyi derle:     make"
echo "2. BIOS modda test:   make run"
echo "3. UEFI modda test:   make run-uefi"
echo "4. Her iki modda:     make test-both"
echo "5. Sistem bilgisi:    make show-qemu-info"
echo ""
echo "ISO dosyası hem BIOS hem UEFI sistemlerde çalışacak şekilde oluşturulur."
echo ""

if [ "$OVMF_FOUND" = false ]; then
    echo "⚠️  UYARI: OVMF bulunamadı. UEFI testleri çalışmayabilir."
    echo "   OVMF kurulum önerileri:"
    echo "   Ubuntu/Debian: sudo apt install ovmf"
    echo "   CentOS/RHEL:   sudo yum install edk2-ovmf"
    echo "   Arch Linux:    sudo pacman -S ovmf"
    echo "   macOS:         brew install qemu (OVMF dahil)"
fi

echo ""
echo "Bağımlılık kurulum scripti tamamlandı."