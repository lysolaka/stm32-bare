/* Linker script for the STM32F407VG */

ENTRY(__reset__)

MEMORY
{
  CCM (xrw)  : ORIGIN = 0x10000000, LENGTH = 64K
  SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
}

/* Put the stack in Core Coupled Memory */
_sp_init_ = ORIGIN(CCM) + LENGTH(CCM);

SECTIONS
{
  .text :
  {
    . = ALIGN(4);

    KEEP(*(.vectors))

    *(.text)
    *(.rodata)

    . = ALIGN(4);
  } > FLASH


  .data :
  {
    . = ALIGN(4);
    _data_start_ = .;

    *(.data)
    *(.data*)

    . = ALIGN(4);
    _data_end_ = .;
  } > SRAM AT > FLASH

  _data_load_ = LOADADDR(.data);

  .bss :
  {
    . = ALIGN(4);
    _bss_start_ = .;

    *(.bss)
    *(.bss*)

    . = ALIGN(4);
    _bss_end_ = .;
  } > SRAM
}
