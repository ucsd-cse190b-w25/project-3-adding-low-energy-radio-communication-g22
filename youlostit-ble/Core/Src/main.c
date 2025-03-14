/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
//#include "ble_commands.h"
#include "ble.h"

#include "timer.h"
#include "i2c.h"
#include "lsm6dsl.h"


#include <stdlib.h>
#include <stdbool.h>

int dataAvailable = 0;

SPI_HandleTypeDef hspi3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI3_Init(void);


volatile int16_t counter1 = 0;
volatile bool isLost = false;
// change mins to seconds
volatile int32_t seconds = 0;
volatile int32_t prevSecond = -1;



/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI3_Init();

  //RESET BLE MODULE
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port,BLE_RESET_Pin,GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port,BLE_RESET_Pin,GPIO_PIN_SET);

  ble_init();

  i2c_init();
  lsm6dsl_init();
  timer_init(TIM2);

  HAL_Delay(10);

  uint8_t nonDiscoverable = 0;

  while (1)
  {

	  if(!nonDiscoverable && HAL_GPIO_ReadPin(BLE_INT_GPIO_Port,BLE_INT_Pin)){
	  		    catchBLE();
	  	}
	  if(!isLost) {
//		  discoveraiblity is false
		  disconnectBLE();
		  setDiscoverability(0);
	      standbyBle();
	  }

	  if (isLost){
		  setDiscoverability(1);
//		  set disoveraibiltihut back to true
//			  use own timer...to count up.
//			  HAL_Delay(1000); // should i pass in 10000?
//			  maybe use a variable to keep track of previous second and see if it changed
			  // Send a string to the NORDIC UART service, remember to not include the newline
//			  pass in a value for <N>
//			  unsigned char test_str[30]; // Ensure it's large enough
//			    sprintf((char*)test_str, "AK missing %d seconds", seconds);
			  unsigned char test_str[20];
			  snprintf((char *)test_str, sizeof(test_str) - 1, "AK: Lost %d", seconds);
			  test_str[sizeof(test_str) - 1] = '\0'; // Ensure null termination


			  if (prevSecond != seconds){
				  updateCharValue(NORDIC_UART_SERVICE_HANDLE, READ_CHAR_HANDLE, 0, strlen((char *)test_str), test_str);
	 			  prevSecond = seconds;
			  }
	  }

	  else{

	  // Wait for interrupt, only uncomment if low power is needed
	  //__WFI();


//		// to get this to work, need extI from accel.
//	    // TODO Clear LPMS bits to set them to "000” (Stop mode)
//	    PWR->CR1 &= ~PWR_CR1_LPMS_Msk;   // Clears only the LPMS bits
//	    PWR->CR1 |= PWR_CR1_LPMS_STOP0;  // Sets STOP 0 mode

//		less power efficient, but can use something other than stop0
			PWR->CR1 &= ~PWR_CR1_LPR_Msk;
		PWR->CR1 |= PWR_CR1_LPR;

	    // Prepare to enter deep sleep mode (Stop mode)
	    // Set the SLEEPDEEP bit in the System Control Register
//	    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	    HAL_SuspendTick(); // Stop the HAL tick timer

	    // Execute the Wait-For-Interrupt instruction.
	    // This puts the CPU into deep sleep mode until an interrupt occurs.
	    __asm volatile ("wfi"); // same as __WFI();

	    HAL_ResumeTick(); // Restart the HAL tick timer

	    // After waking up, clear the SLEEPDEEP bit if you plan to return to a lighter sleep mode
//	    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;

//	    external interrupt from accelerometer
//	    accelermeter can trigger interrupt. wake up when value changes.

//	    lsi or lse we can use stop? bc those run all the time?
	  }
  }
}

/**
  * @brief System Clock Configuration
  * @attention This changes the System clock frequency, make sure you reflect that change in your timer
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  // This lines changes system clock frequency
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_7;

//  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}


void TIM2_IRQHandler()   //changed to fire once every 10 seconds
{
	if(TIM2->SR & TIM_SR_UIF){
	    TIM2->SR &= ~TIM_SR_UIF;

	    int16_t x = 0;
	    int16_t y = 0;
	    int16_t z = 0;
	    lsm6dsl_read_xyz(&x, &y, &z);
	    int magnitude = (int32_t)(x*x) + (int32_t)(y*y) + (int32_t)(z*z);
	    if(magnitude <= 1200119301){  //1000119301
	    	counter1+=1;
	    }
	    else{
	    	counter1 = 0;
	    	seconds = 0;
	    	prevSecond = -1;
	    	isLost = false;
	    }

		  // if (counter1 >= 1200 && counter1 %1200 == 0){ //1200
			//   isLost = true;
		  // }
		  // if (isLost && (counter1-1200) %200 == 0) {
			//   seconds = (counter1-1200)/20;
		  // }

      if (counter1 >= 6){   //every 10 seconds
			  isLost = true;
		  }
      seconds = counter1*10;
//		  if (isLost) {
//			  seconds = counter1*10;
//		  }
//		  if (counter1 >= 500 && counter1 %500 == 0){ //1200
//			  isLost = true;
//		  }
//		  if (isLost && (counter1-1200) %200 == 0) {
//			  seconds = (counter1-1200)/200;
//		  }
	}

}


/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_LED1_GPIO_Port, GPIO_LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);


  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : BLE_INT_Pin */
  GPIO_InitStruct.Pin = BLE_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_LED1_Pin BLE_RESET_Pin */
  GPIO_InitStruct.Pin = GPIO_LED1_Pin|BLE_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_CS_Pin */
  GPIO_InitStruct.Pin = BLE_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(BLE_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}




#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
