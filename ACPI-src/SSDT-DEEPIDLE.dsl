/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20200925 (64-bit version)
 * Copyright (c) 2000 - 2020 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of SSDT-DEEPIDLE.aml, Sun Nov 22 14:18:38 2026
 *
 * Original Table Header:
 *     Signature        "SSDT"
 *     Length           0x00000044 (68)
 *     Revision         0x02
 *     Checksum         0x10
 *     OEM ID           "S0IX"
 *     OEM Table ID     "DEEPIDLE"
 *     OEM Revision     0x00001000 (4096)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20251212 (539300370)
 */
DefinitionBlock ("", "SSDT", 2, "S0IX", "DEEPIDLE", 0x00001000)
{
    Scope (\_SB)
    {
        Method (LPS0, 0, NotSerialized)
        {
            Return (One)
        }
    }

    Scope (\_GPE)
    {
        Method (LXEN, 0, NotSerialized)
        {
            Return (One)
        }
    }
}

