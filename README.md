# Dell Latitude 7430 — OpenCore EFI (macOS Tahoe)

EFI OpenCore untuk **Dell Latitude 7430** (Intel Core i7-1265U, Alder Lake-P) yang menjalankan **macOS Tahoe 26.7.1**.
Isinya sama dengan EFI yang dipakai sehari-hari di laptop ini. Sebelum di-upload, data pribadi (SMBIOS) dan sisa debug sudah dibersihkan.

> [!WARNING]
> **SMBIOS di `config.plist` sengaja dikosongkan.** Kamu wajib membuat serial sendiri sebelum boot.
> Caranya ada di [docs/POST-INSTALL.md](docs/POST-INSTALL.md#1-smbios).

## Spesifikasi

| Komponen | Detail |
| --- | --- |
| Model | Dell Latitude 7430, BIOS 1.41.0 |
| CPU | Intel Core i7-1265U (2P + 8E core, 12 thread) |
| iGPU | Intel Iris Xe Graphics (`8086:46A8`), DVMT 60 MB |
| RAM | 32 GB |
| Layar | 14" 1920×1080 eDP |
| Penyimpanan | NVMe SK hynix 512 GB |
| Wi-Fi / BT | Intel AX211 (CNVi) |
| Audio | Realtek (codec HDA) |
| Trackpad | Synaptics (I2C/RMI) |
| Keyboard | PS/2 |
| SMBIOS | `MacBookPro16,2` |

## Versi

| Komponen | Versi |
| --- | --- |
| macOS | Tahoe 26.7.1 (25G241) |
| OpenCore | 1.0.8 RELEASE |
| Lilu | 1.7.3 |
| WhateverXe | 0.8.3 |
| ForbiddenItlwm | 0.6.0 |

Daftar lengkap ada di [docs/KEXTS.md](docs/KEXTS.md).

## Status

| Fitur | Status | Catatan |
| --- | --- | --- |
| Akselerasi grafis (Metal, Core Animation) | ✅ | WhateverXe + paket **WhateverXe Graphics** (dipasang setelah instalasi) |
| Brightness layar + tombol | ✅ | BrightnessKeys |
| Wi-Fi | ✅ | ForbiddenItlwm, Wi-Fi 6 (`itlwm_he=1`), ~345/348 Mbps di 5 GHz. Kadang tidak nyala sendiri saat boot (bug upstream), lihat [Masalah](docs/MASALAH.md#wi-fi-tidak-nyala-setelah-boot) |
| AirDrop / AWDL | ✅ | Pakai set flag `-itlawdl*` dari ForbiddenItlwm |
| Bluetooth (termasuk TWS/audio) | ✅ | IntelBluetoothFirmware + BlueToolFixup + BTLEAdvFixup |
| Audio (speaker, headphone, mic) | ✅ | VoodooHDA di `/Library/Extensions`, **tidak** ada di EFI |
| Trackpad + gesture | ✅ | VoodooI2C + VoodooRMI |
| Keyboard | ✅ | VoodooPS2 (patch *shutfix*: keyboard tetap jalan di BIOS/picker setelah Restart) |
| Baterai | ✅ | SMCBatteryManager |
| Sensor suhu + RPM kipas | ✅ | SMCProcessor + **SMCDellDDV** (kext buatan sendiri) |
| Lid (tutup → layar mati) | ✅ | Opsi BIOS **Lid Switch** wajib Enabled |
| Hemat daya idle (S0ix / package C-state) | ✅ | S0ixStandby Deep Idle + SSDT-DEEPIDLE. Opsi BIOS **C-States Control** wajib Enabled |
| Scheduler P-core / E-core | ✅ | IntelHybridCores (`hcpower=4,2`) |
| Restart / Shut Down | ✅ | |
| Sleep (tidur) | ❌ | Dimatikan (`pmset disablesleep 1`): WhateverXe belum mendukung sleep di Alder Lake-P |
| Webcam, HDMI/USB-C display, fingerprint, kartu SD | ❔ | Belum dites |

## Struktur repo

```
EFI/                 EFI siap pakai (BOOT + OC), salin ke partisi EFI
ACPI-src/            Source .dsl dari semua SSDT di EFI/OC/ACPI
src/SMCDellDDV/      Source + skrip build kext SMCDellDDV
patches/             Patch yang dipakai pada kext upstream (VoodooPS2, VoodooInput, VoodooGPIO)
tools/wifi-boot/     LaunchDaemon: nyalakan ulang Wi-Fi kalau firmware gagal start saat boot
docs/                Dokumentasi
```

## Cara pakai

1. Atur BIOS sesuai [docs/BIOS.md](docs/BIOS.md).
2. Buat SMBIOS sendiri dan isi ke `EFI/OC/config.plist` ([docs/POST-INSTALL.md](docs/POST-INSTALL.md#1-smbios)).
3. Salin folder `EFI` ke partisi EFI flashdisk, lalu **uji dulu dari flashdisk** (F12 → pilih flashdisk).
4. Setelah boot lancar, salin ke partisi EFI di NVMe internal.
5. Ikuti langkah setelah instalasi di [docs/POST-INSTALL.md](docs/POST-INSTALL.md): paket grafis WhateverXe, audio, dan penjaga Wi-Fi.

> [!TIP]
> Simpan selalu satu flashdisk berisi EFI yang sudah terbukti jalan, plus recovery macOS. Kalau EFI internal rusak,
> tinggal F12 → flashdisk.

## Dokumentasi

| File | Isi |
| --- | --- |
| [docs/BIOS.md](docs/BIOS.md) | Pengaturan BIOS yang wajib |
| [docs/KEXTS.md](docs/KEXTS.md) | Daftar kext, fungsinya, sumbernya, dan kext yang dimodifikasi |
| [docs/ACPI.md](docs/ACPI.md) | SSDT dan patch ACPI |
| [docs/BOOT-ARGS.md](docs/BOOT-ARGS.md) | Penjelasan setiap boot-arg, plus flag debug opsional |
| [docs/POST-INSTALL.md](docs/POST-INSTALL.md) | SMBIOS, paket grafis, audio, penjaga Wi-Fi, `pmset` |
| [docs/MASALAH.md](docs/MASALAH.md) | Masalah yang diketahui dan solusinya |

## Kredit

Semua kext pihak ketiga tetap milik pembuatnya masing-masing, dengan lisensinya sendiri. Daftar dan tautannya ada di
[docs/KEXTS.md](docs/KEXTS.md). Terima kasih khusus untuk acidanthera (OpenCore, Lilu, VirtualSMC, VoodooPS2),
b00t0x (WhateverXe), retropineaple (ForbiddenItlwm), labrouss (S0ixStandby), rodgod-rr (IntelHybridCores),
tim VoodooI2C, 1Revenger1 (VoodooRMI), zxystd (IntelBluetoothFirmware), dan dhinakg (USBToolBox).
