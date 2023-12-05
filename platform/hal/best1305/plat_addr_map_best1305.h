/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __PLAT_ADDR_MAP_BEST1305_H__
#define __PLAT_ADDR_MAP_BEST1305_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_BASE                                0x28020000
#define ROMX_BASE                               0x00020000

#ifndef ROM_SIZE
#define ROM_SIZE                                0x00008000
#endif
#define ROM_EXT_SIZE                            0x00008000

#define PATCH_ENTRY_NUM                         8
#define PATCH_CTRL_BASE                         0x00030000
#define PATCH_DATA_BASE                         0x00030020

#define RAM0_BASE                               0x20000000
#define RAMX0_BASE                              0x00200000
#define RAM1_BASE                               0x20020000
#define RAMX1_BASE                              0x00220000
#define RAM2_BASE                               0x20040000
#define RAMX2_BASE                              0x00240000
#define RAM3_BASE                               0x20050000
#define RAMX3_BASE                              0x00250000
#define RAM4_BASE                               0x20060000
#define RAMX4_BASE                              0x00260000
#define RAM5_BASE                               0x20068000
#define RAMX5_BASE                              0x00268000
#define RAM6_BASE                               0x20070000
#define RAMX6_BASE                              0x00270000
#define RAM_BASE                                RAM0_BASE
#define RAMX_BASE                               RAMX0_BASE

#define RAM6_SIZE                               0x00010000

#define RAM_TOTAL_SIZE                          (RAM6_BASE + RAM6_SIZE - RAM0_BASE)

#ifdef CHIP_HAS_CP
#define RAMCP_TOP                               (RAM6_BASE + RAM6_SIZE)

#ifndef RAMCP_SIZE
#ifdef UNIFY_HEAP_ENABLED
#define RAMCP_SIZE                              ((RAM6_BASE - RAM4_BASE) + RAM6_SIZE)
#else
#define RAMCP_SIZE                              ((RAM6_BASE - RAM5_BASE) + RAM6_SIZE)
#endif
#endif
#define RAMCP_BASE                              (RAMCP_TOP - RAMCP_SIZE)

#ifndef RAMCPX_SIZE
#define RAMCPX_SIZE                             (RAMX4_BASE - RAMX3_BASE)
#endif
#ifdef UNIFY_HEAP_ENABLED
#define RAMCPX_BASE                             (RAM_TO_RAMX(RAMCP_BASE) - RAMCPX_SIZE)
#else
#define RAMCPX_BASE                             RAMX3_BASE
#endif
#endif

#ifndef MEM_POOL_SIZE
#define MEM_POOL_SIZE                           (RAM3_BASE - RAM2_BASE)
#endif

#ifndef FAST_XRAM_SECTION_SIZE
#define FAST_XRAM_SECTION_SIZE                  (RAMX5_BASE - RAMX4_BASE)
#endif

#define FRAMX_BASE                              RAMX4_BASE

#ifndef RAM_SIZE
#ifdef CHIP_HAS_CP
#ifdef UNIFY_HEAP_ENABLED
#define RAM_SIZE                                (RAMCPX_BASE - RAMX_BASE)
#else
#define RAM_SIZE                                (RAMX3_BASE - RAMX_BASE)
#endif
#else /* !CHIP_HAS_CP */
#define RAM_SIZE                                (RAM6_BASE + RAM6_SIZE - RAM_BASE)
#endif /* !CHIP_HAS_CP */
#endif /* !RAM_SIZE */

#define FLASH_BASE                              0x3C000000
#define FLASH_NC_BASE                           0x38000000
#define FLASHX_BASE                             0x0C000000
#define FLASHX_NC_BASE                          0x08000000

#define ICACHE_CTRL_BASE                        0x07FFC000
/* No data cache */

#define CMU_BASE                                0x40000000
#define MCU_WDT_BASE                            0x40001000
#define MCU_TIMER0_BASE                         0x40002000
#define MCU_TIMER1_BASE                         0x40003000
#define I2C0_BASE                               0x40005000
#define I2C1_BASE                               0x40006000
#define SPI_BASE                                0x40007000
#define ISPI_BASE                               0x40009000
#define UART0_BASE                              0x4000B000
#define UART1_BASE                              0x4000C000
#define BTPCM_BASE                              0x4000E000
#define I2S0_BASE                               0x4000F000
#define I2C2_BASE                               0x40032000

#define AON_CMU_BASE                            0x40080000
#define AON_GPIO_BASE                           0x40081000
#define AON_WDT_BASE                            0x40082000
#define AON_PWM_BASE                            0x40083000
#define AON_TIMER_BASE                          0x40084000
#define AON_PSC_BASE                            0x40085000
#define AON_IOMUX_BASE                          0x40086000

#define AUDMA_BASE                              0x40120000
#define GPDMA_BASE                              0x40130000
#define FLASH_CTRL_BASE                         0x40140000
#define BTDUMP_BASE                             0x40150000
#define I2C_SLAVE_BASE                          0x40160000

#define CODEC_BASE                              0x40300000

#define BT_SUBSYS_BASE                          0xA0000000
#define BT_RAM_BASE                             0xC0000000
#define BT_RAM_SIZE                             0x00008000
#define BT_EXCH_MEM_BASE                        0xD0210000
#define BT_EXCH_MEM_SIZE                        0x00008000
#define BT_UART_BASE                            0xD0300000
#define BT_CMU_BASE                             0xD0330000

#define IOMUX_BASE                              AON_IOMUX_BASE
#define GPIO_BASE                               AON_GPIO_BASE
#define PWM_BASE                                AON_PWM_BASE
#ifdef CORE_SLEEP_POWER_DOWN
#define TIMER0_BASE                             AON_TIMER_BASE
#else
#define TIMER0_BASE                             MCU_TIMER0_BASE
#endif
#define TIMER1_BASE                             MCU_TIMER1_BASE
#define WDT_BASE                                AON_WDT_BASE

/* For linker scripts */

#define VECTOR_SECTION_SIZE                     256
#define REBOOT_PARAM_SECTION_SIZE               64
#define ROM_BUILD_INFO_SECTION_SIZE             40
#define ROM_EXPORT_FN_SECTION_SIZE              128
#define BT_INTESYS_MEM_OFFSET                   0x00000000

/* For module features */
#define CODEC_FREQ_CRYSTAL                      CODEC_FREQ_26M
#define CODEC_FREQ_EXTRA_DIV                    2
#define CODEC_PLL_DIV                           16
#define CODEC_CMU_DIV                           8
#define CODEC_PLAYBACK_BIT_DEPTH                24
#define CODEC_HAS_FIR
#define GPADC_CTRL_VER                          3
#define GPADC_VALUE_BITS                        16
#define DCDC_CLOCK_CONTROL
#ifndef AUD_SECTION_STRUCT_VERSION
#define AUD_SECTION_STRUCT_VERSION              2
#endif

/* For boot struct version */
#ifndef SECURE_BOOT_VER
#define SECURE_BOOT_VER                         3
#endif

#ifdef __cplusplus
}
#endif

#endif
