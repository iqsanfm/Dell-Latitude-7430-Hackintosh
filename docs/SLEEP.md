# Sleep (eksperimental)

Sleep di laptop ini berupa **Deep Idle** (S0ix / *modern standby*) lewat S0ixStandby, bukan S3. Statusnya per
26 November 2026: **masuk dan bangun dari sleep sudah jalan, tapi baru dites singkat**, jadi config di repo ini tetap
memakai `pmset disablesleep 1`.

## Yang dibutuhkan

| Bagian | Isi |
| --- | --- |
| S0ixStandby 0.5.15 + `SSDT-DEEPIDLE` | Deep Idle (tipe sleep 7), boot-arg `-s0ixnogpureset -s0ixecwake` |
| WhateverXe 0.8.5 + patch [`csb-resync`](../patches/WhateverXe-csb-resync.patch) | GPU tetap hidup setelah bangun, boot-arg `-wex-s0-csb-resync` |
| **Spoof CPUID Ice Lake** | Supaya CPU bisa turun ke PC10 dan S0ix tercapai (lihat di bawah) |
| BIOS **C-States Control** Enabled | Tanpa ini package C-state selalu 0 % |

## Spoof CPUID: Comet Lake → Ice Lake

macOS tidak mengenal Alder Lake, jadi CPUID di-spoof lewat `Kernel > Emulate`. Dulu repo ini memakai Comet Lake.
Dengan Comet Lake, XCPM berhenti di core C6 saat Deep Idle, sehingga package hanya sampai **PC6** dan S0ix **0 %**.
Dokumentasi tes S0ixStandby (`TESTING.md`) melaporkan hal yang sama di Tiger Lake, dan dengan spoof Ice Lake angkanya
naik ke PC10 99 %. Di laptop ini (Alder Lake-P) hasilnya sama:

| `Kernel > Emulate` | `Cpuid1Data` | `Cpuid1Mask` |
| --- | --- | --- |
| Comet Lake (lama) | `VQYKAAAAgAAAAAAAAAAAAA==` | `/////wAA/wAAAAAAAAAAAA==` |
| **Ice Lake (sekarang)** | `5QYHAAAAgAAAAAAAAAAAAA==` | `/////wAA/wAAAAAAAAAAAA==` |

SMBIOS `MacBookPro16,2` memang model Ice Lake, jadi spoof ini cocok. Setelah diganti: CPU terbaca model 126
(`0x706E5`), tetap 10 core / 12 thread, XCPM aktif, frekuensi 400–4800 MHz, dan IntelHybridCores tetap jalan.

## Hasil tes

Semua tes memakai sleep 5–10 menit dengan baterai, bangun lewat alarm RTC atau tombol power.

| CPUID | Jumlah tes | Bangun OK | S0ix (SLP_S0) | Package C-state | Core C-state |
| --- | --- | --- | --- | --- | --- |
| Comet Lake | 5 | 5/5 | 0 % | PC6 99 % | C6 99 %, CC7 0 % |
| **Ice Lake** | 1 | 1/1 | **97,2 %** | **PC10 97 %** | **CC7 99 %**, C10 97 % |

Tidak ada panic, GPU restart, atau hang. Metal dan resolusi 1920×1080 tetap jalan setelah bangun. Konsumsi daya selama
sleep belum terukur dengan baik, karena gauge baterai terlalu kasar untuk tes 5 menit.

## Belum dites / masalah

- Sleep lewat **tutup lid**, sleep berjam-jam, dan sleep saat **charger tercolok**. Di Tiger Lake, data upstream
  menunjukkan sleep saat charger tercolok kadang hang ketika masuk sleep.
- Wi-Fi baru tersambung ±1 menit setelah bangun ([MASALAH.md](MASALAH.md#wi-fi-baru-tersambung-1-menit-setelah-bangun-dari-sleep)).
- Daftar IP yang masih menyala saat halt tetap menyebut ISH dan CNVi, walaupun SLP_S0 tercapai.

## Cara mencoba

```sh
sudo pmset -a disablesleep 0      # izinkan sleep
pmset sleepnow                    # atau: sudo pmset relative wake 300 && pmset sleepnow
sudo pmset -a disablesleep 1      # matikan lagi
```

Hasil sleep terakhir bisa dilihat dengan `ioreg -rd1 -c S0ixStandby -w0` (tipe sleep, sumber bangun, blocker), atau
dengan skrip `macos/s0ix-status.sh` dari repo S0ixStandby (residency S0ix, PC10, dan core C-state).
Kalau setelah 8 menit layar tetap mati, tekan Caps Lock: kalau LED-nya menyala, sistem masih hidup, jadi tunggu
beberapa menit sebelum menahan tombol power.
