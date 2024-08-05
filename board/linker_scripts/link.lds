/*
 * linker script for STM32F4xx with GNU ld
 * bernard.xiong 2009-10-14
 * flybreak      2018-11-19  Add support for RAM2
 */

/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH =  1024k /* 1024KB  - 128kB flash */
    RAM (rw) : ORIGIN = 0x20000000, LENGTH =  128k /* 128K sram */
    CCMRAM (rw) : ORIGIN = 0x10000000, LENGTH =   64k /* 64K ccm sram */
}
ENTRY(Reset_Handler)
_system_stack_size = 0x400;

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _stext = .;
        KEEP(*(.isr_vector))            /* Startup code */

        . = ALIGN(4);
        *(.text)                        /* remaining code */
        *(.text.*)                      /* remaining code */
        *(.rodata)                      /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        /* section information for finsh shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        /* section information for initial. */
        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;

        . = ALIGN(4);

        PROVIDE(__ctors_start__ = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        PROVIDE(__ctors_end__ = .);

        /* section information for utest */
        . = ALIGN(4);
        __rt_utest_tc_tab_start = .;
        KEEP(*(UtestTcTab))
        __rt_utest_tc_tab_end = .;

        . = ALIGN(4);

        _etext = .;
    } > FLASH = 0

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)

        /* This is used by the startup in order to initialize the .data secion */
        _sidata = .;
    } > FLASH
    __exidx_end = .;

    /* used by the startup to initialize data */
    _sidata = LOADADDR(.data);
    /* .data section which is used for initialized data */
    .data :
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)

        PROVIDE(__dtors_start__ = .);
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _edata = . ;
    } >RAM  AT> FLASH

    _siccmdata = LOADADDR(.ccmdata); /* May be already present */

    /* Initialized CCM-RAM section 
    * 
    * IMPORTANT NOTE! 
    * If initialized variables will be placed in this section, 
    * the startup code needs to be modified to copy the init-values.  
    */
    .ccmdata : 
    {
        . = ALIGN(4);
        _sccmdata = .;        /* create a global symbol at data start */
        *(.ccmdata)           /* .data sections */
        *(.ccmdata*)          /* .data* sections */
/*
        *data_reg.o(.data .data*)
        *monitor*.o(.data .data*)
        *sender*.o(.data .data*)
        *kernel*.o(.data .data*)
*/
        . = ALIGN(4);
        _eccmdata = .;        /* define a global symbol at data end */
    } >CCMRAM AT> FLASH


    /* _siccm_bss = LOADADDR(.ccm_bss); */
    /* CCM-RAM section
    *
    * IMPORTANT NOTE!
    * If initialized variables will be placed in this section,
    * the startup code needs to be modified to copy the init-values.
    */
    .ccm_bss :
    {
        . = ALIGN(4);
        _sccm_bss = .;       /* create a global symbol at ccmram start */
        *(.ccm_bss)
        *(.ccm_bss*)

        /* build\\core\\*.o(.bss .bss*) */
        build\\monitor\\**.o(.bss .bss*)
        build\\sender\\**.o(.bss .bss*)
        /* build\\mapping\\**.o(.bss .bss*) */

        /* *board*.o(.bss .bss*) */
        /* *autosave.o(.bss .bss*)*/
        /* *application*autosave.o(.bss .bss*) */
        /* *data_send.o(.bss .bss*)  */
        /* *isee_http.o(.bss .bss*)  */
        /* *config.o(.bss .bss*) */
        /* *dio.o(.bss .bss*) */
        /* *eth.o(.bss .bss*) */
        /* *led.o(.bss .bss*) */
 
        /* *packages*agile_telnet*.o(.bss .bss*)  */
        /* *packages*agile_console*.o(.bss .bss*)  */
        /* *packages*cJSON*.o(.bss .bss*)   */
        /*  *packages*CmBacktrace*.o(.bss .bss*)*/
        /*   *packages*easyblink*.o(.bss .bss*) */
        /*  *packages*webclient*.o(.bss .bss*) */
        /*  *packages*netutils*.o(.bss .bss*)  */
        /*  *packages*syswatch*.o(.bss .bss*)   */
        /*  *packages*UrlEncode*.o(.bss .bss*) */

        build\\kernel\\src\\*.o(.bss .bss*)
        /* build\\kernel\\libcpu\\*.o(.bss .bss*) */
        /* build\\kernel\\components\\libc\\*.o(.bss .bss*)
        /* build\\kernel\\components\\drivers\\*.o(.bss .bss*) */
        /* build\\kernel\\components\\dfs\\*.o(.bss .bss*) */
        build\\kernel\\components\\net\\lwip\\lwip-2.0.3\\*.o(.bss .bss*)
        /* build\\kernel\\components\\net\\lwip\\port\\**.o(.bss .bss*) */
        /* build\\kernel\\components\\net\\sal\\*.o(.bss .bss*) */
        /* build\\kernel\\components\\net\\netdev\\*.o(.bss .bss*) */
        /* build\\kernel\\components\\net\\sal_socket\\*.o(.bss .bss*) */

        . = ALIGN(4);
        _eccm_bss = .;       /* create a global symbol at ccmram end */
    } >CCMRAM

    .stack :
    {
        . = ALIGN(4);
        _sstack = .;
        . = . + _system_stack_size;
        . = ALIGN(4);
        _estack = .;
    } >RAM

    __bss_start = .;
    .bss :
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _ebss = . ;

        *(.bss.init)
    } > RAM
    __bss_end = .;

    _end = .;

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
