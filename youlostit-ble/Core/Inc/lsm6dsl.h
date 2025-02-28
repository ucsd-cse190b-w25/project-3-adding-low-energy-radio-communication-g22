#ifndef LSM6DSL_H
#define LSM6DSL_H

#include <stdint.h>

void lsm6dsl_init();
void lsm6dsl_read_xyz(int16_t* x, int16_t* y, int16_t* z); 

#endif