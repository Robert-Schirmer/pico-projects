#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <stdio.h>
#include "moisture_sensor.h"

#if 0
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, uint8_t *src, size_t len, char *name)
{
  switch (i2c_write_blocking(i2c, addr, src, len, false))
  {
  case PICO_ERROR_GENERIC:
    DEBUG_printf("[moisture_sensor][%s] addr not acknowledged!\n", name);
    break;
  case PICO_ERROR_TIMEOUT:
    DEBUG_printf("[moisture_sensor][%s] timeout!\n", name);
    break;
  default:
    // DEBUG_printf("[%s] wrote successfully %lu bytes!\n", name, len);
    break;
  }
}

inline static int fancy_read(i2c_inst_t *i2c, uint8_t addr, uint8_t *reg, size_t len, uint8_t *buf, char *name)
{
  i2c_write_blocking(i2c, addr, reg, 1, true);

  int num_bytes_read = i2c_read_blocking(i2c, addr, buf, len, false);

  DEBUG_printf("[moisture_sensor][%s] read %d bytes\n", name, num_bytes_read);

  return num_bytes_read;
}

/**
 * Give the sensor 0.5-1 second time to boot up after a reset
 */
void reset_sensor(moisture_sensor_t *sensor)
{
  uint8_t reg = SOILMOISTURESENSOR_RESET;

  fancy_write(sensor->i2c_i, sensor->address, &reg, 1, "reset");
}

int8_t get_sensor_version(moisture_sensor_t *sensor)
{
  uint8_t reg = SOILMOISTURESENSOR_GET_VERSION;
  uint8_t buf[1] = {0};

  int num_bytes_read = fancy_read(sensor->i2c_i, sensor->address, &reg, 1, buf, "get_version");

  if (num_bytes_read == 1)
  {
    return buf[0];
  }

  return -1;
}

/**
 * Returns the temperature in tenths of degress Celsius
 */
int get_sensor_temperature(moisture_sensor_t *sensor)
{
  uint8_t reg = SOILMOISTURESENSOR_GET_TEMPERATURE;
  uint8_t buf[2] = {0};

  int num_bytes_read = fancy_read(sensor->i2c_i, sensor->address, &reg, 2, buf, "get_temperature");

  if (num_bytes_read == 2)
  {
    return (buf[0] << 8) | buf[1];
  }

  return -1;
}

/**
 * Light sensor currently not working
 */
int get_sensor_light(moisture_sensor_t *sensor)
{
  uint8_t reg1 = SOILMOISTURESENSOR_MEASURE_LIGHT;

  i2c_write_blocking(sensor->i2c_i, sensor->address, &reg1, 1, false);
  sleep_ms(3000);

  uint8_t reg2 = SOILMOISTURESENSOR_GET_LIGHT;
  uint8_t buf[2] = {0};

  i2c_write_blocking(sensor->i2c_i, sensor->address, &reg2, 1, true);
  int num_bytes_read = i2c_read_blocking(sensor->i2c_i, sensor->address, buf, 2, false);

  if (num_bytes_read == 2)
  {
    return (buf[0] << 8) | buf[1];
  }

  return -1;
}

int is_sensor_busy(moisture_sensor_t *sensor)
{
  uint8_t reg = SOILMOISTURESENSOR_GET_BUSY;
  uint8_t buf[1] = {0};

  int num_bytes_read = fancy_read(sensor->i2c_i, sensor->address, &reg, 1, buf, "is_busy");

  if (num_bytes_read == 1)
  {
    return buf[0];
  }

  return -1;
}

int get_sensor_capacitance(moisture_sensor_t *sensor)
{
  uint8_t reg = SOILMOISTURESENSOR_GET_CAPACITANCE;
  uint8_t buf[2] = {0};

  int num_bytes_read = fancy_read(sensor->i2c_i, sensor->address, &reg, 2, buf, "get_capacitance");

  if (num_bytes_read == 2)
  {
    return (buf[0] << 8) | buf[1];
  }

  return -1;
}
