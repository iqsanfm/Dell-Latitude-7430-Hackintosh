# Kext

Urutan di bawah sama dengan urutan `Kernel > Add` di `config.plist`. Urutan ini penting: Lilu dan VirtualSMC harus
dimuat sebelum plugin-nya.

## Daftar

| Kext | Versi | Fungsi | Sumber |
| --- | --- | --- | --- |
| Lilu | 1.7.3 | Fondasi semua plugin | [acidanthera/Lilu](https://github.com/acidanthera/Lilu) |
| AMFIAnchor | 0.1.2 | AMFI tetap aktif untuk WhateverXe Graphics | [b00t0x/AMFIAnchor](https://github.com/b00t0x/AMFIAnchor) |
| AMFIAnchorInjector | 1.0.0 | Sertifikat WhateverXe-Graphics | [b00t0x/AMFIAnchor](https://github.com/b00t0x/AMFIAnchor) |
| WhateverXe | 0.8.3 | Akselerasi Iris Xe (Alder Lake-P) | [b00t0x/WhateverXe](https://github.com/b00t0x/WhateverXe) |
| IntelBluetoothFirmware | 2.5.1 | Firmware Bluetooth AX211 | [OpenIntelWireless/IntelBluetoothFirmware](https://github.com/OpenIntelWireless/IntelBluetoothFirmware) |
| BlueToolFixup | 2.7.3 | Bluetooth pihak ketiga di macOS 12+ | [acidanthera/BrcmPatchRAM](https://github.com/acidanthera/BrcmPatchRAM) |
| BTLEAdvFixup | 0.9.3 | BLE advertising (AirDrop, Handoff) | [retropineaple/ForbiddenItlwm](https://github.com/retropineaple/ForbiddenItlwm) |
| VirtualSMC | 1.3.8 | Emulasi SMC | [acidanthera/VirtualSMC](https://github.com/acidanthera/VirtualSMC) |
| BrightnessKeys | 1.0.3 | Tombol brightness | [acidanthera/BrightnessKeys](https://github.com/acidanthera/BrightnessKeys) |
| RestrictEvents | 1.1.7 | Nama CPU di "About This Mac" + patch kecil | [acidanthera/RestrictEvents](https://github.com/acidanthera/RestrictEvents) |
| SMCBatteryManager | 1.3.8 | Baterai | VirtualSMC |
| SMCProcessor | 1.3.8 | Sensor suhu CPU | VirtualSMC |
| **SMCDellDDV** | 1.0.8 | RPM kipas + sensor suhu Dell | **Buatan sendiri**, source di [`src/SMCDellDDV`](../src/SMCDellDDV) |
| VoodooGPIO | 1.2 | GPIO Tiger/Alder Lake-LP | [VoodooI2C/VoodooGPIO](https://github.com/VoodooI2C/VoodooGPIO) + **patch** |
| VoodooI2CServices | 1.0 | Layanan VoodooI2C | [VoodooI2C/VoodooI2C](https://github.com/VoodooI2C/VoodooI2C) |
| VoodooInput | 1.1.7 | Multitouch | [acidanthera/VoodooInput](https://github.com/acidanthera/VoodooInput) + **patch** |
| VoodooI2C | 2.9.1 | Bus I2C | [VoodooI2C/VoodooI2C](https://github.com/VoodooI2C/VoodooI2C) |
| VoodooRMI + RMII2C | 1.4.3 | Trackpad Synaptics RMI lewat I2C | [VoodooSMBus/VoodooRMI](https://github.com/VoodooSMBus/VoodooRMI) |
| VoodooI2CHID | 1.0 | Perangkat I2C HID | VoodooI2C |
| VoodooPS2Controller (+ Keyboard, Mouse, Trackpad) | 2.3.8 | Keyboard PS/2 | [acidanthera/VoodooPS2](https://github.com/acidanthera/VoodooPS2) + **patch** |
| XHCI-unsupported | 0.9.2 | Kontroler USB seri 600 | [RehabMan/OS-X-USB-Inject-All](https://github.com/RehabMan/OS-X-USB-Inject-All) |
| USBToolBox + UTBDefault | 1.2.0 / 1.0 | Peta USB | [USBToolBox/kext](https://github.com/USBToolBox/kext) |
| ForbiddenItlwm | 0.6.0 | Wi-Fi Intel AX211 + AirDrop (AWDL) | [retropineaple/ForbiddenItlwm](https://github.com/retropineaple/ForbiddenItlwm) |
| S0ixStandby | 0.5.4 | S0ix / Deep Idle | [labrouss/S0ixStandby](https://github.com/labrouss/S0ixStandby) |
| IntelHybridCores | 0.2.0 | Scheduler P-core / E-core (lisensi MIT) | [rodgod-rr/IntelHybridCores](https://github.com/rodgod-rr/IntelHybridCores) |

Kext di luar EFI (dipasang di sistem, lihat [POST-INSTALL.md](POST-INSTALL.md)):

| Kext | Versi | Fungsi |
| --- | --- | --- |
| WhateverXe Graphics (`AppleIntelTGLCompat` + driver TGL) | 0.8.1 | Driver grafis Apple untuk Iris Xe, dari paket `.pkg` WhateverXe-Graphics |
| VoodooHDA | 3.6.5 | Audio (speaker, headphone, mic) |

## Kext yang dimodifikasi

Binary di `EFI/OC/Kexts` dibuild dari upstream ditambah patch di folder [`patches/`](../patches).

| Kext | Patch | Basis upstream | Isi |
| --- | --- | --- | --- |
| VoodooPS2Controller 2.3.8 | [`VoodooPS2-shutfix.patch`](../patches/VoodooPS2-shutfix.patch) | `acidanthera/VoodooPS2@7eab4a3` | Saat **Restart** dari macOS, keyboard mati di BIOS/picker OpenCore. Patch ini mengembalikan state 8042 dan memaksa *full reset* PCH (`outb 0xCF9, 0x0E`), sehingga keyboard dan brightness BIOS kembali normal |
| VoodooInput 1.1.7 (di dalam VoodooI2C) | [`VoodooInput-jumpfix.patch`](../patches/VoodooInput-jumpfix.patch) | `acidanthera/VoodooInput@d897813` | Rotasi ID transducer + penjaga *teleport* untuk kursor yang melompat. Penyebab utama lompatan ternyata polling SMCDellDDV (sudah diperbaiki di 1.0.5+). Patch ini tetap dipasang karena tidak berefek negatif |
| VoodooGPIO 1.2 | [`VoodooGPIO-tigerlakelp-start.patch`](../patches/VoodooGPIO-tigerlakelp-start.patch) | `VoodooI2C/VoodooGPIO@717ab2a` | Perbaikan `start()` Tiger Lake-LP, supaya jalan di pin controller `INTC1055` (Alder Lake-P) |

## SMCDellDDV

Plugin VirtualSMC yang **hanya membaca**. Ia membaca RPM kipas dan sensor suhu dari metode WMI Dell "DDV"
(`\_SB.AMWV.WMDV`), lalu menampilkannya sebagai key SMC (`F0Ac`, `TA0P`, `TA1P`, `TMA1`, `TZ0C`, `TZ1C`). Kext ini
tidak pernah menulis kontrol kipas.

- Kext memakai workloop sendiri. Kalau berbagi workloop, pembacaan ACPI (85–200 ms) membuat trackpad I2C melompat.
- Polling pertama ditunda 60 detik setelah boot, supaya tidak bentrok dengan inisialisasi Wi-Fi.
- Boot-arg `-ddvoff` mematikan kext, `-ddvdbg` menyalakan log debug.

Build: `DEPS=/folder/deps zsh src/SMCDellDDV/build.sh`. Folder `DEPS` harus berisi `MacKernelSDK/`, `Lilu/`, dan `VirtualSMC/`.

## Tidak disertakan

- **BtIdle**: port pribadi yang membuat Bluetooth Intel USB bisa *suspend* saat idle (hemat sekitar 0,75 W).
  Kext ini memang tidak untuk disebar, karena sumbernya tidak berlisensi. Tanpa BtIdle, semua fitur lain tetap jalan.
- Kext yang tidak aktif di config (AppleALC, WhateverGreen, NootedGreen, itlwm, dan lain-lain) sudah dihapus dari EFI.
