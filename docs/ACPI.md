# ACPI

Source `.dsl` untuk setiap SSDT ada di [`ACPI-src/`](../ACPI-src), hasil dekompilasi dari `.aml` di EFI.

## SSDT (`ACPI > Add`)

| File | Fungsi |
| --- | --- |
| `MaLd0n.aml` | DSDT patch dari Olarila untuk Latitude 7430. Menggantikan DSDT asli secara utuh, jadi **hanya cocok untuk BIOS dengan DSDT yang sama** (dites di BIOS 1.41.0) |
| `SSDT-PLUG-ALT.aml` | Membuat objek `Processor` untuk CPU yang di BIOS dideklarasikan sebagai `ACPI0007`, supaya XCPM berjalan |
| `SSDT-USB-Reset.aml` | Mematikan `RHUB` di Darwin (`_STA = 0`), supaya port diatur oleh USBToolBox |
| `SSDT-GPI0.aml` | Mengaktifkan controller GPIO `GPI0` untuk VoodooI2C. Butuh rename `_STA → XSTA` |
| `SSDT-XOSI.aml` | Emulasi `_OSI("Windows 20xx")` di Darwin, supaya fitur seperti I2C dan trackpad aktif |
| `SSDT-XPS-I2C.aml` | Mendefinisikan ulang `I2C0`/`I2C1` dan trackpad `TPD0`/`TPL0` untuk VoodooI2C |
| `SSDT-DEEPIDLE.aml` | `LPS0` dan `LXEN` mengembalikan `One`, supaya S0ixStandby bisa masuk Deep Idle |

## Patch (`ACPI > Patch`)

| Komentar | Fungsi |
| --- | --- |
| GPI0 `_STA` to `XSTA` | Untuk SSDT-GPI0 (hanya di DSDT) |
| OSID to XSID | Harus diterapkan **sebelum** rename `_OSI` |
| `_OSI` to `XOSI` | Untuk SSDT-XOSI |
| I2C0/I2C1 `_ADR` to `XADR` | Untuk SSDT-XPS-I2C |
| TPL0/TPD0 `_INI` to `XINI`, `_CRS` to `XCRS` | Untuk SSDT-XPS-I2C |

## Jalur event lid (referensi)

Kalau event lid berhenti terbaca, cek opsi BIOS **Lid Switch** lebih dulu ([BIOS.md](BIOS.md)). Jalurnya di DSDT:

```
EC _Q66 → NEVT → EV3(3) → \_SB.BTNV → GFX0.GLID (SSDT6, OpRegion IGDM) → Notify(LID0, 0x80)
```

Tidak ada patch OpenCore yang menyentuh jalur ini.
