/*
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <soc.h>

/*
 * Zephyr has no "reboot to bootloader" on STM32: &bootloader only does a
 * plain reset. So, as the original keymini firmware does, store a magic
 * value in RAM that survives the reset, and jump to the ST system memory
 * bootloader (USB DFU) at the very start of the next boot, before any
 * clock or peripheral is configured.
 */

/* RST_UF2 from ZMK dt-bindings/zmk/reset.h, the type used by &bootloader */
#define RST_UF2 0x57

#define SYSTEM_MEMORY_ADDR 0x1FFFC800
#define GOTO_BOOTLOADER_MAGIC 0xB007B007

static __noinit uint32_t goto_bootloader;

void sys_arch_reboot(int type) {
    goto_bootloader = type == RST_UF2 ? GOTO_BOOTLOADER_MAGIC : 0;
    NVIC_SystemReset();
}

/* Called first thing by the reset handler (CONFIG_PLATFORM_SPECIFIC_INIT) */
void z_arm_platform_init(void) {
    bool software_reset = RCC->CSR & RCC_CSR_SFTRSTF;
    bool jump = software_reset && goto_bootloader == GOTO_BOOTLOADER_MAGIC;

    goto_bootloader = 0;
    if (!jump) {
        return;
    }

    const uint32_t *vector_table = (const uint32_t *)SYSTEM_MEMORY_ADDR;

    __asm__ volatile("msr msp, %0\n"
                     "bx %1\n" ::"r"(vector_table[0]),
                     "r"(vector_table[1]));
}
