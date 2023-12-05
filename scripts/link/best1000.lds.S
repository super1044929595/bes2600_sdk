/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "plat_addr_map.h"

#ifdef RB_CODEC
#define HEAP_SECTION_SIZE			0x1000
#else
#define HEAP_SECTION_SIZE			0x1000
#endif

#if defined(__HOST_GEN_ECDH_KEY__)
#define STACK_SECTION_SIZE			0x2000
#else
#define STACK_SECTION_SIZE			0x1000
#endif
#define CP_STACK_SECTION_SIZE		0x1000

#if defined(ROM_UTILS_ON)
#if defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define ROM_UTILS_RESV_RAM_SIZE     0x64
#else
#define ROM_UTILS_RESV_RAM_SIZE     0x0
#endif
#endif

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
#define OVERLAY_DATA_SECTION_SIZE	0x8000
#elif defined(RB_CODEC)
#define OVERLAY_DATA_SECTION_SIZE	0xA000
#else
#define OVERLAY_DATA_SECTION_SIZE	0x8000
#endif


#if (FAST_XRAM_SECTION_SIZE & (32 - 1))
#error "FAST_XRAM_SECTION_SIZE should align with 32 bytes"
#endif

/* TODO: Add FLASH_REGION_OFFSET if there are multiple levels of boot loaders */
#if !defined(OTA_BOOT_SIZE) && defined(OTA_CODE_OFFSET)
#define FLASH_REGION_BASE			(FLASH_BASE + OTA_CODE_OFFSET)
#else
#ifdef OTA_BOOT_OFFSET
#define FLASH_REGION_BASE			(FLASH_BASE + OTA_BOOT_OFFSET)
#else
#define FLASH_REGION_BASE			(FLASH_BASE)
#endif
#endif

#ifndef FLASH_REGION_SIZE
#define FLASH_REGION_SIZE			(FLASH_SIZE - (FLASH_REGION_BASE - FLASH_BASE))
#endif

#if (FLASH_REGION_BASE + FLASH_REGION_SIZE) > (FLASH_BASE + FLASH_SIZE)
#error "(FLASH_REGION_BASE + FLASH_REGION_SIZE) should <= (FLASH_BASE + FLASH_SIZE)"
#endif

#define FLASH_NC_REGION_BASE		FLASH_C_TO_NC(FLASH_REGION_BASE)
#define FLASHX_REGION_BASE			FLASH_TO_FLASHX(FLASH_REGION_BASE)

#define RESERVED_SECTION_SIZE		0x1000

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
#define OVERLAY_FOR_CP
#endif

#ifdef OVERLAY_FOR_CP
#define REGION_OVERLAY_TEXT			RAMCPX
#define REGION_OVERLAY_DATA			RAMCP
#else
#define REGION_OVERLAY_TEXT			FRAMX
#define REGION_OVERLAY_DATA			RAM
#endif

/* Linker script to configure memory regions. */
/* See plat_addr_map.h and common.mk for the macro definitions */
MEMORY
{
	ROM			(rx)	: ORIGIN = ROM_BASE,				LENGTH = ROM_SIZE
	FLASH 		(r)		: ORIGIN = FLASH_REGION_BASE,		LENGTH = FLASH_REGION_SIZE
	FLASH_NC	(r)		: ORIGIN = FLASH_NC_REGION_BASE,	LENGTH = FLASH_REGION_SIZE
	FLASHX		(rx)	: ORIGIN = FLASHX_REGION_BASE,		LENGTH = FLASH_REGION_SIZE
#if defined(UNIFY_HEAP_ENABLED)
	RAM			(rwx)	: ORIGIN = RAM_BASE,				LENGTH = RAM_SIZE - FAST_XRAM_SECTION_SIZE
	RAMX		(rx)	: ORIGIN = RAMX_BASE,				LENGTH = RAM_SIZE - FAST_XRAM_SECTION_SIZE
	FRAMX		(rwx)	: ORIGIN = RAMX_BASE + RAM_SIZE - FAST_XRAM_SECTION_SIZE,	LENGTH = FAST_XRAM_SECTION_SIZE
#else
	RAM			(rwx)	: ORIGIN = RAM_BASE,				LENGTH = RAM_SIZE
	RAMX		(rx)	: ORIGIN = RAMX_BASE,				LENGTH = RAM_SIZE
	FRAMX		(rwx)	: ORIGIN = FRAMX_BASE,				LENGTH = FAST_XRAM_SECTION_SIZE
#endif
#ifdef PSRAM_BASE
	PSRAM 		(rwx)	: ORIGIN = PSRAM_BASE,				LENGTH = PSRAM_SIZE
	PSRAM_NC	(rwx)	: ORIGIN = PSRAM_NC_BASE,			LENGTH = PSRAM_SIZE
#endif
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
	RAMCP 		(rwx)	: ORIGIN = RAMCP_BASE,				LENGTH = RAMCP_SIZE
	RAMCPX 		(rwx)	: ORIGIN = RAMCPX_BASE,				LENGTH = RAMCPX_SIZE
#endif
}

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __export_fn_rom
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __free_ram
 *   __factory_start
 *   __factory_end
 *   __calib_start
 *   __calib_end
 *   __flash_start
 *   __flash_end
 *   __free_flash
 *   __boot_sram_start_flash__
 *   __boot_sram_end_flash__
 *   __boot_sram_start__
 *   __boot_bss_sram_start__
 *   __boot_bss_sram_end__
 *   __sram_text_data_start_flash__
 *   __sram_text_data_end_flash__
 *   __fast_sram_text_data_start__
 *   __fast_sram_text_data_end__
 *   __fast_sram_text_data_start_flash__
 *   __fast_sram_text_data_end_flash__
 *   __sram_text_data_start__
 *   __sram_bss_start__
 *   __sram_bss_end__
 *   __overlay_text_start__
 *   __overlay_text_exec_start__
 *   __overlay_data_start__
 */
ENTRY(Boot_Loader)

SECTIONS
{
	__export_fn_rom = (ORIGIN(ROM) + LENGTH(ROM) - ROM_BUILD_INFO_SECTION_SIZE - ROM_EXPORT_FN_SECTION_SIZE);

	.boot_struct (ORIGIN(FLASH)) :
	{
		__flash_start = .;
		KEEP(*(.boot_struct))
		. = ALIGN(4);
	} > FLASH

	. = FLASH_TO_FLASHX(.);

	.boot_text_flash (.) : AT (FLASHX_TO_FLASH(ADDR(.boot_text_flash)))
	{
		*(.boot_loader)
		*(.boot_text_flash*)
		*(.boot_rodata_flash*)
		. = ALIGN(4);
	} > FLASHX

#if defined(__BES_OTA_MODE__)
	.ota_boot_info (ALIGN(4096)) :
	{
		*(.ota_boot_info)
		. = (0x1000);
	} > FLASHX

/*	.ota_boot_rev (ALIGN(4096)) :
	{
		*(.ota_boot_rev)
		. = (0x1000);
	} > FLASHX */
#endif

	. = FLASHX_TO_FLASH(.);

	.got_info (.) :
	{
		__got_info_start = .;
		__got_start = .;
		*(.got)
		. = ALIGN(4);
		__got_end = .;
		__got_plt_start = .;
		*(.got.plt)
		. = ALIGN(4);
		__igot_plt_start = .;
		*(.igot.plt)
		. = ALIGN(4);
		__dynamic_start = .;
		*(.dynamic)
		. = ALIGN(4);
		__got_info_end = .;
	} > FLASH

	.vector_table (ORIGIN(RAM)) (NOLOAD) :
	{
		KEEP(*(.vector_table))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > RAM

	.reboot_param (.) (NOLOAD) :
	{
		KEEP(*(.reboot_param))
		. = REBOOT_PARAM_SECTION_SIZE;
		. = ALIGN(4);
	} > RAM

	.userdata_pool (.) (NOLOAD) :
	{
		*(.userdata_pool)
		. = ALIGN(4);
		__userdata_pool_end__ = .;
	} > RAM

	__boot_sram_start_flash__ = LOADADDR(.got_info) + SIZEOF(.got_info);
	__boot_sram_start__ = __userdata_pool_end__;

	.boot_text_sram (RAM_TO_RAMX(__boot_sram_start__)) : AT (__boot_sram_start_flash__)
	{
		_stext = .;
		/* memcpy.o or libc_nano.a(lib_a-memcpy.o/lib_a-memcpy-stub.o) */
		*:memcpy*.o(.text*)
		*:lib_a-memcpy*.o(.text*)
		*:libc_rom.o(.text*)
		*:libc_rom_mem.o(.text*)

		/* memset.o or libc_nano.a(lib_a-memset.o) */
		*:memset*.o(.text*)
		*:lib_a-memset*.o(.text*)

		*:hal_norflash*.o(.text*)
		*:norflash_*.o(EXCLUDE_FILE(*:norflash_api.o) .text*)

#ifdef BOOT_CODE_IN_RAM
		*:hal_cmu_*.o(.text*)
		*:stack_protector*.o(.text*)
		*:hal_iomux_*.o(.text*)
		*:hal_analogif_*.o(.text*)
		*:hal_spi.o(.text*)
		*:hal_timer.o(.text*)
		*:hal_sysfreq.o(.text*)
#endif

		*(.boot_text_sram*)
		. = ALIGN(4);
		__boot_text_sram_end__ = .;
	} > RAMX

	.boot_data_sram (RAMX_TO_RAM(ADDR(.boot_text_sram) + SIZEOF(.boot_text_sram))) :
					AT (__boot_sram_start_flash__ + SIZEOF(.boot_text_sram))
	{
		__boot_data_sram_start__ = .;
		*:hal_norflash*.o(.data* .rodata*)
		*:norflash_*.o(EXCLUDE_FILE(*:norflash_api.o) .data* EXCLUDE_FILE(*:norflash_api.o) .rodata*)
		*:hal_psram.o(.data* .rodata*)
		*:libc_rom.o(.rodata*)
		*:libc_rom_mem.o(.rodata*)

#ifdef BOOT_CODE_IN_RAM		
		*:stack_protector*.o(.data* .rodata*)
#endif
		/* Stack protector instrumentation rodata for functions in special sections */
		*(.rodata.cst4)

		*(.boot_rodata_sram*)
		*(.boot_data*)
		. = ALIGN(4);
		__boot_data_sram_end__ = .;
	} > RAM

	__boot_sram_end_flash__ = __boot_sram_start_flash__ +
								SIZEOF(.boot_text_sram) + SIZEOF(.boot_data_sram);

	.boot_bss_sram (.) (NOLOAD) : AT (ADDR(.boot_bss_sram))
	{
		__boot_bss_sram_start__ = .;
		*:hal_norflash*.o(.bss*)
		*:norflash_*.o(EXCLUDE_FILE(*:norflash_api.o) .bss*)
#ifdef BOOT_CODE_IN_RAM		
		*:stack_protector*.o(.bss*)
#endif
		*:hal_psram.o(.bss*)
		*(.boot_bss*)
		. = ALIGN(32);
		__boot_bss_sram_end__ = .;
	} > RAM

	__sram_text_data_start_flash__ = __boot_sram_end_flash__;
	__sram_text_data_start__ = __boot_bss_sram_end__;

	.sram_text (RAM_TO_RAMX(__sram_text_data_start__)) : AT (__sram_text_data_start_flash__)
	{
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
		*:hal_memsc.o(.text*)
		*:hal_trace.o(.text*)
		*:hal_trace_mod.o(.text*)
		*:mpu_armv*.o(.text*)
		*:codec_sbc.o(.text*)
		*:multi_heap.o(.text*)
		*:pool_api.o(.text*)
		*:_arm_addsubdf3.o(.text*)
		*:_arm_cmpdf2.o(.text*)
		*:_arm_fixdfsi.o(.text*)
		*:_arm_truncdfsf2.o(.text*)
		*:_aeabi_uldivmod.o(.text*)
		*:_udivmoddi4.o(.text*)
		*:arm_fully_connected_s8.o(.text*)
		*:floor_div.o(.text*)
		*:lib_a-sf_finite.o(.text*)
		*:lib_a-kf_sin.o(.text*)
		*:lib_a-sf_expm1.o(.text*)
#if defined(SPEECH_TX_NS7)
		*:lib_a-wf_sqrt.o(.text*)
		*:lib_a-ef_sqrt.o(.text*)
#endif
#if defined(SPEECH_TX_DTLN)
		*:lib_a-s_matherr.o(.text*)
		*:lib_a-sf_fabs.o(.text*)
		*:lib_a-sf_tanh.o(.text*)
		*:lib_a-ef_exp.o(.text*)
		*:lib_a-wf_exp.o(.text*)
		*:lib_a-ef_log.o(.text*)
		*:lib_a-wf_log.o(.text*)
		*:lib_a-sf_cos.o(.text*)
		*:lib_a-ef_rem_pio2.o(.text*)
		*:lib_a-wf_sqrt.o(.text*)
		*:lib_a-ef_sqrt.o(.text*)
		*:lib_a-kf_cos.o(.text*)
#endif
		/* for LIBC_ROM=0 */
#ifndef MTEST_IN_FLASH
		*libc_nano.a:(.text*)
#endif
		/* for NOSTD=1  */
		*:atoi.o(.text*)
		*:ctype.o(.text*)
		*:memchr.o(.text*)
		*:memcmp.o(.text*)
		*:memmove*.o(.text*)
		*:strcat.o(.text*)
		*:strchr.o(.text*)
		*:strcmp.o(.text*)
		*:strcpy.o(.text*)
		*:strlen.o(.text*)
		*:strncat.o(.text*)
		*:strncmp.o(.text*)
		*:strncpy.o(.text*)
		*:strnicmp.o(.text*)
		*:strstr.o(.text*)
		*:strtol.o(.text*)
		*:strtoul.o(.text*)
#ifndef MTEST_IN_FLASH
		*:vsprintf.o(.text*)
#endif
		*:vsscanf.o(.text*)
#endif
		*:norflash_api.o(.text*)
		*(.sram_text*)
		/*
		   If you want protect one section by armv8's mpu, the section should
		   be aligned to 32bytes
		 */
		. = ALIGN(32);
		__sram_text_end__ = .;
	} > RAMX

	.sram_data (RAMX_TO_RAM(ADDR(.sram_text) + SIZEOF(.sram_text))) :
				AT (__sram_text_data_start_flash__ + SIZEOF(.sram_text))
	{
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
		*:hal_memsc.o(.data* .rodata*)
		*:hal_trace.o(.data* .rodata*)
		*:hal_trace_mod.o(.data* .rodata*)
		*:mpu_armv*.o(.data* .rodata*)
		*:multi_heap.o(.data* .rodata*)
		*:pool_api.o(.data* .rodata*)
		*:arm_fully_connected_s8.o(.data* .rodata*)
		*:floor_div.o(.data* .rodata*)
		*:lib_a-sf_finite.o(.data* .rodata*)
		*:lib_a-kf_sin.o(.data* .rodata*)
		*:lib_a-sf_expm1.o(.data* .rodata*)
#if defined(SPEECH_TX_2MIC_NS7)
		*:lib_a-wf_log10.o(.data* .rodata*)
		*:lib_a-ef_log10.o(.data* .rodata*)
		*:lib_a-ef_log.o(.data* .rodata*)
		*:lib_a-wf_pow.o(.data* .rodata*)
		*:lib_a-ef_pow.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_NS7)
		*:lib_a-wf_sqrt.o(.data* .rodata*)
		*:lib_a-ef_sqrt.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_DTLN)
		*:lib_a-s_matherr.o(.data* .rodata*)
		*:lib_a-sf_fabs.o(.data* .rodata*)
		*:lib_a-sf_tanh.o(.data* .rodata*)
		*:lib_a-ef_exp.o(.data* .rodata*)
		*:lib_a-wf_exp.o(.data* .rodata*)
		*:lib_a-ef_log.o(.data* .rodata*)
		*:lib_a-wf_log.o(.data* .rodata*)
		*:lib_a-sf_cos.o(.data* .rodata*)
		*:lib_a-ef_rem_pio2.o(.data* .rodata*)
		*:lib_a-wf_sqrt.o(.data* .rodata*)
		*:lib_a-ef_sqrt.o(.data* .rodata*)
		*:lib_a-kf_cos.o(.data* .rodata*)
#endif
		/* for LIBC_ROM=0 */
#ifndef MTEST_IN_FLASH
		*libc_nano.a:(.rodata*)
#endif
		/* for NOSTD=1  */
		*:atoi.o(.rodata*)
		*:ctype.o(.rodata*)
		*:memchr.o(.rodata*)
		*:memcmp.o(.rodata*)
		*:memmove*.o(.rodata*)
		*:strcat.o(.rodata*)
		*:strchr.o(.rodata*)
		*:strcmp.o(.rodata*)
		*:strcpy.o(.rodata*)
		*:strlen.o(.rodata*)
		*:strncat.o(.rodata*)
		*:strncmp.o(.rodata*)
		*:strncpy.o(.rodata*)
		*:strnicmp.o(.rodata*)
		*:strstr.o(.rodata*)
		*:strtol.o(.rodata*)
		*:strtoul.o(.rodata*)
#ifndef MTEST_IN_FLASH
		*:vsprintf.o(.rodata*)
#endif
		*:vsscanf.o(.rodata*)
#endif
		*:norflash_api.o(.rodata*)
#if defined(ROM_UTILS_ON)
		*:rom_utils.o(.rodata*)
#endif
#if defined(SCO_CP_ACCEL)
#if defined(_CVSD_BYPASS_) || defined(CVSD_BYPASS)
		*:Pcm8k_Cvsd.o(.data* .rodata*)
		*:cvsd_codec.o(.data* .rodata*)
#endif
		*:table.o(.data* .rodata*)
		/* Move algos from overlay to here */
#endif
		/* for SCO_CP_ACCEL=1 */
		*:cmsis_dsp_lib.o(.rodata.armBitRevIndexTable256)
		*:cmsis_dsp_lib.o(.rodata.twiddleCoef_256)
		*:cmsis_dsp_lib.o(.rodata.twiddleCoef_rfft_512)
		*:cmsis_dsp_lib.o(.rodata.armBitRevIndexTable128)
		*:cmsis_dsp_lib.o(.rodata.twiddleCoef_128)
		*:cmsis_dsp_lib.o(.rodata.twiddleCoef_rfft_256)
		*:cmsis_dsp_lib.o(.rodata.sinTable_f32)
		*:utils.o(.rodata*)
		*:cqueue.o(.rodata*)
		*(.sram_data*)
		. = ALIGN(4);
	} > RAM

	__sram_text_data_end_flash__ = __sram_text_data_start_flash__ +
								SIZEOF(.sram_text) + SIZEOF(.sram_data);

	.sram_bss (.) (NOLOAD) : AT (ADDR(.sram_bss))
	{
		__sram_bss_start__ = .;
		*:utils.o(.bss*)
		*(.sram_bss*)
		. = ALIGN(4);
		__sram_bss_end__ = .;
	} > RAM

	__fast_sram_text_data_start_flash__ = __sram_text_data_end_flash__;

	.fast_text_sram (ORIGIN(FRAMX)) : AT (__fast_sram_text_data_start_flash__)
	{
		__fast_sram_text_exec_start__ = .;
#if defined(SCO_CP_ACCEL)
		*:frame_resize.o(.text*)
		*:buffer_manager.o(.text*)
#if defined(_CVSD_BYPASS_) || defined(CVSD_BYPASS)
		*:Pcm8k_Cvsd.o(.text*)
		*:cvsd_codec.o(.text*)
#endif
		*:lpc_plc.o(.text*)
#endif
#if defined(ROM_UTILS_ON)
		*:rom_utils.o(.text*)
#endif
		*:cmsis_dsp_lib.o(.text*)
		/* memmove.o or libc_nano.a(lib_a-memmove.o) */
		*:memmove*.o(.text*)
		*:lib_a-memmove.o(.text*)
		*:utils.o(.text*)
		*:cqueue.o(.text*)
/*		*:audio_resample_ex.o(.text*) */

		*:crossover_filter.o(.text*)
		*:data_compressor.o(.text*)

		*:sbcplc.o(.text*)
		*:iirfilt.o(.text*)
#if defined(SPEECH_TX_2MIC_NS7)
		*:lib_a-wf_log10.o(.text*)
		*:lib_a-ef_log10.o(.text*)
		*:lib_a-ef_log.o(.text*)
		*:lib_a-wf_pow.o(.text*)
		*:lib_a-ef_pow.o(.text*)
#endif
#if defined(SPEECH_TX_NS7)
		*:lib_a-sf_fabs.o(.text*)
#endif
		*(.fast_text_sram*)
		/*
		   If you want protect one section by armv8's mpu, the section should
		   be aligned to 32bytes
		 */
		. = ALIGN(32);
		__fast_sram_text_exec_end__ = .;
	} > FRAMX

	__fast_sram_text_data_end_flash__ = __fast_sram_text_data_start_flash__ + SIZEOF(.fast_text_sram);
	__fast_sram_text_data_start__ = RAMX_TO_RAM(__fast_sram_text_exec_start__);
	__fast_sram_text_data_end__ = RAMX_TO_RAM(__fast_sram_text_exec_end__);

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
	__cp_text_sram_start_flash__ = __fast_sram_text_data_end_flash__;

	.cp_text_sram (ORIGIN(RAMCPX)) : AT (__cp_text_sram_start_flash__)
	{
		__cp_text_sram_exec_start__ = .;
		*(.cp_text_sram*)
		*:cp_queue.o(.text*)
		. = ALIGN(4);
		__cp_text_sram_exec_end__ = .;
	} > RAMCPX

	__cp_text_sram_start = RAMX_TO_RAM(__cp_text_sram_exec_start__);
	__cp_text_sram_end = RAMX_TO_RAM(__cp_text_sram_exec_end__);

	.vector_table_cp (ORIGIN(RAMCP)) (NOLOAD) :
	{
		KEEP(*(.vector_table_cp))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > RAMCP

	__cp_data_sram_start_flash__ = __cp_text_sram_start_flash__ + SIZEOF(.cp_text_sram);

	.cp_data_sram (.) : AT (__cp_data_sram_start_flash__)
	{
		__cp_data_sram_start = .;
		*(.cp_data*)
		*:cp_queue.o(.data* .rodata*)
		. = ALIGN(4);
		__cp_data_sram_end = .;
	} > RAMCP

	__cp_sram_end_flash__ = __cp_data_sram_start_flash__ + SIZEOF(.cp_data_sram);

	.cp_bss_sram (.) (NOLOAD) : AT (ADDR(.cp_bss_sram))
	{
		__cp_bss_sram_start = .;
		*(.cp_bss*)
		. = ALIGN(4);
		__cp_bss_sram_end = .;
	} > RAMCP

	__overlay_text_start_flash__ = __cp_sram_end_flash__;
#else
	__overlay_text_start_flash__ = __fast_sram_text_data_end_flash__;
#endif

#ifdef OVERLAY_FOR_CP
	__overlay_text_start__ = __cp_text_sram_end;
#else
	__overlay_text_start__ = __fast_sram_text_data_end__;
#endif
	__overlay_text_exec_start__ = RAM_TO_RAMX(__overlay_text_start__);

	OVERLAY (__overlay_text_exec_start__) : NOCROSSREFS AT (__overlay_text_start_flash__)
	{
#ifndef NO_OVERLAY
		.overlay_text0
		{
			LONG(0);
			*:smallft.o(.text*)
			*:fftwrap.o(.text*)
			*:lib_a-s_floor.o(.text*)
			*:filterbank_pflt.o(.text*)
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
			*:bt_sco_chain.o( .text.speech_tx_process)
			*:bt_sco_chain.o( .text.speech_rx_process)
			*:bt_sco_chain.o( .text._speech_tx_process_pre)
			*:bt_sco_chain.o( .text._speech_rx_process_pre)
			*:bt_sco_chain_cp.o(.text*)
#endif
#if defined(SPEECH_TX_DTLN) && defined(DTLN_CP)
			*:fully_connected_common.o(.text*)
			*:dequantize.o(.text*)
			*:logistic.o(.text*)
			*:kernel_util.o(.text*)
			*:tanh.o(.text*)
			*:sub_common.o(.text*)
			*:strided_slice.o(.text*)
			*:quantize_common.o(.text*)
			*:quantize.o(.text*)
			*:logistic_common.o(.text*)
			*:add_common.o(.text*)
			*:add.o(.text*)
			*:reduce.o(.text*)
			*:unpack.o(.text*)
			*:squared_difference.o(.text*)
			*:mul_common.o(.text*)
			*:reshape.o(.text*)
			*:elementwise.o(.text*)
			*:split.o(.text*)
			*:sub.o(.text*)
			*:elementwise.o(.text*)
			*:common.o(.text*)
			*:arm_cmplx_mag_f32.o(.text*)
			*:error_reporter.o(.text*)
			*:flatbuffer_conversions.o(.text*)
			*:op_resolver.o(.text*)
			*:all_ops_resolver.o(.text*)
			*:flatbuffer_utils.o(.text*)
			/*:greedy_memory_planner.o(.text*)*/
			*:memory_helpers.o(.text*)
			*:micro_allocator.o(.text*)
			*:micro_error_reporter.o(.text*)
			*:micro_graph.o(.text*)
			*:micro_error_reporter.o(.text*)
			*:micro_interpreter.o(.text*)
			*:micro_utils.o(.text*)
			*:simple_memory_allocator.o(.text*)
			*:schema_utils.o(.text*)
			*:kernel_util.o(.text*)
			*:quantization_util.o(.text*)
			*:dtln_denoise.o(.text*)
			*:fully_connected.o(.text*)
			*:dequantize_common.o(.text*)
			*:pack.o(.text*)
			*:maximum_minimum.o(.text*)
			*:mul.o(.text*)
			*:debug_log.o(.text*)
			*:micro_string.o(.text*)
			*:cmsis_fftwrap.o(.text*)
			*:arm_fully_connected_q7.o(.text*)
			*:arm_q7_to_q15_reordered_no_shift.o(.text*)
			*:activations_common.o(.text*)

			*:resize_nearest_neighbor.o(.text*)
			*:transpose_conv.o(.text*)
			*:maximum_minimum.o(.text*)

			/*:conv.o(.text*)*/

			*:rnn.o(.text*)
			*:bes_fully_connected_mat_q7_vec_f32.o(.text.bes_fully_connected_mat_q7_vec_f32)
			*:arm_fully_connected_q7.o(.text.arm_fully_connected_q7_bes_q31)
			*:arm_q7_to_q15_reordered_no_shift.o(.text.arm_q7_to_q15_reordered_no_shift)
#if defined(SPEECH_TX_NS4)
			*:speech_ns4.o(.text*)
			*:speech_ns4_core.o(.text*)
#endif
#else
#if defined(SPEECH_TX_NS5)
			*:ns5_pitch.o(.text*)
			*:ns5_celt_lpc.o(.text*)
			*:rnn.o(.text*)
			*:rnn_data_small.o(.text*)
			*:rnn_data_large.o(.text*)
			*:speech_ns5_nn.o(.text*)

			*:nsx_core_freq.o(.text*)
			*:copy_set_operations.o(.text*)
			*:division_operations.o(.text*)
			*:energy.o(.text*)
			*:get_scaling_square.o(.text*)
			*:spl_sqrt_floor.o(.text*)

			*:echo_suppression_core.o(.text*)
			*:integer_delay.o(.text*)
			*:coherence.o(.text*)
			*:logmmse.o(.text*)
			*:lcmmse.o(.text*)
			*:mdf2.o(.text*)
			*:noise_generator.o(.text*)
			*:pink_noise_gen.o(.text*)
			*:white_noise_gen.o(.text*)
#endif
#if defined(SPEECH_TX_AEC2FLOAT) && !defined(AUDIO_ANC_FB_ADJ_MC) && !defined(AUDIO_ADJ_EQ)
			*:coherence.o(.text*)
			*:echo_canceller.o(.text*)
			*:echo_suppression.o(.text*)
			*:lcmmse.o(.text*)
			*:logmmse.o(.text*)
			*:mdf2.o(.text*)
			*:noise_generator.o(.text*)
			*:pink_noise_gen.o(.text*)
			*:white_noise_gen.o(.text*)
#endif
#if defined(SPEECH_TX_NS4)
			*:speech_ns4.o(.text*)
			*:speech_ns4_core.o(.text*)
#endif
#if defined(SPEECH_TX_NS7)
			*:base_layer.o(.text*)
			*:logmmse.o(.text*)
			*:lcmmse.o(.text*)
			*:besnet_utils.o(.text*)
			*:dw_conv_layer.o(.text*)
			*:dw_deconv_layer.o(.text*)
			*:fully_connected_layer.o(.text*)
			*:gru_layer.o(.text*)
			*:output_layer.o(.text*)
			*:speech_ns7.o(.text*)
			*:speech_win.o(.text*)
			*:filterbank_pflt.o(.text*)
			*:arm_depthwise_separable_conv2x4_q15_q7_q31.o(.text*)
			*:arm_convolve_1x1_HWC_q15_q7_q31.o(.text*)
			*:arm_depthwise_transposed_conv2x4_q15_q7.o(.text*)
			*:arm_fully_connected_q15_q7_q31.o(.text*)
			*:arm_mat_trans_q15.o(.text*)
			*:arm_mat_inverse_f64.o(.text*)
			*:lib_a-sf_frexp.o(.text*)
			*:lib_a-wf_exp.o(.text*)
			*:lib_a-ef_exp.o(.text*)
			*:lib_a-sf_tanh.o(.text*)
#endif
#if defined(SPEECH_TX_2MIC_NS7)
			*:integer_delay.o(.text*)
			*:noise_suppression.o(.text*)
			*:ns_core.o(.text*)
			*:speech_2mic_ns7.o(.text*)
			*:spectral_wdrc.o(.text*)
			*:base_layer.o(.text*)
			*:logmmse.o(.text*)
			*:lcmmse.o(.text*)
			*:besnet_utils.o(.text*)
			*:dw_conv_layer.o(.text*)
			*:dw_deconv_layer.o(.text*)
			*:fully_connected_layer.o(.text*)
			*:gru_layer.o(.text*)
			*:output_layer.o(.text*)
			*:speech_ns7.o(.text*)
			*:speech_win.o(.text*)
			*:filterbank_pflt.o(.text*)
			*:arm_depthwise_separable_conv2x4_q15_q7_q31.o(.text*)
			*:arm_convolve_1x1_HWC_q15_q7_q31.o(.text*)
			*:arm_depthwise_transposed_conv2x4_q15_q7.o(.text*)
			*:arm_fully_connected_q15_q7_q31.o(.text*)
			*:arm_mat_trans_q15.o(.text*)
			*:arm_mat_inverse_f64.o(.text*)
			*:lib_a-sf_frexp.o(.text*)
			*:lib_a-wf_exp.o(.text*)
			*:lib_a-ef_exp.o(.text*)
			*:lib_a-sf_tanh.o(.text*)
			*:lib_a-sf_fabs.o(.text*)
			*:arm_depthwise_transposedconv_2x4_q15_q7.o(.text*)
			*:echo_suppression_core.o(.text*)
			*:coherence.o(.text*)
			*:mdf2.o(.text*)
			*:noise_generator.o(.text*)
			*:pink_noise_gen.o(.text*)
			*:white_noise_gen.o(.text*)
#endif

			*:cmsis_fftwrap.o(.text*)
			*:arm_rfft_fast_f32.o(.text*)
			*:arm_cfft_f32.o(.text*)
#endif
			*(.overlay_text0)
			. = ALIGN(4);
		}
		.overlay_text1
		{
			LONG(0);
#if !defined(RB_CODEC)
			*:fixedpoint.o(.text*)
			*:replaygain.o(.text*)
			*:dsp_core.o(.text*)
			*:dsp_misc.o(.text*)
			*:dsp_filter.o(.text*)
			*:dsp_sample_input.o(.text*)
			*:dsp_sample_output.o(.text*)
			*:eq.o(.text*)
#if defined(A2DP_SBC_PLC_ENABLED)
			*:sbcplc.o(.text*)
#endif
			*:pga.o(.text*)
#if !defined(A2DP_AAC_ON) && !defined(A2DP_LHDC_ON)
			*:fir_process.o(.text*)
			*:iir_process.o(.text*.iir_run .text*.*iir_run_per_*)
#endif
#endif
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
			*:a2dp_decoder_sbc.o(.text*)
#endif
			*(.overlay_a2dp_sbc*)
			*(.overlay_text1)
			. = ALIGN(4);
		}
		.overlay_text2
		{
			LONG(0);
			*:fmdec.o(.text* .data* .rodata* .bss*)
			*(.overlay_text2)
			. = ALIGN(4);
		}
		.overlay_text3
		{
			LONG(0);
			*:fft128dot.o(.text*)
			*(.overlay_text3)
			. = ALIGN(4);
		}
		.overlay_text4
		{
			LONG(0);
#if defined(A2DP_AAC_ON)
			*:fdkaac_codec.o(.text*)
#endif
			*(.overlay_a2dp_aac*)
			*(.overlay_text4)
			. = ALIGN(4);
		}
		.overlay_text5
		{
			LONG(0);
			*:mpa.o(.text*)
			*:bit.o(.text*)
			*:frame.o(.text*)
			*:huffman.o(.text*)
			*:layer12.o(.text*)
			*:layer3.o(.text*)
			*:stream.o(.text*)
			*:synth.o(.text*)
			*(.overlay_text5)
			. = ALIGN(4);
		}
		.overlay_text6
		{
			LONG(0);
#if defined(A2DP_SCALABLE_ON)
			*:Audio_asm.o(.text*)
			*:vq.o(.text*)
			*:util.o(.text*)
			*:rate.o(.text*)
			*:quant_bands.o(.text*)
			*:modes.o(.text*)
			*:mathops.o(.text*)
			*:laplace.o(.text*)
			*:entdec.o(.text*)
			*:entcode.o(.text*)
			*:cwrs.o(.text*)
			*:audio_int32.o(.text*)
			*:audio_decoder.o(.text*)
			*:audio.o(.text*)
			*(.overlay_a2dp_ssc*)
#endif

#if defined(A2DP_LHDC_ON)
#ifdef A2DP_LHDC_V3
			*(.overlay_a2dp_lhdc*)
			*:a2dp_decoder_lhdc.o(.text*)
			*:testkey.o(.text*)
			*:lhdcUtil.o(.text*)
			*:lzh_dec.o(.text*)
			*:licdecrypt.o(.text*)
			*:lic_check.o(.text*)
			*:fast_upsampling_2x.o(.text*)
			*:encdec.o(.text*)
			*:dec.o(.text*)
			*:cirbuf.o(.text*)
			*:DecodeUps2xLPF.o(.text*)
#else
			*:lhdcUtil.o(.text*)
			*:dec.o(.text*)
			*:cirbuf.o(.text*)
			*(.overlay_a2dp_lhdc*)
#endif
#endif
#if defined(A2DP_LDAC_ON)
			*:ldacBT_api.o(.text*)
			*:decode_ldac.o(.text*)
			*:dequant_ldac.o(.text*)
			*:imdct_ldac.o(.text*)
			*:ldacBT_internal.o(.text*)
			*:ldaclib_api.o(.text*)
			*:memory_ldac.o(.text*)
			*:setpcm_ldac.o(.text*)
			*:sigana_ldac.o(.text*)
			*:tables_ldac.o(.text*)
			*:tables_sigproc_ldac.o(.text*)
			*:unpack_ldac.o(.text*)
			*(.overlay_a2dp_ldac*)
#endif

			*(.overlay_text6)
			. = ALIGN(4);
		}
		.overlay_text7
		{
			LONG(0);
			*(.overlay_text7)
			. = ALIGN(4);
		}
#endif
		.overlay_text_last
		{
			LONG(0);
			. = ALIGN(4);
		}
	} > REGION_OVERLAY_TEXT

	.dummy_overlay_text_end (.) (NOLOAD) : AT (ADDR(.dummy_overlay_text_end))
	{
		__overlay_text_exec_end__ = .;
	} > REGION_OVERLAY_TEXT

#ifdef OVERLAY_FOR_CP
	__fast_sram_end__ = __fast_sram_text_data_end__;
#else
	__fast_sram_end__ = RAMX_TO_RAM(__overlay_text_exec_end__);
#endif

	ASSERT(__fast_sram_end__ - __fast_sram_text_data_start__ <= FAST_XRAM_SECTION_SIZE, "fast xram sections too large")
	__free_fram = FAST_XRAM_SECTION_SIZE - (__fast_sram_end__ - __fast_sram_text_data_start__);

#ifdef OVERLAY_FOR_CP
	__overlay_data_start__ = __cp_bss_sram_end;
#else
	__overlay_data_start__ = __sram_bss_end__;
#endif

	OVERLAY (__overlay_data_start__) : NOCROSSREFS AT (__load_stop_overlay_text_last)
	{
#ifndef NO_OVERLAY
		.overlay_data0
		{
			/* Explicitly place 4 bytes at section start to avoid "section type changed to PROGBITS" warnings */
			LONG(0);
			*:smallft.o(.data* .rodata*)
			*:fftwrap.o(.data* .rodata*)
			*:lib_a-s_floor.o(.data* .rodata*)
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
			*:bt_sco_chain.o(.data* .rodata*)
			*:bt_sco_chain_cp.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_DTLN) && defined(DTLN_CP)
			*:fully_connected_common.o(.data* .rodata*)
			*:dequantize.o(.data* .rodata*)
			*:logistic.o(.data* .rodata*)
			*:tanh.o(.data* .rodata*)
			*:sub_common.o(.data* .rodata*)
			*:mul.o(.data* .rodata*)
			*:strided_slice.o(.data* .rodata*)
			*:quantize_common.o(.data* .rodata*)
			*:quantize.o(.data* .rodata*)
			*:logistic_common.o(.data* .rodata*)
			*:add_common.o(.data* .rodata*)
			*:add.o(.data* .rodata*)
			*:unpack.o(.data* .rodata*)
			*:dequantize.o(.data* .rodata*)
			*:reduce.o(.data* .rodata*)
			*:squared_difference.o(.data* .rodata*)
			*:pack.o(.data* .rodata*)
			*:mul_common.o(.data* .rodata*)
			*:reshape.o(.data* .rodata*)
			*:elementwise.o(.data* .rodata*)
			*:split.o(.data* .rodata*)
			*:dequantize_common.o(.data* .rodata*)
			*:sub.o(.data* .rodata*)
			*:common.o(.data* .rodata*)
			*:op_resolver.o(.data* .rodata*)
			*:flatbuffer_utils.o(.data* .rodata*)
			*:op_resolver.o(.data* .rodata*)
			/*:greedy_memory_planner.o(.data* .rodata*)*/
			*:memory_helpers.o(.data* .rodata*)
			*:micro_allocator.o(.data* .rodata*)
			*:micro_graph.o(.data* .rodata*)
			*:micro_interpreter.o(.data* .rodata*)
			*:micro_string.o(.data* .rodata*)
			*:simple_memory_allocator.o(.data* .rodata*)
			*:kernel_util.o(.data* .rodata*)
			*:dtln_denoise.o(.data* .rodata*)
			*:fully_connected.o(.data* .rodata*)
			*:flatbuffer_conversions.o(.data* .rodata*)
			*:micro_error_reporter.o(.data* .rodata*)
			*:cmsis_fftwrap.o(.data* .rodata*)
			*:activations_common.o(.data* .rodata*)
			*:error_reporter.o(.data* .rodata*)
			*:quantization_util.o(.data* .rodata*)

			*:resize_nearest_neighbor.o(.data* .rodata*)
			*:transpose_conv.o(.data* .rodata*)
			*:maximum_minimum.o(.data* .rodata*)

			/*:conv.o(.data* .rodata*)*/

			*:rnn.o(.data* .rodata*)
#if defined(SPEECH_TX_NS4)
			*:speech_ns4.o(.data* .rodata*)
			*:speech_ns4_core.o(.data* .rodata*)
#endif
#else
#if defined(SPEECH_TX_NS5)
			*:ns5_pitch.o(.data* .rodata*)
			*:ns5_celt_lpc.o(.data* .rodata*)
			*:rnn.o(.data* .rodata*)
			*:rnn_data_small.o(.data* .rodata.input_dense*)
			*:rnn_data_small.o(.data* .rodata.noise_gru_bias*)
			*:rnn_data_small.o(.data* .rodata.noise_gru)

			*:rnn_data_small.o(.data* .rodata.denoise_gru_bias*)
			*:rnn_data_small.o(.data* .rodata.denoise_gru)

			*:rnn_data_small.o(.data* .rodata.denoise_output_weights*)
			*:rnn_data_small.o(.data* .rodata.denoise_output_bias)
			*:rnn_data_small.o(.data* .rodata.denoise_output)
			*:rnn_data_small.o(.data* .rodata.ns5_model_small)

			*:rnn_data_large.o(.data* .rodata.input_dense*)
			*:rnn_data_large.o(.data* .rodata.noise_gru_bias*)
			*:rnn_data_large.o(.data* .rodata.noise_gru)

			*:rnn_data_large.o(.data* .rodata.denoise_gru_bias*)
			*:rnn_data_large.o(.data* .rodata.denoise_gru)

			*:rnn_data_large.o(.data* .rodata.denoise_output_weights*)
			*:rnn_data_large.o(.data* .rodata.denoise_output_bias)
			*:rnn_data_large.o(.data* .rodata.denoise_output)
			*:rnn_data_large.o(.data* .rodata.ns5_model_large)
			*:speech_ns5_nn.o(.data* .rodata*)

			*:nsx_core_freq.o(.data* .rodata*)
			
			*:echo_suppression_core.o(.data* .rodata*)
			*:integer_delay.o(.data* .rodata*)
			*:coherence.o(.data* .rodata*)
			*:logmmse.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:mdf2.o(.data* .rodata*)
			*:noise_generator.o(.data* .rodata*)
			*:pink_noise_gen.o(.data* .rodata*)
			*:white_noise_gen.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_AEC2FLOAT) && !defined(AUDIO_ANC_FB_ADJ_MC) && !defined(AUDIO_ADJ_EQ)
			*:coherence.o(.data* .rodata*)
			*:echo_canceller.o(.data* .rodata*)
			*:echo_suppression.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:logmmse.o(.data* .rodata*)
			*:mdf2.o(.data* .rodata*)
			*:noise_generator.o(.data* .rodata*)
			*:pink_noise_gen.o(.data* .rodata*)
			*:white_noise_gen.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_NS4)
			*:speech_ns4.o(.data* .rodata*)
			*:speech_ns4_core.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_NS7)
			*:logmmse.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:besnet_utils.o(.data* .rodata*)
			*:dw_conv_layer.o(.data* .rodata*)
			*:dw_deconv_layer.o(.data* .rodata*)
			*:fully_connected_layer.o(.data* .rodata*)
			*:gru_layer.o(.data* .rodata*)
			*:output_layer.o(.data* .rodata*)
			*:speech_ns7.o(.data* .rodata*)
			*:speech_win.o(.data* .rodata*)
			*:filterbank_pflt.o(.data* .rodata*)
			*:arm_depthwise_separable_conv2x4_q15_q7_q31.o(.data* .rodata*)
			*:arm_convolve_1x1_HWC_q15_q7_q31.o(.data* .rodata*)
			*:arm_depthwise_transposed_conv2x4_q15_q7.o(.data* .rodata*)
			*:arm_fully_connected_q15_q7_q31.o(.data* .rodata*)
			*:arm_mat_trans_q15.o(.data* .rodata*)
			*:arm_mat_inverse_f64.o(.data* .rodata*)
			*:lib_a-sf_frexp.o(.data* .rodata*)
			*:lib_a-wf_exp.o(.data* .rodata*)
			*:lib_a-ef_exp.o(.data* .rodata*)
			*:lib_a-sf_tanh.o(.data* .rodata*)
			*:lib_a-sf_fabs.o(.data* .rodata*)
			*:arm_depthwise_transposedconv_2x4_q15_q7.o(.data* .rodata*)
			*:echo_suppression_core.o(.data* .rodata*)
			*:integer_delay.o(.data* .rodata*)
			*:coherence.o(.data* .rodata*)
			*:mdf2.o(.data* .rodata*)
			*:noise_generator.o(.data* .rodata*)
			*:pink_noise_gen.o(.data* .rodata*)
			*:white_noise_gen.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_2MIC_NS7)
			*:integer_delay.o(.data* .rodata*)
			*:ns_core.o(.data* .rodata*)
			*:noise_suppression.o(.data* .rodata*)
			*:speech_2mic_ns7.o(.data* .rodata*)
			/*:rnn.o(.data* .rodata*)*/
			*:spectral_wdrc.o(.data* .rodata*)
			*:logmmse.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:besnet_utils.o(.data* .rodata*)
			*:dw_conv_layer.o(.data* .rodata*)
			*:dw_deconv_layer.o(.data* .rodata*)
			*:fully_connected_layer.o(.data* .rodata*)
			*:gru_layer.o(.data* .rodata*)
			*:output_layer.o(.data* .rodata*)
			*:speech_ns7.o(.data* .rodata*)
			*:speech_win.o(.data* .rodata*)
			*:filterbank_pflt.o(.data* .rodata*)
			*:arm_depthwise_separable_conv2x4_q15_q7_q31.o(.data* .rodata*)
			*:arm_convolve_1x1_HWC_q15_q7_q31.o(.data* .rodata*)
			*:arm_depthwise_transposed_conv2x4_q15_q7.o(.data* .rodata*)
			*:arm_fully_connected_q15_q7_q31.o(.data* .rodata*)
			*:arm_mat_trans_q15.o(.data* .rodata*)
			*:arm_mat_inverse_f64.o(.data* .rodata*)
			*:lib_a-sf_frexp.o(.data* .rodata*)
			*:lib_a-wf_exp.o(.data* .rodata*)
			*:lib_a-ef_exp.o(.data* .rodata*)
			*:lib_a-sf_tanh.o(.data* .rodata*)
			*:lib_a-sf_fabs.o(.data* .rodata*)
#endif
			*:cmsis_fftwrap.o(.rodata*)
			*:arm_rfft_fast_f32.o(.rodata*)
			*:arm_cfft_f32.o(.rodata*)
#endif
			*:speech_win.o(.data* .rodata*)
			*(.overlay_data0 .overlay_rodata0)
			. = ALIGN(4);
			*(.overlay_sco_cache*)
			. = ALIGN(4);
		}
		.overlay_data1
		{
			LONG(0);
#if !defined(RB_CODEC)
			*:fixedpoint.o(.data* .rodata*)
			*:replaygain.o(.data* .rodata*)
			*:dsp_core.o(.data* .rodata*)
			*:dsp_misc.o(.data* .rodata*)
			*:dsp_filter.o(.data* .rodata*)
			*:dsp_sample_input.o(.data* .rodata*)
			*:dsp_sample_output.o(.data* .rodata*)
			*:eq.o(.data* .rodata*)
			*:pga.o(.data* .rodata*)
#if defined(A2DP_SBC_PLC_ENABLED)
			*:sbcplc.o(.data* .rodata*)
#endif
#if !defined(A2DP_AAC_ON)
			*:fir_process.o(.data* .rodata*)
#endif
#endif
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
			*:a2dp_decoder_sbc.o(.rodata*)
#endif
			*(.overlay_data1 .overlay_rodata1)
			. = ALIGN(4);
		}
		.overlay_data2
		{
			LONG(0);
			*(.overlay_data2 .overlay_rodata2)
			. = ALIGN(4);
		}
		.overlay_data3
		{
			LONG(0);
			*:fft128dot.o(.data* .rodata*)
			*(.overlay_data3 .overlay_rodata3)
			. = ALIGN(4);
		}
		.overlay_data4
		{
			LONG(0);
#if defined(A2DP_AAC_ON)
			*:fdkaac_codec.o(.data .rodata*)
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
			*:a2dp_decoder_aac_lc.o(.data .rodata*)
#endif
#endif
			*(.overlay_data4 .overlay_rodata4)
			. = ALIGN(4);
		}
		.overlay_data5
		{
			LONG(0);
			*:mpa.o(.data* .bss*)
			*:bit.o(.data* .bss*)
			*:frame.o(.data* .bss*)
			*:huffman.o(.data* .bss*)
			*:layer12.o(.data* .bss*)
			*:layer3.o(.data* .bss*)
			*:stream.o(.data* .bss*)
			*:synth.o(.data* .bss*)
			*(.overlay_data5 .overlay_rodata5)
			. = ALIGN(4);
		}
		.overlay_data6
		{
			LONG(0);
#if defined(A2DP_SCALABLE_ON)
			*:rc_decode.o(.data* .rodata*)
			*:audio_int32.o(.data* .rodata*)
			*:ssc_ifft.o(.data* .rodata*)
			*:vector_quant.o(.data* .rodata*)
			*:ssc_quant.o(.data* .rodata*)
			*:ssc_pulsealloc.o(.data* .rodata*)
			*:audio.o(.data* .rodata*)
			*:set_sbm.o(.data* .rodata*)
			*:rc_code.o(.data* .rodata*)
			*:SBM_pitchBased.o(.data* .rodata*)
			*:ssc_imdct.o(.data* .rodata*)
			*:audio_decoder.o(.data* .rodata*)
			*:SoundBooster_DRC_classic.o(.data* .rodata*)
			*:ssc_cwrs.o(.data* .rodata*)
			*:ssc_mode.o(.data* .rodata*)
			*:ssc_math.o(.data* .rodata*)
			*:ssc_lap.o(.data* .rodata*)
			*:SeamlessBufferManagement.o(.data* .rodata*)
#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
			*:a2dp_decoder_scalable.o(.rodata*)
			*:a2dp_decoder_scalable.o(.bss*ss_pcm_buff)
#endif
#endif
#if defined(A2DP_LHDC_ON)
			/*LLAC*/
			*:llac_dec_alg.o(.data* .rodata*)
			*:llac_dec_alg_code.o(.data* .rodata*)
			*:llac_dec_alg_mdct.o(.data* .rodata*)
			*:llac_dec_alg_quant.o(.data* .rodata*)
			*:llac_dec_alg_signal.o(.data* .rodata*)
			*:llac_dec_const.o(.data* .rodata*)
			*:llac_dec_init.o(.data* .rodata*)
			*:llac_dec_mem_alloc.o(.data* .rodata*)
			*:llac_dec_wrapper.o(.data* .rodata*)
			*:sv_math.o(.data* .rodata*)
			*:uhead_api.o(.data* .rodata*)

			/*LHDC*/
			*:a2dp_decoder_lhdc.o(.data* .rodata*)
			*:testkey.o(.data* .rodata*)
			*:lhdcUtil.o(.data* .rodata*)
			*:lzh_dec.o(.data* .rodata*)
			*:licdecrypt.o(.data* .rodata*)
			*:lic_check.o(.data* .rodata*)
			*:fast_upsampling_2x.o(.data* .rodata*)
			*:encdec.o(.data* .rodata*)
			*:dec.o(.data* .rodata*)
			*:cirbuf.o(.data* .rodata*)
			*:DecodeUps2xLPF.o(.data* .rodata*)
#endif
#if defined(A2DP_SOURCE_LHDC_ON)
			*:EncodeLPF.o(.data* .rodata*)
			*:enc.o(.data* .rodata*)
			*:encdec.o(.data* .rodata*)
			*:lhdc_enc_api.o(.data* .rodata*)
			*:lhdc_process.o(.data* .rodata*)
			*:lic_check.o(.data* .rodata*)
			*:licdecrypt.o(.data* .rodata*)
			*:lzh_enc.o(.data* .rodata*)
			*:win_sinc.o(.data* .rodata*)
			*:a2dp_encoder_lhdc_cp.o(.data* .rodata*)
#endif
#if defined(A2DP_LDAC_ON)
			*:a2dp_decoder_ldac.o(.data* .rodata*)
			*:bitalloc_ldac.o(.data* .rodata*)
			*:bitalloc_sub_ldac.o(.data* .rodata*)
			*:encode_ldac.o(.data* .rodata*)
			*:ldacBT.o(.data* .rodata*)
			*:ldaclib.o(.data* .rodata*)
			*:pack_ldac.o(.data* .rodata*)
			*:quant_ldac.o(.data* .rodata*)
			*:mdct_ldac.o(.data* .rodata*)
			*:ldacBT_api.o(.data* .rodata*)
			*:decode_ldac.o(.data* .rodata*)
			*:dequant_ldac.o(.data* .rodata*)
			*:imdct_ldac.o(.data* .rodata*)
			*:ldacBT_internal.o(.data* .rodata*)
			*:ldaclib_api.o(.data* .rodata*)
			*:memory_ldac.o(.data* .rodata*)
			*:setpcm_ldac.o(.data* .rodata*)
			*:sigana_ldac.o(.data* .rodata*)
			*:tables_ldac.o(.data* .rodata*)
			*:tables_sigproc_ldac.o(.data* .rodata*)
			*:unpack_ldac.o(.data* .rodata*)
#endif
			*:mp3_dec.o(.data* .rodata*)
			*:mp3_enc.o(.data* .rodata*)
			*(.overlay_data6 .overlay_rodata6)
			. = ALIGN(4);
		}
		.overlay_data7
		{
			LONG(0);
			*(.overlay_data7 .overlay_rodata7)
			. = ALIGN(4);
		}
#endif
		.overlay_data_last
		{
			LONG(0);
			. = ALIGN(4);
		}
	} > REGION_OVERLAY_DATA

	.dummy_overlay_data_end (.) (NOLOAD) : AT (ADDR(.dummy_overlay_data_end))
	{
		__overlay_data_end__ = .;
	} > REGION_OVERLAY_DATA

	ASSERT(__overlay_data_end__ - __overlay_data_start__ <= OVERLAY_DATA_SECTION_SIZE, "overlay data sections too large")

#if defined(CHIP_HAS_CP) && (RAMCP_SIZE > 0)
	.cp_ramx_last_dummy (NOLOAD) : AT (ADDR(.cp_ramx_last_dummy))
	{
		__cp_ramx_last_dummy_start = .;
	} > RAMCPX

	.cp_ram_last_dummy (NOLOAD) : AT (ADDR(.cp_ram_last_dummy))
	{
		. = ALIGN(8);
		__cp_ram_last_dummy_start = .;
	} > RAMCP

	__cp_stack_top = ORIGIN(RAMCP) + LENGTH(RAMCP);
	__cp_stack_limit = __cp_stack_top - CP_STACK_SECTION_SIZE;

	/* LD v2.24 (GCC 4.9) cannot handle address conversion correctly if involving immediate value.
	   E.g., <absolute_addr> + <immediate_val> + <relative_addr> */
	__free_ramcpx = ORIGIN(RAMCPX) + LENGTH(RAMCPX) - ABSOLUTE(__cp_ramx_last_dummy_start);
	ASSERT(__cp_stack_limit >= __cp_ram_last_dummy_start, "region RAMCP overflowed with stack")
	__free_ramcp = __cp_stack_limit - __cp_ram_last_dummy_start;
	__cp_mem_pool_start__ = __cp_ram_last_dummy_start;
	__cp_mem_pool_end__ = __cp_stack_limit;
#endif

	.text (FLASH_TO_FLASHX(__load_stop_overlay_data_last)) : AT (FLASHX_TO_FLASH(ADDR(.text)))
	{
		*(.text*)
		*(.flash_text*)

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)
	} > FLASHX

	.ARM.extab (.) : AT (FLASHX_TO_FLASH(ADDR(.ARM.extab)))
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASHX

	__exidx_start = .;
	/* .ARM.exidx contains R_ARM_PREL31 (+-0x40000000) offset to functions, which means
	 * the session location cannot be too far away from the function addresses */
	.ARM.exidx (.) : AT (FLASHX_TO_FLASH(ADDR(.ARM.exidx)))
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASHX
	__exidx_end = .;

	. = FLASHX_TO_FLASH(.);
#ifdef TRACE_STR_SECTION
	.rodata_str (.):
	{
	    __rodata_str_start__ = .;
		*(.rodata.__func__.*)
		*(.rodata.*__func__)
		*(.rodata.str1*)
		*(.rodata.*.str1*)
		*(.rodata.__FUNCTION__.*)
		*(.rodata.*__FUNCTION__)
		__rodata_str_end__ = .;
	} > FLASH

	RODATA_ADDRESS = .;
	. = 0xFFFC0000;
	.trc_str (.):
	{
		*(.rodata.__func__.*)
		*(.rodata.*__func__)
		*(.rodata.__FUNCTION__.*)
		*(.rodata.*__FUNCTION__)
		*(.trc_str*)
	}
	. = RODATA_ADDRESS;
#endif
	.rodata (.) :
	{
		*(.rodata*)
		*(.flash_rodata*)
		*(.constdata)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		. = ALIGN(4);
		__cust_cmd_hldr_tbl_start = .;
		KEEP(*(.cust_cmd_hldr_tbl))
		__cust_cmd_hldr_tbl_end = .;
		. = ALIGN(4);
		__ai_gatt_server_table_start = .;
		KEEP(*(.ai_gatt_server_table))
		__ai_gatt_server_table_end = .;
		. = ALIGN(4);
		__ai_ble_handler_table_start = .;
		KEEP(*(.ai_ble_handler_table))
		__ai_ble_handler_table_end = .;
		. = ALIGN(4);
		__ai_spp_register_table_start = .;
		KEEP(*(.ai_spp_register_table))
		__ai_spp_register_table_end = .;
		. = ALIGN(4);
		__tota_handler_table_start = .;
		KEEP(*(.tota_handler_table))
		__tota_handler_table_end = .;
		. = ALIGN(4);
		__ai_handler_function_table_start = .;
		KEEP(*(.ai_handler_function_table))
		__ai_handler_function_table_end = .;
		. = ALIGN(4);
		__custom_handler_table_start = .;
		KEEP(*(.custom_handler_table))
		__custom_handler_table_end = .;
		. = ALIGN(4);
		__thirdparty_handler_table_start = .;
		KEEP(*(.thirdparty_handler_table))
		__thirdparty_handler_table_end = .;

		KEEP(*(.eh_frame*))
		*(.note.gnu.build-id)
	} > FLASH

#if defined(FIRMWARE_REV)
	.firmware_rev (.) :
	{
		. = ALIGN(64);
		*(.fw_rev)
	} > FLASH
#endif

	/* To copy multiple FLASH to RAM sections,
	 * uncomment .copy.table section and,
	 * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */
	/*
	.copy.table (.) :
	{
		. = ALIGN(4);
		__copy_table_start__ = .;
		LONG (__etext)
		LONG (__data_start__)
		LONG (__data_end__ - __data_start__)
		LONG (__etext2)
		LONG (__data2_start__)
		LONG (__data2_end__ - __data2_start__)
		__copy_table_end__ = .;
	} > FLASH
	*/

	/* To clear multiple BSS sections,
	 * uncomment .zero.table section and,
	 * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */
	/*
	.zero.table (.) :
	{
		. = ALIGN(4);
		__zero_table_start__ = .;
		LONG (__bss_start__)
		LONG (__bss_end__ - __bss_start__)
		LONG (__bss2_start__)
		LONG (__bss2_end__ - __bss2_start__)
		__zero_table_end__ = .;
	} > FLASH
	*/

	/* Location counter can end up 2byte aligned with narrow Thumb code but
	   __etext is assumed by startup code to be the LMA of a section in RAM
	   which must be 4byte aligned */
	__etext = ALIGN(4);

	/* The VMA is either the end of overlay_data or the end of sram bss */

	.data : AT (__etext)
	{
		__data_start__ = .;

		*(vtable)
		. = ALIGN(4);
		*(.data*)

		KEEP(*(.jcr*))

		/* All data end */
		. = ALIGN(4);
		__data_end__ = .;

	} > RAM

	.bss (.) (NOLOAD) : AT (ADDR(.bss))
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM

	.reboot_custom_param (.) (NOLOAD) :
	{
		KEEP(*(.reboot_custom_param*))
		. = ALIGN(4);
		__reboot_custom_param_end__ = .;
	} > RAM

	.heap (.) (NOLOAD) :
	{
		. = ALIGN(8);
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		. += HEAP_SECTION_SIZE;
		. = ALIGN(8);
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (.) (COPY) :
	{
		. = STACK_SECTION_SIZE;
		. = ALIGN(8);
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
#if defined(ROM_UTILS_ON)
	__StackTop = ORIGIN(RAM) + LENGTH(RAM) - ROM_UTILS_RESV_RAM_SIZE;
#else
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
#endif
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
	__free_ram = __StackLimit - __HeapLimit;

	__mem_pool_start__ = __HeapLimit;
	__mem_pool2_start__ = __mem_pool_end__ - MEM_POOL_SIZE;
	__mem_pool_end__ = __StackLimit;

	. = __etext + SIZEOF(.data);

	.system_info (.) :
	{
		KEEP(*(.system_info_list_size))
		KEEP(*(.system_info_list))
		. = ALIGN(4);
	}  > FLASH

	.build_info (.) :
	{
		KEEP(*(.build_info))
		. = ALIGN(4);
	} > FLASH = 0x00000000

#ifdef SLAVE_BIN_FLASH_OFFSET
	.slave_code (ORIGIN(FLASH) + SLAVE_BIN_FLASH_OFFSET):
	{
		KEEP(*(.slave_code_flash_rodata))
	} > FLASH
#endif

	/* The following section MUST be the last loaded section */
	.code_start_addr (.) :
	{
#ifdef TRACE_STR_SECTION
		LONG(ABSOLUTE(__rodata_str_start__));
		LONG(0x30000);  /* TRC_STR_OFFSET, should same to --change-section-lma .trc_str value in Makefile  */
#else
		LONG(0);
#endif
		LONG(BUILD_INFO_MAGIC);
		LONG(ABSOLUTE(__flash_start));
	} > FLASH

	__flash_end = .;

#ifdef QIOT_ENABLED
	.qiot_core_section (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
#ifdef _STDF__
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - CORE_DUMP_SECTION_SIZE -PROMPT_SECTION_SIZE - STDF_RESERVED_0_SECTION_SIZE - STDF_RESERVED_1_SECTION_SIZE -  QIOT_OTA_SECTION_SIZE - QIOT_CORE_SECTION_SIZE) (NOLOAD) :
#else
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - CORE_DUMP_SECTION_SIZE -PROMPT_SECTION_SIZE -  QIOT_OTA_SECTION_SIZE - QIOT_CORE_SECTION_SIZE) (NOLOAD) :
#endif
	{
		__qiot_core_section_start = .;
		. = QIOT_CORE_SECTION_SIZE;
		__qiot_core_section_end = .;
	} > FLASH_NC

	.qiot_ota_section (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
#ifdef _STDF__
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - PROMPT_SECTION_SIZE - CORE_DUMP_SECTION_SIZE - STDF_RESERVED_0_SECTION_SIZE - STDF_RESERVED_1_SECTION_SIZE - QIOT_OTA_SECTION_SIZE) (NOLOAD) :
#else
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - PROMPT_SECTION_SIZE - CORE_DUMP_SECTION_SIZE - QIOT_OTA_SECTION_SIZE) (NOLOAD) :
#endif
	{
		__qiot_ota_section_start = .;
		. = QIOT_OTA_SECTION_SIZE;
		__qiot_ota_section_end = .;
	} > FLASH_NC
#endif
#ifdef __STDF__
    .stdf_reserved_1 (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
        AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
        CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - CORE_DUMP_SECTION_SIZE -
        PROMPT_SECTION_SIZE - STDF_RESERVED_0_SECTION_SIZE - STDF_RESERVED_1_SECTION_SIZE) (NOLOAD):
    {
        __stdf_reserved_1_section_start = .;
        . = STDF_RESERVED_1_SECTION_SIZE;
        __stdf_reserved_1_section_end = .;
    } > FLASH_NC
    .stdf_reserved_0 (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
        AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
        CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - CORE_DUMP_SECTION_SIZE -
        PROMPT_SECTION_SIZE - STDF_RESERVED_0_SECTION_SIZE) (NOLOAD):
    {
        __stdf_reserved_0_section_start = .;
        . = STDF_RESERVED_0_SECTION_SIZE;
        __stdf_reserved_0_section_end = .;
    } > FLASH_NC
#endif /* __STDF__ */

	.prompt_section (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - CORE_DUMP_SECTION_SIZE - PROMPT_SECTION_SIZE) (NOLOAD) :
	{
		__prompt_section_start = .;
		. = PROMPT_SECTION_SIZE;
		__prompt_section_end = .;
	} > FLASH_NC

	.coredump_section (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE - CORE_DUMP_SECTION_SIZE) (NOLOAD) :
	{
		__coredump_section_start = .;
		. = CORE_DUMP_SECTION_SIZE;
		__coredump_section_end = .;
	} > FLASH_NC

	.ota_upgrade_log (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE - OTA_UPGRADE_LOG_SIZE) (NOLOAD) :
	{
		__ota_upgrade_log_start = .;
		. = OTA_UPGRADE_LOG_SIZE;
		__ota_upgrade_log_end = .;
	} > FLASH_NC

	.log_dump (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
		CRASH_DUMP_SECTION_SIZE - LOG_DUMP_SECTION_SIZE) (NOLOAD) :
	{
		__log_dump_start = .;
		. = LOG_DUMP_SECTION_SIZE;
		__log_dump_end = .;
	} > FLASH_NC

	.crash_dump (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE -  USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE -
		CRASH_DUMP_SECTION_SIZE) (NOLOAD) :
	{
		__crash_dump_start = .;
		. = CRASH_DUMP_SECTION_SIZE;
		__crash_dump_end = .;
	} > FLASH_NC

	.custom_parameter (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
	 AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2 - CUSTOM_PARAMETER_SECTION_SIZE) (NOLOAD) :
	{
		__custom_parameter_start = .;
		. = CUSTOM_PARAMETER_SECTION_SIZE;
		__custom_parameter_end = .;
	} > FLASH_NC

	.userdata (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE  - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE - USERDATA_SECTION_SIZE*2) (NOLOAD) :
	{
		__userdata_start = .;
		. = USERDATA_SECTION_SIZE*2;
		__userdata_end = .;
	} > FLASH_NC

	.audio (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE - RESERVED_SECTION_SIZE -
		AUD_SECTION_SIZE) (NOLOAD) :
	{
		__aud_start = .;
		. = AUD_SECTION_SIZE;
		__aud_end = .;
	} > FLASH_NC

	.reserved (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE - RESERVED_SECTION_SIZE) (NOLOAD) :
	{
		__reserved_start = .;
		. = RESERVED_SECTION_SIZE;
		__reserved_end = .;
	} > FLASH_NC

	.factory (ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) - FACTORY_SECTION_SIZE) (NOLOAD) :
	{
		__factory_start = .;
		. = FACTORY_SECTION_SIZE;
		__factory_end = .;
	} > FLASH_NC

#ifdef OTA_BOOT_SIZE
#if (OTA_BOOT_SIZE > FLASH_SIZE)
#error "OTA_BOOT_SIZE should <= FLASH_SIZE"
#endif
#if defined(OTA_CODE_OFFSET) && (OTA_BOOT_SIZE > OTA_CODE_OFFSET)
#error "OTA_BOOT_SIZE should <= OTA_CODE_OFFSET"
#endif
	__tail_section_start = FLASH_BASE + OTA_BOOT_SIZE;
#else
#ifdef QIOT_ENABLED
	__tail_section_start = __qiot_core_section_start;
#else
#ifdef __STDF__
    __tail_section_start = __stdf_reserved_1_section_start;
#endif
	__tail_section_start = __prompt_section_start;
#endif
#endif
#if 0
	ASSERT(FLASH_NC_TO_C(__tail_section_start) >= __flash_end, "region FLASH overflowed")
	__free_flash = FLASH_NC_TO_C(__tail_section_start) - __flash_end;
#endif
#if !defined(OTA_BOOT_SIZE) && defined(OTA_REMAP_OFFSET)
	ASSERT((FLASH_BASE & 0x03FFFFFF) == 0, "bad FLASH_BASE")
	ASSERT((FLASH_SIZE & (FLASH_SIZE - 1)) == 0, "bad FLASH_SIZE")
#ifdef CHIP_BEST2300P
	ASSERT(__flash_end <= FLASH_BASE + OTA_REMAP_OFFSET, "flash code size too large to remap")
	ASSERT(__flash_start - FLASH_BASE >= FLASH_NC_BASE + FLASH_SIZE - __tail_section_start, "flash code conflicted with tail sections in remap")
	ASSERT(__flash_start - FLASH_BASE >= FLASH_SIZE / (1 << (4 + 1)), "flash code cannot remap within 4 regions (max code size is half of flash size)")
	ASSERT((__flash_start - FLASH_BASE) % (FLASH_SIZE / (1 << (4 + 1))) == 0, "flash code start address not aligned with remap region boundary")
#else
	ASSERT(((OTA_REMAP_OFFSET) & (0x1000 - 1)) == 0, "OTA_REMAP_OFFSET not aligned with 4K sector boundary")
	ASSERT(__flash_end <= FLASH_BASE + (OTA_REMAP_OFFSET), "flash code size too large to remap")
	ASSERT((__tail_section_start & (0x1000 - 1)) == 0, "__tail_section_start not aligned with 4K sector boundary")
	ASSERT(__flash_end + (OTA_REMAP_OFFSET) <= __tail_section_start, "flash code conflicted with tail sections in remap")
#endif
#endif
}

