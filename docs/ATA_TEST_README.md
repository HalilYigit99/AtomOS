# ATA Disk Sürücüsü Test Rehberi

Bu rehber AtomOS ATA disk sürücüsünün nasıl test edileceğini ve kullanılacağını açıklar.

## 🚀 Hızlı Başlangıç

### 1. Testleri Çalıştırma

Ana kernel dosyasında (`src/main.c`) test kodlarının yorum satırlarını kaldırın:

```c
// Bu satırları aktif hale getirin:
currentOutputStream->printf("\n=== Running ATA Driver Tests ===\n");
ata_test_main();
ata_usage_example();
```

### 2. Derleme ve Çalıştırma

```bash
make clean
make build
make run
```

## 📋 Test Modülleri

### 1. **ATA Initialization Test**
- ATA controller'ı başlatır
- Temel yapılandırmaları kontrol eder

### 2. **Device Detection Test**
- Mevcut ATA cihazlarını algılar
- Primary/Secondary Master/Slave'leri listeler
- Cihaz sayısını raporlar

### 3. **Device Information Test**
- Cihaz modellerini, seri numaralarını gösterir
- Disk boyutlarını, sektör sayılarını listeler
- Desteklenen özellik setlerini kontrol eder

### 4. **Basic I/O Operations Test**
- Tek sektör okuma/yazma işlemlerini test eder
- Veri bütünlüğünü doğrular
- Güvenli test sektörleri kullanır (LBA 1000)

### 5. **Multi-Sector I/O Test**
- Çoklu sektör (4 sektör) okuma/yazma
- Sektör bazında veri doğrulama
- Performans değerlendirmesi

### 6. **Error Handling Test**
- NULL pointer kontrolü
- Geçersiz parametre kontrolü
- Sınır dışı LBA kontrolü

### 7. **ATAPI Support Test**
- CD/DVD sürücü desteği
- ATAPI cihaz algılama
- Optik medya okuma testi

### 8. **Performance Test**
- Okuma/yazma hız ölçümü
- Büyük veri blokları ile test
- PIO vs DMA karşılaştırması

## 💡 Kullanım Örnekleri

### Basit Cihaz Erişimi
```c
#include <driver/ata/ata.h>

// İlk cihazı al
AtaDevice* device = ata_get_device(0);
if (device) {
    printf("Model: %s\n", ata_get_model(device));
    printf("Boyut: %llu sektör\n", ata_get_disk_size(device));
}
```

### Sektör Okuma
```c
uint8_t buffer[512];
if (ata_read_sector(device, 1000, buffer)) {
    printf("Sektör başarıyla okundu\n");
    // buffer'ı kullan
} else {
    ata_print_error(device);
}
```

### Sektör Yazma
```c
uint8_t data[512] = {0};
strcpy((char*)data, "AtomOS Test Data");

if (ata_write_sector(device, 1000, data)) {
    printf("Sektör başarıyla yazıldı\n");
    ata_flush_cache(device); // Cache'i temizle
} else {
    ata_print_error(device);
}
```

### Çoklu Sektör İşlemleri
```c
uint8_t big_buffer[512 * 8]; // 8 sektör

// 8 sektör oku
if (ata_read_sectors(device, 1000, 8, big_buffer)) {
    printf("8 sektör başarıyla okundu\n");
}

// 8 sektör yaz
if (ata_write_sectors(device, 1000, 8, big_buffer)) {
    printf("8 sektör başarıyla yazıldı\n");
}
```

### Cihaz Enumerasyonu
```c
printf("Bulunan ATA cihazları:\n");
for (uint8_t i = 0; i < ata_get_device_count(); i++) {
    AtaDevice* dev = ata_get_device(i);
    if (dev) {
        ata_print_device_info(dev);
    }
}
```

### ATAPI (CD/DVD) Kullanımı
```c
// ATAPI cihazı bul
for (uint8_t i = 0; i < 4; i++) {
    AtaDevice* device = ata_get_device(i);
    if (device && device->type == ATA_TYPE_PATAPI) {
        uint8_t cd_buffer[2048]; // CD sektör boyutu
        
        // İlk sektörü oku
        if (atapi_read_sectors(device, 0, 1, cd_buffer)) {
            printf("CD sektörü okundu\n");
        }
        
        // Medyayı çıkar
        atapi_eject(device);
        break;
    }
}
```

## ⚠️ Güvenlik Notları

### Güvenli Test Alanları
- **LBA 0-63**: Boot sektörleri - DOKUNMAYIN!
- **LBA 1000-2000**: Test için güvenli alan
- **LBA 2000-3000**: Çoklu sektör testleri için
- **LBA 3000+**: Performans testleri için

### Önemli Uyarılar
1. **Her zaman orijinal veriyi yedekleyin**
2. **Yazma testlerinden önce backup alın**
3. **Cache'i flush etmeyi unutmayın**
4. **Return değerlerini kontrol edin**
5. **NULL pointer'ları kontrol edin**

## 🔧 Hata Ayıklama

### Yaygın Problemler

**"No ATA devices found"**
- QEMU/VirtualBox'ta disk eklenmemiş olabilir
- Gerçek donanımda BIOS/UEFI ayarları kontrol edin

**"Read/Write failed"**
- Disk korumalı olabilir
- Hatalı LBA adresi
- Hardware problemi

**"Data integrity failed"**
- Yazma cache problemi
- Hardware hatası
- Sektör alignment problemi

### Debug Bilgileri
```c
// Hata detaylarını göster
if (!ata_read_sector(device, lba, buffer)) {
    ata_print_error(device);
    uint8_t error = ata_get_error(device->channel);
    printf("Error register: 0x%02X\n", error);
    printf("Error string: %s\n", ata_get_error_string(error));
}
```

## 📊 Performans İpuçları

1. **Çoklu sektör kullanın**: Tek tek sektör yerine toplu işlem
2. **Cache'i etkin kullanın**: Write-back cache avantajları
3. **Alignment**: 512-byte sınırlarında çalışın
4. **DMA implementasyonu**: Gelecekte PIO yerine DMA

## 🎯 Gelecek Geliştirmeler

- [ ] DMA mode desteği
- [ ] IRQ tabanlı asenkron I/O
- [ ] SMART monitoring
- [ ] Hot-plug desteği
- [ ] NCQ (Native Command Queuing)
- [ ] SATA 3.0+ özellikleri

Bu test suite'i kullanarak ATA sürücünüzün doğru çalıştığını doğrulayabilir ve nasıl kullanılacağını öğrenebilirsiniz!
