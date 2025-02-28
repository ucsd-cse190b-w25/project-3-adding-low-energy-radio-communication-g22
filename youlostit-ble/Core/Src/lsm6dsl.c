#include "lsm6dsl.h"
#include "i2c.h"

//register address in data?
//when read, do repeated start after sending register address?
//readxyz right?
//sysclock 4MHz?

void lsm6dsl_init(){
    //LSM6DSL addr: 0x6A
    uint8_t ctrl1_xl[2] = {0x10, 0x60};  //CTRL1_XL addr: 0x10, value: 0x60
    i2c_transaction(0x6A, 0, ctrl1_xl, 2);

    uint8_t int1_ctrl[2] = {0x0D, 0x01};  //INT1_CTRL addr: 0x0D, value: 0x01
    i2c_transaction(0x6A, 0, int1_ctrl, 2);

//    uint8_t read_value[2] = {0x10, 0x00};
//    i2c_transaction(0x6A, 1, &read_value, 1);
//    printf("setupreadingreg: %d\n", read_value[1]);
}

void lsm6dsl_read_xyz(int16_t* x, int16_t* y, int16_t* z){
	uint8_t status[2] = {0x1E, 0};
    uint8_t outx_l_xl[2] = {0x28, 0}; //OUTX_L_XL addr: 0x28
    uint8_t outx_h_xl[2] = {0x29, 0}; //OUTX_H_XL addr: 0x29    //H: MSB, L: LSB
    uint8_t outy_l_xl[2] = {0x2A, 0}; //OUTY_L_XL addr: 0x2A
    uint8_t outy_h_xl[2] = {0x2B, 0}; //OUTY_H_XL addr: 0x2B
    uint8_t outz_l_xl[2] = {0x2C, 0}; //OUTZ_L_XL addr: 0x2C
    uint8_t outz_h_xl[2] = {0x2D, 0}; //OUTZ_H_XL addr: 0x2D

    i2c_transaction(0x6A, 1, &status, 1);
    printf("status: %d\n", status[1]);
    while(status[2] & 0x01 == 0){
    	i2c_transaction(0x6A, 1, &status, 1);
        printf("status: %d\n", status[1]);
    }


    i2c_transaction(0x6A, 1, outx_l_xl, 1);
    i2c_transaction(0x6A, 1, outx_h_xl, 1);
    i2c_transaction(0x6A, 1, outy_l_xl, 1);
    i2c_transaction(0x6A, 1, outy_h_xl, 1);
    i2c_transaction(0x6A, 1, outz_l_xl, 1);
    i2c_transaction(0x6A, 1, outz_h_xl, 1);

    printf("reading z1 %d \n" ,outz_l_xl[1]);
    printf("reading z2 %d \n" ,outz_h_xl[1]);

    *x = (outx_h_xl[1] << 8) | outx_l_xl[1];
    *y = (outy_h_xl[1] << 8) | outy_l_xl[1];
    *z = (outz_h_xl[1] << 8) | outz_l_xl[1];
}

