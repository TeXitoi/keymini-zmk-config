/*
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/linker/section_tags.h>
#include <soc.h>

/*
 * Zephyr has no "reboot to bootloader" on STM32: &bootloader only does a
 * plain reset. So, as the original keymini firmware does, store a magic
 * value in RAM that survives the reset, and jump to the ST system memory
 * bootloader (USB DFU) at the very start of the next boot, before any
 * clock or peripheral is configured.
 *
 * Also, `dfu-util :leave` makes the ST bootloader jump to the firmware
 * without a reset, leaving some of its state behind: ZMK then hangs. As
 * this state can't be reliably detected, ZMK only boots after a reset done
 * by itself, marked by another magic value: any other boot first resets.
 */

/* RST_UF2 from ZMK dt-bindings/zmk/reset.h, the type used by &bootloader */
#define RST_UF2 0x57

#define SYSTEM_MEMORY_ADDR 0x1FFFC800
#define GOTO_BOOTLOADER_MAGIC 0xB007B007
#define CLEAN_BOOT_MAGIC 0xB007C1EA

static __noinit uint32_t boot_magic;

static void reset_with(uint32_t magic) {
    boot_magic = magic;
    NVIC_SystemReset();
}

void sys_arch_reboot(int type) {
    reset_with(type == RST_UF2 ? GOTO_BOOTLOADER_MAGIC : CLEAN_BOOT_MAGIC);
}

/* Called first thing by the reset handler (CONFIG_PLATFORM_SPECIFIC_INIT) */
void z_arm_platform_init(void) {
    uint32_t magic = boot_magic;

    boot_magic = 0;
    if (magic == CLEAN_BOOT_MAGIC) {
        return;
    }
    if (magic != GOTO_BOOTLOADER_MAGIC) {
        reset_with(CLEAN_BOOT_MAGIC);
    }

    const uint32_t *vector_table = (const uint32_t *)SYSTEM_MEMORY_ADDR;

    __asm__ volatile("msr msp, %0\n"
                     "bx %1\n" ::"r"(vector_table[0]),
                     "r"(vector_table[1]));
}
