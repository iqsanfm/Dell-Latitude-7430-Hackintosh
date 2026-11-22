# Boot-args

Nilai `boot-args` di `config.plist` (NVRAM `7C436110-AB2A-4BBB-A880-FE41995C9F82`):

```
-allow3d -disablegfxfirmware npci=0x2000 -s0ixnogpureset -s0ixecwake -btleadv -itlawdlcoexprobe -itlawdliface -itlawdlnativeup -itlawdlcoexrx -itlawdlcoextx -itlawdlcoexmulti -itlawdlcoexdata -itlawdlmitx -itlawdlelicit -itlawdlcaps -itlawdlsvcmerge -itlawdlawsync itlawdlintcoal=4 -itlawdlcoexbatx -itlawdlcoexpeerpause -itlawdlcoexbatrace itlawdltxqueue=4096 -btleadvbeta -igfxdvmt hcpower=4,2
```

## Grafis (WhateverXe)

| Flag | Fungsi |
| --- | --- |
| `-allow3d` | Wajib dari WhateverXe: mengaktifkan akselerasi 3D |
| `-disablegfxfirmware` | Wajib dari WhateverXe: tidak memuat firmware GuC |
| `-igfxdvmt` | DVMT BIOS tetap 60 MB, jadi stolen memory di-patch lewat `DeviceProperties` |

## Sistem

| Flag | Fungsi |
| --- | --- |
| `npci=0x2000` | Mencegah hang saat konfigurasi PCI |
| `hcpower=4,2` | IntelHybridCores: mode 4 saat baterai, mode 2 saat charger |

## S0ixStandby

| Flag | Fungsi |
| --- | --- |
| `-s0ixnogpureset` | Jangan pernah me-reset iGPU saat bangun dari Deep Idle |
| `-s0ixecwake` | Biarkan GPE embedded controller aktif selama Deep Idle |

## Bluetooth LE dan AirDrop (ForbiddenItlwm)

Set flag ini adalah **set resmi yang dites** oleh ForbiddenItlwm. Tanpa flag ini, AirDrop mati dan ForbiddenItlwm
berperilaku seperti AirportItlwm biasa. Penjelasan per flag ada di dokumentasi ForbiddenItlwm (`docs/BOOT-FLAGS.md`).

| Flag | Ringkasan |
| --- | --- |
| `-btleadv`, `-btleadvbeta` | Menyalakan BTLEAdvFixup, termasuk di versi macOS yang belum ditandai kompatibel |
| `-itlawdlcoexprobe`, `-itlawdliface`, `-itlawdlnativeup` | Membuat listener AWDL dan interface `awdl0` |
| `-itlawdlcoexrx`, `-itlawdlcoextx`, `-itlawdlcoexmulti`, `-itlawdlcoexdata` | Terima/kirim frame AWDL di kanal sendiri |
| `-itlawdlmitx`, `-itlawdlelicit`, `-itlawdlcaps`, `-itlawdlsvcmerge`, `-itlawdlawsync` | Pengumuman AWDL agar Mac terlihat oleh iPhone |
| `-itlawdlcoexbatx`, `-itlawdlcoexpeerpause`, `-itlawdlcoexbatrace` | Block Ack dan aggregasi (kecepatan Mac → iPhone) |
| `itlawdlintcoal=4`, `itlawdltxqueue=4096` | Latensi dan ukuran antrean kirim `awdl0` |

## Flag debug (opsional, sudah dihapus dari config)

Flag ini dipakai saat debugging di laptop ini. Tambahkan lagi hanya kalau diperlukan.

| Flag | Fungsi |
| --- | --- |
| `-v` | Boot verbose (teks) |
| `keepsyms=1` | Simbol kernel di laporan panic |
| `debug=0x100` | Jangan langsung reboot saat panic, supaya pesannya bisa dibaca |
| `io=0x800` | Panic kalau ada driver yang timeout saat `setPowerState`. Laporan panic akan menyebut nama driver-nya |
| `-s0ixdbg` | Log detail S0ixStandby |
| `-itllog` | Log internal ForbiddenItlwm di ioreg (`ITLAWDLLog`, hex). **Bisa berisi alamat jaringan**, jadi periksa dulu sebelum dibagikan |
| `-ddvdbg` | Log debug SMCDellDDV |
