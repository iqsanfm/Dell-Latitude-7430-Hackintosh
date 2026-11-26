# Setelah instalasi

## 1. SMBIOS

`PlatformInfo > Generic` di repo ini sengaja dikosongkan. Isi dengan data milikmu sendiri:

1. Jalankan [GenSMBIOS](https://github.com/corpnewt/GenSMBIOS) dan buat SMBIOS untuk **`MacBookPro16,2`**.
2. Isi ke `EFI/OC/config.plist`:

   | Key | Isi |
   | --- | --- |
   | `SystemSerialNumber` | Serial dari GenSMBIOS |
   | `MLB` | Board serial dari GenSMBIOS |
   | `SystemUUID` | UUID dari GenSMBIOS |
   | `ROM` | 6 byte. Paling aman pakai MAC address Wi-Fi (`en0`) tanpa titik dua |

3. Cek serialnya di [checkcoverage.apple.com](https://checkcoverage.apple.com). Hasil yang benar adalah **"tidak valid"**.
   Kalau ternyata serial itu milik Mac sungguhan, buat ulang.

> [!CAUTION]
> Jangan pernah commit atau upload `config.plist` yang sudah berisi SMBIOS-mu.

## 2. Grafis: WhateverXe Graphics

Driver grafis Apple untuk Iris Xe **tidak ada di EFI**. Driver ini dipasang sebagai paket:

1. Boot macOS memakai EFI ini. Layar sudah tampil, tapi belum ada akselerasi.
2. Unduh `.pkg` terbaru dari [b00t0x/WhateverXe-Graphics](https://github.com/b00t0x/WhateverXe-Graphics/releases), pasang, lalu restart.
3. Cek: `kmutil showloaded | grep -i TGL` harus menampilkan `AppleIntelTGLCompat`, `AppleIntelTGLGraphics`, dan `AppleIntelTGLGraphicsFramebuffer`.

## 3. Audio: VoodooHDA

AppleALC tidak dipakai. Audio memakai **VoodooHDA 3.6.5** di `/Library/Extensions`:

```sh
sudo cp -R VoodooHDA.kext /Library/Extensions/
sudo chown -R root:wheel /Library/Extensions/VoodooHDA.kext
sudo kmutil install --update-all
```

Setelah itu izinkan di **System Settings → Privacy & Security**, lalu restart.

## 4. Penjaga Wi-Fi saat boot

ForbiddenItlwm kadang gagal start firmware saat boot (bug upstream: `SSM_EVENT_FW_LOADED` tidak pernah muncul), dan
Wi-Fi baru jalan setelah di-toggle. [`tools/wifi-boot`](../tools/wifi-boot) memasang LaunchDaemon yang jalan sekali tiap
boot. Ia menunggu event itu sampai 240 detik, dan kalau tidak muncul, mematikan lalu menyalakan Wi-Fi (maksimal 2 kali).

```sh
cd tools/wifi-boot
sudo zsh pasang.sh     # pasang
sudo zsh copot.sh      # copot
```

Log ada di `/var/log/xe-wifi-boot.log`, snapshot diagnostik di `/var/log/xe-wifi-boot/`.

## 5. Power management

Sleep dimatikan secara default karena masih eksperimental ([SLEEP.md](SLEEP.md)):

```sh
sudo pmset -a disablesleep 1
sudo pmset -a hibernatemode 0 standby 0 powernap 0 tcpkeepalive 0
```

Layar tetap bisa mati sendiri (`displaysleep`) dan saat lid ditutup. Saat idle, S0ixStandby (Deep Idle) menjaga
konsumsi daya tetap rendah.

## 6. SIP

`csr-active-config` diatur ke `EF0F0000` (SIP dimatikan sebagian). Sesuaikan kalau kamu ingin SIP lebih ketat.
WhateverXe sendiri minimal membutuhkan `03080000`.
