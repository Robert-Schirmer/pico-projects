#include "hardware/i2c.h"

#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#define SOILMOISTURESENSOR_DEFAULT_ADDR 0x20

// Soil Moisture Sensor Register Addresses
#define SOILMOISTURESENSOR_GET_CAPACITANCE 0x00 // (r) 	2 bytes
#define SOILMOISTURESENSOR_SET_ADDRESS 0x01     // (w) 	1 byte
#define SOILMOISTURESENSOR_GET_ADDRESS 0x02     // (r) 	1 byte
#define SOILMOISTURESENSOR_MEASURE_LIGHT 0x03   // (w) 	n/a
#define SOILMOISTURESENSOR_GET_LIGHT 0x04       // (r) 	2 bytes
#define SOILMOISTURESENSOR_GET_TEMPERATURE 0x05 // (r) 	2 bytes
#define SOILMOISTURESENSOR_RESET 0x06           // (w) 	n/a
#define SOILMOISTURESENSOR_GET_VERSION 0x07     // (r) 	1 bytes
#define SOILMOISTURESENSOR_SLEEP 0x08           // (w)  n/a
#define SOILMOISTURESENSOR_GET_BUSY 0x09        // (r)	1 bytes

typedef struct
{
  i2c_inst_t *i2c_i;
  uint8_t address;
} moisture_sensor_t;

void reset_sensor(moisture_sensor_t *sensor);
int8_t get_sensor_version(moisture_sensor_t *sensor);
int get_sensor_temperature(moisture_sensor_t *sensor);

int get_sensor_light(moisture_sensor_t *sensor);

int get_sensor_capacitance(moisture_sensor_t *sensor);

int is_sensor_busy(moisture_sensor_t *sensor);

#endif
