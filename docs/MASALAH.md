# Masalah yang diketahui

## Wi-Fi tidak nyala setelah boot

**Gejala:** Wi-Fi tidak scan atau tidak konek setelah boot. Normal lagi setelah dimatikan lalu dinyalakan dari Control Center.

**Penyebab:** bug ForbiddenItlwm (issue upstream #20). Firmware tidak *ALIVE* saat interface di-enable, sehingga
`SCAN_MANAGER` berhenti.

**Solusi:** pasang [`tools/wifi-boot`](../tools/wifi-boot) ([POST-INSTALL.md](POST-INSTALL.md#4-penjaga-wi-fi-saat-boot)).

## Wi-Fi lambat (download jauh di bawah upload)

**Gejala:** upload normal, tapi download cuma puluhan Mbps, padahal OS lain di laptop yang sama dapat ratusan.

**Penyebab:** boot-arg `-itlawdlcoexmulti` membuat firmware membagi waktu radio antara kanal AWDL dan kanal router.

**Solusi:** hapus `-itlawdlcoexmulti` dari boot-args, dan tambahkan `itlwm_he=1` untuk Wi-Fi 6. Config di repo ini
sudah begitu. Detail dan hasil ukurnya ada di [BOOT-ARGS.md](BOOT-ARGS.md#kecepatan-wi-fi).

## Sleep masih eksperimental

Sleep (Deep Idle) sudah bisa masuk dan bangun dengan WhateverXe + patch `csb-resync`, tapi belum cukup dites untuk
dipakai sehari-hari, jadi default-nya tetap `pmset disablesleep 1`. Status, hasil tes, dan cara mencobanya ada di
[SLEEP.md](SLEEP.md).

## Wi-Fi baru tersambung ±1 menit setelah bangun dari sleep

Setelah bangun, ForbiddenItlwm melaporkan interface *OFF* selama ±20 detik, lalu tersambung sendiri dalam 40–60 detik.
Belum ada solusi. Tunggu saja, atau matikan lalu nyalakan Wi-Fi dari Control Center.

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
