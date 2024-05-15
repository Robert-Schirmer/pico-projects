/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Sweep through all 7-bit I2C addresses, to see if any slaves are present on
// the I2C bus. Print out a table that looks like this:
//
// I2C Bus Scan
//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
// 0
// 1       @
// 2
// 3             @
// 4
// 5
// 6
// 7
//
// E.g. if slave addresses 0x12 and 0x34 were acknowledged.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "pico/binary_info.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
// All 7-bit addresses should be greater than 0x07 and less than 0x78 (120)
bool reserved_addr(uint8_t addr)
{
    return (addr <= 0x07) || (addr >= 0x78);
    // return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main()
{
    stdio_init_all();

    cyw43_arch_init();

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    i2c_inst_t *i2c = i2c0;
    const uint sda_pin = 16;
    const uint scl_pin = 17;

    i2c_init(i2c, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(sda_pin, scl_pin, GPIO_FUNC_I2C));

    sleep_ms(2000);

    while (true)
    {
        printf("\nI2C Bus Scan\n");
        printf("    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n");
        for (int addr = 0; addr < (1 << 7); ++addr)
        {
            if (addr % 16 == 0)
            {
                printf("%02x ", addr);
            }

            int ret = 0;
            int8_t rxdata;
            // Skip over any reserved addresses.
            if (reserved_addr(addr))
            {
                ret = -9;
            }
            else
            {
                ret = i2c_read_timeout_us(i2c, addr, &rxdata, 1, false, 1 * 1000 * 1000);
            }
            if (ret < 0) {
                printf("%d", ret);
            } else {
                printf("OK");
            }
            printf(addr % 16 == 15 ? "\n" : "  ");
        }
        printf("Scanning Done.\n");
        sleep_ms(5000);
    }
}
