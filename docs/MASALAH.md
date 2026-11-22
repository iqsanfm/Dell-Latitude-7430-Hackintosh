# Masalah yang diketahui

## Wi-Fi tidak nyala setelah boot

**Gejala:** Wi-Fi tidak scan atau tidak konek setelah boot. Normal lagi setelah dimatikan lalu dinyalakan dari Control Center.

**Penyebab:** bug ForbiddenItlwm (issue upstream #20). Firmware tidak *ALIVE* saat interface di-enable, sehingga
`SCAN_MANAGER` berhenti.

**Solusi:** pasang [`tools/wifi-boot`](../tools/wifi-boot) ([POST-INSTALL.md](POST-INSTALL.md#4-penjaga-wi-fi-saat-boot)).

## Sleep tidak didukung

WhateverXe belum mendukung siklus sleep/wake di Alder Lake-P, jadi sleep dimatikan dengan `pmset disablesleep 1`.
Lid dan display sleep tetap jalan.

## Lid tidak terbaca

Opsi BIOS **Lid Switch** dalam keadaan Disabled. Nyalakan di BIOS. Masalah ini bukan dari SSDT atau kext.

## Package C-state selalu 0% / boros daya

Opsi BIOS **C-States Control** dalam keadaan Disabled. Setelah dinyalakan, package C-state idle mencapai 86–89%.

## Keyboard mati di BIOS / picker setelah Restart

Sudah diperbaiki oleh VoodooPS2 *shutfix* ([KEXTS.md](KEXTS.md#kext-yang-dimodifikasi)). Kalau kembali muncul
setelah VoodooPS2 diganti versi upstream, pasang lagi patch-nya. Restart dari Linux tidak mengalami masalah ini.

## Kursor trackpad melompat

Penyebabnya polling ACPI SMCDellDDV yang memakai workloop bersama. Sudah diperbaiki sejak SMCDellDDV 1.0.5
(workloop sendiri). Kalau gejala ini kembali, tes dengan boot-arg `-ddvoff`.

## Partisi EFI internal korup

Di laptop ini, partisi EFI dipakai bersama bootloader Linux (limine). Pernah terjadi file kext berubah menjadi
`FSCK0000.000` setelah `fsck`, yang membuat kernel panic dan keyboard/trackpad mati. Cek dengan
`sudo fsck_msdos -n /dev/diskXsY` sebelum dan sesudah menyalin EFI, dan simpan selalu backup EFI di flashdisk.

## Sequoia di SSD eksternal panic "nvme Command timeout Identify"

Ini terjadi saat booting macOS 15 dari disk lain sementara NVMe SK hynix internal terpasang. Solusinya: di
`Kernel > Block`, blokir `com.apple.iokit.IONVMeFamily` (Strategy `Disable`) **khusus kernel 24.x**. Config ini tidak
memakai blok itu karena hanya ditujukan untuk Tahoe.
