#include "i2c.h"
#include <stm32l475xx.h>


void i2c_init(){
    //enable the clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;

    //clear the bits
    GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    //set to alternate function mode
    GPIOB->MODER |= GPIO_MODER_MODE10_1;
    GPIOB->MODER |= GPIO_MODER_MODE11_1;

    GPIOB->OTYPER |= (GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11);

    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0);
    //set to AF4
    GPIOB->AFR[1] |= GPIO_AFRH_AFSEL10_2;
    GPIOB->AFR[1] |= GPIO_AFRH_AFSEL11_2;



    //configure baud rate to 400kHz
    // I2C2->TIMINGR = ?;
    //set I2C2SEL to SYSCLK?
    RCC->CCIPR &= ~RCC_CCIPR_I2C2SEL;       // Clear I2C2SEL
    RCC->CCIPR |= RCC_CCIPR_I2C2SEL_0;        // Set I2C2SEL to 01
    I2C2->TIMINGR |= (1U << I2C_TIMINGR_PRESC_Pos); // Set PRESC to 1
    //tI2CCLK: 1/(4*10^6) = 250ns
    //target: 400KHz = 1/(400*10^3) = 2500ns
    //2500ns/250ns = 10
    //SDADEL:5  SCLDEL:5   5*250ns = 1250ns  250ns<1250ns
    I2C2->TIMINGR |= (5U << I2C_TIMINGR_SCLDEL_Pos); // Set SCLDEL to 5
    I2C2->TIMINGR |= (5U << I2C_TIMINGR_SDADEL_Pos); // Set SDADEL to 5

    //enable
    I2C2->CR1 |= I2C_CR1_PE;
}


uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len){
    //set number of bytes to send
	I2C2->CR2 &= ~I2C_CR2_NBYTES;
    I2C2->CR2 |= (len << I2C_CR2_NBYTES_Pos);
    //set slave address
    I2C2->CR2 &= ~I2C_CR2_ADD10; //7-bit address
    I2C2->CR2 &= ~I2C_CR2_SADD; //clear address
    I2C2->CR2 |= ((address & 0x7F) << 1);  //0x7F: 0111 1111

    // //set dir  TODO: need to change this cuz read needs repeated START
    // if(dir == 0){ //write
    //     I2C2->CR2 &= ~I2C_CR2_RD_WRN;
    // }
    // else{ //read
    //     I2C2->CR2 |= I2C_CR2_RD_WRN;
    // }
    I2C2->CR2 |= I2C_CR2_RD_WRN;
    I2C2->CR2 &= ~I2C_CR2_RD_WRN; //write bit

    //start transaction
    I2C2->CR2 |= I2C_CR2_START;

    //NACK

    //I2C_ISR_TXIS
    if(dir == 1){ //read
        //sending register address
        //TODO: set START bit to 0?
//        while((I2C2->ISR & I2C_ISR_TXE) == 0);
//        I2C2->TXDR = data[0];
    	printf("flag:%d \n", I2C2->ISR & I2C_ISR_RXNE);
        while((I2C2->ISR & I2C_ISR_NACKF) != 1){    //if NACK is not received
        	if ((I2C2->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS) {
        		continue;
        	}
            I2C2->TXDR = data[0];
            printf("readregisteraddr:%d\n", I2C2->TXDR);

            //set to read bit
//            I2C2->CR2 |= I2C_CR2_STOP;

            I2C2->CR2 |= (I2C_CR2_START | I2C_CR2_RD_WRN);

            for(uint8_t i = 1; i <= len; i++){
                //wait until flag is 1(which means the data register RXDR is not empty)
//            	I2C2->ISR &= I2C_ISR_RXNE;
            	if ((I2C2->ISR & I2C_ISR_NACKF) == 0){
            		while((I2C2->ISR & I2C_ISR_RXNE) == 0){
            			printf("here!\n");
            			continue;
//            			break;
            		}
            	}



            	printf("flag1:%d\n", I2C2->ISR & I2C_ISR_RXNE);
            	data[i] = I2C2->RXDR;
            	uint8_t val = I2C2->RXDR;

            	printf("%d\n",val);
//                while((I2C2->ISR & I2C_ISR_RXNE) != I2C_ISR_RXNE);
//                data[i] = I2C2->RXDR;
            }
            break;
        }
    }
    else{

        for(uint8_t i = 0; i < len; i++){
            //wait until flag is 1(which means the data register TXDR is empty)
//            while((I2C2->ISR & I2C_ISR_TXIS) == 0);
            while((I2C2->ISR & I2C_ISR_NACKF) != I2C_ISR_NACKF){
            	//if NACK is not received
            	if ((I2C2->ISR & I2C_ISR_TXIS) != I2C_ISR_TXIS) {
            		continue;
            	}
            	break;
            }
            I2C2->TXDR = data[i];

            // andy
//            while ((I2C2->ISR & I2C_ISR_TC) != I2C_ISR_TC); // waits for it to complete...

            uint8_t value = I2C2->TXDR;
            printf("writing:%d\n", value);
        }
    }

    //stop transaction
    I2C2->CR2 |= I2C_CR2_STOP;

    return 1;
}
