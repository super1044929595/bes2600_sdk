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
#ifdef CHIP_SUBSYS_SENS
#ifndef __ARM_ARCH_ISA_ARM

#include "cmsis_nvic.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_location.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif
#include "plat_types.h"
#include "plat_addr_map.h"
#include "system_sens.h"

#define RESET_HANDLER_LOC_SENS                  __attribute__((section(".reset_handler_sens")))

void Reset_Handler(void) __attribute__((alias("system_sens_reset_handler")));

void NAKED RESET_HANDLER_LOC_SENS system_sens_reset_handler(void)
{
    asm volatile (
        "ldr r3, =" TO_STRING(__StackTop) ";"
        "msr msp, r3;"
#ifdef __ARM_ARCH_8M_MAIN__
        "ldr r0, =" TO_STRING(__StackLimit) ";"
        "msr msplim, r0;"
#endif
        "bl hal_senscmu_wakeup_check;"
        "movs r4, 0;"
        "mov r5, r4;"
        "mov r6, r4;"
        "mov r7, r4;"
        "mov r8, r4;"
        "mov r9, r4;"
        "mov r10, r4;"
        "mov r11, r4;"
        "mov r12, r4;"
#if !defined(__SOFTFP__) && defined(__ARM_FP) && (__ARM_FP >= 4)
        "ldr.w r0, =0xE000ED88;"
        "ldr r1, [r0];"
        "orr r1, r1, #(0xF << 20);"
        "str r1, [r0];"
        "dsb;"
        "isb;"
        "vmov s0, s1, r4, r5;"
        "vmov s2, s3, r4, r5;"
        "vmov s4, s5, r4, r5;"
        "vmov s6, s7, r4, r5;"
        "vmov s8, s9, r4, r5;"
        "vmov s10, s11, r4, r5;"
        "vmov s12, s13, r4, r5;"
        "vmov s14, s15, r4, r5;"
        "vmov s16, s17, r4, r5;"
        "vmov s18, s19, r4, r5;"
        "vmov s20, s21, r4, r5;"
        "vmov s22, s23, r4, r5;"
        "vmov s24, s25, r4, r5;"
        "vmov s26, s27, r4, r5;"
        "vmov s28, s29, r4, r5;"
        "vmov s30, s31, r4, r5;"
#endif

        "bl system_sens_init;"

#if defined(NOSTD) || defined(__ARMCC_VERSION)
        "ldr r1, =" TO_STRING(__bss_start__) ";"
        "ldr r2, =" TO_STRING(__bss_end__) ";"
        "movs r0, 0;"
        "_loop_bss:;"
        "cmp r1, r2;"
        "itt lt;"
        "strlt r0, [r1], #4;"
        "blt _loop_bss;"
#endif

#if defined(__ARMCC_VERSION) && !defined(NOSTD)
        "bl __rt_entry;"
#else
        "bl _start;"
#endif
    );
}

extern uint32_t __boot_bss_sram_start__[];
extern uint32_t __boot_bss_sram_end__[];

void system_sens_init(void)
{
    uint32_t *dst;

    NVIC_InitVectors();

    hal_senscmu_set_wakeup_vector(SCB->VTOR);

    SystemInit();

    for (dst = __boot_bss_sram_start__; dst < __boot_bss_sram_end__; dst++) {
        *dst = 0;
    }

    hal_senscmu_setup();
}

void system_sens_term(void)
{
}

#endif
#endif

