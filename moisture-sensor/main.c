#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "pico/binary_info.h"
#include "moisture_sensor.h"

enum
{
    CELCIUS,
    FAHRENHEIT
};
void print_temperature(int tenths_celcius, int unit);

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

    sleep_ms(5000);

    moisture_sensor_t sensor = {i2c, SOILMOISTURESENSOR_DEFAULT_ADDR};

    uint8_t sensor_version = get_sensor_version(&sensor);
    if (sensor_version == -1)
    {
        printf("Failed to get sensor version\n");
        return 1;
    }

    printf("Sensor version: %d\n", sensor_version);

    reset_sensor(&sensor);
    // Wait for the sensor to reset
    sleep_ms(1000);

    while (true)
    {
        int busy = is_sensor_busy(&sensor);
        printf("Sensor busy: %d\n", busy);

        int temp = get_sensor_temperature(&sensor);

        print_temperature(temp, FAHRENHEIT);

        int capacitence = get_sensor_capacitance(&sensor);
        printf("Capacitence: %d\n", capacitence);

        sleep_ms(1000);
    }
}

void print_temperature(int tenths_celcius, int unit)
{
    int temp_tenths = unit == CELCIUS ? tenths_celcius : (tenths_celcius * 9) / 5 + 320;
    char unit_char = unit == CELCIUS ? 'C' : 'F';

    printf("Temperature: %d.%d°%c\n", temp_tenths / 10, temp_tenths % 10, unit_char);
}
