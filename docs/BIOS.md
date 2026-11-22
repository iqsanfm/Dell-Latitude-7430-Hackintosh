# Pengaturan BIOS (Dell Latitude 7430, BIOS 1.41.0)

Masuk BIOS dengan menekan **F2** saat logo Dell muncul. Menu boot sekali pakai ada di **F12**.

## Wajib

| Menu | Opsi | Nilai | Kenapa |
| --- | --- | --- | --- |
| Boot Configuration | Secure Boot | **Disabled** | OpenCore tidak ditandatangani |
| Storage | SATA/NVMe Operation | **AHCI/NVMe** (bukan RAID/VMD) | macOS tidak mengenali Intel VMD |
| Performance | C-States Control | **Enabled** | Kalau Disabled, MSR 0xE2 membatasi CPU di C0/C1 dan package C-state selalu 0% (boros daya) |
| Power | Lid Switch | **Enabled** | Kalau Disabled, macOS sama sekali tidak menerima event buka/tutup lid |
| Pre-boot Behavior | Fastboot | **Thorough** | Inisialisasi USB/keyboard lengkap tiap boot |

## Catatan

- **DVMT Pre-Allocated** di laptop ini tidak bisa diubah (tetap 60 MB). Karena itu config memakai `-igfxdvmt`
  plus `framebuffer-patch-enable` dan `framebuffer-stolenmem` di `DeviceProperties`, sesuai panduan WhateverXe.
- **CFG Lock** tidak dibuka. Config memakai quirk `AppleXcpmCfgLock = true`.
- Setelah **update BIOS** atau **reset CMOS**, cek ulang tabel di atas. Opsi Lid Switch dan C-States Control pernah
  ditemukan dalam keadaan Disabled, dan gejalanya mirip bug driver.
