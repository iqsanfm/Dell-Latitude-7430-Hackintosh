/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20200925 (64-bit version)
 * Copyright (c) 2000 - 2020 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of SSDT-XPS-I2C.aml, Sun Nov 22 14:18:38 2026
 *
 * Original Table Header:
 *     Signature        "SSDT"
 *     Length           0x0000016A (362)
 *     Revision         0x02
 *     Checksum         0x1D
 *     OEM ID           "HACK"
 *     OEM Table ID     "XPSI2C"
 *     OEM Revision     0x00002000 (8192)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20200925 (538970405)
 */
DefinitionBlock ("", "SSDT", 2, "HACK", "XPSI2C", 0x00002000)
{
    External (_SB_.PC00.I2C0, DeviceObj)
    External (_SB_.PC00.I2C0.TPL0, DeviceObj)
    External (_SB_.PC00.I2C0.TPL0.XCRS, MethodObj)    // 0 Arguments
    External (_SB_.PC00.I2C0.TPL0.XINI, MethodObj)    // 0 Arguments
    External (_SB_.PC00.I2C1, DeviceObj)
    External (_SB_.PC00.I2C1.TPD0, DeviceObj)
    External (_SB_.PC00.I2C1.TPD0.XCRS, MethodObj)    // 0 Arguments
    External (_SB_.PC00.I2C1.TPD0.XINI, MethodObj)    // 0 Arguments

    Scope (_SB.PC00.I2C0)
    {
        Name (_ADR, 0x00150000)  // _ADR: Address
    }

    Scope (_SB.PC00.I2C1)
    {
        Name (_ADR, 0x00150001)  // _ADR: Address
    }

    Scope (_SB.PC00.I2C0.TPL0)
    {
        Method (_INI, 0, NotSerialized)  // _INI: Initialize
        {
            XINI ()
        }

        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Return (XCRS ())
        }
    }

    Scope (_SB.PC00.I2C1.TPD0)
    {
        Method (_INI, 0, NotSerialized)  // _INI: Initialize
        {
            XINI ()
        }

        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Return (XCRS ())
        }
    }
}

