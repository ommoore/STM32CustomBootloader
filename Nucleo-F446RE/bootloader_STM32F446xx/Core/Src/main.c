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
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//enable this line to get debug messages over debug uart
//#define BL_DEBUG_MSG_EN

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

#define D_UART &huart3
#define C_UART &huart2

uint8_t supported_commands[] = {
																BL_GET_VER, 
																BL_GET_HELP,
																BL_GET_CID,
																BL_GET_RDP_STATUS,
																BL_GO_TO_ADDR, 
																BL_FLASH_ERASE, 
																BL_MEM_WRITE, 
																BL_READ_SECTOR_P_STATUS };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_USART3_UART_Init(void);
																
/* USER CODE BEGIN PFP */
static void printmsg(char *format, ...);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char somedata[] = "Hello from Bootloader\r\n";

#define BL_RX_LEN 200
uint8_t bl_rx_buffer[BL_RX_LEN];

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_CRC_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	
	if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
		printmsg("BL_DEBUG_MSG: Button is pressed .. going to BL mode\n\r");
		//we should continue in bootloader mode
		bootloader_uart_read_data();
		
	} else {
		printmsg("BL_DEBUG_MSG: Button is not pressed .. executing user app\n\r");
		bootloader_jump_to_user_app();
	}
}

void bootloader_uart_read_data(void) {
	
	uint8_t rcv_len = 0;
	
	while(1) {
		memset(bl_rx_buffer, 0, 200);
		
		//here we will read and decode the commands coming from host
		
		//Read first byte (Length) of data packet
		HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY);
		rcv_len = bl_rx_buffer[0];
		
		//Read the rest of the data packet
		HAL_UART_Receive(C_UART, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY);
		
		//command codes
		switch(bl_rx_buffer[1]) {
			case BL_GET_VER:
				bootloader_handle_getver(bl_rx_buffer);
				break;
			
			case BL_GET_HELP:
				bootloader_handle_gethelp(bl_rx_buffer);
				break;
			
			case BL_GET_CID:
				bootloader_handle_getcid(bl_rx_buffer);
				break;
			
			case BL_GET_RDP_STATUS:
				bootloader_handle_getrdp(bl_rx_buffer);
				break;
			
			case BL_GO_TO_ADDR:
				bootloader_handle_go_to_addr(bl_rx_buffer);
				break;
			
			case BL_FLASH_ERASE:
				bootloader_handle_flash_erase(bl_rx_buffer);
				break;
			
			case BL_MEM_WRITE:
				bootloader_handle_mem_write(bl_rx_buffer);
				break;
					
			case BL_EN_RW_PROTECT:
				bootloader_handle_en_rw_protect(bl_rx_buffer);
				break;
			
			case BL_DIS_RW_PROTECT:
				bootloader_handle_dis_rw_protect(bl_rx_buffer);
				break;
			
			case BL_MEM_READ:
				bootloader_handle_mem_read(bl_rx_buffer);
				break;
			
			case BL_READ_SECTOR_P_STATUS:
				bootloader_handle_read_sector_protection_status(bl_rx_buffer);
				break;
						
			case BL_OTP_READ:
				bootloader_handle_read_otp(bl_rx_buffer);
				break;
									
			default:
				printmsg("BL_DEBUG_MSG: Invalid command code received from host\n\r");
				break;
		}	
	}
}

void bootloader_jump_to_user_app(void) {
	//just a function pointer to hold the address of the reset handler of the user app
	void (*app_reset_handler)(void);
	printmsg("BL_DEBUG_MSG: bootloader_jump_to_user_app\n\r");
	
	//1. configure the MSP by reading the value from the base address of Sector 2
	uint32_t msp_value = *(volatile uint32_t*)FLASH_SECTOR2_BASE_ADDRESS;
	printmsg("BL_DEBUG_MSG: MSP value : %#x\n\r", msp_value);
	
	//This function comes from CMSIS
	__set_MSP(msp_value);
	
	//SCB->VTOR = FLASH_SECTOR1_BASE_ADDRESS
	
	//2. Now fetch the reset handler address of the user application from the location FLASH_SECTOR2_BASE_ADDRESS+4
	uint32_t resethandler_address = *(volatile uint32_t*)(FLASH_SECTOR2_BASE_ADDRESS + 4);
	app_reset_handler = (void*)resethandler_address;
	printmsg("BL_DEBUG_MSG: app reset handler addr : %#x\n\r", app_reset_handler);
	
	//3. jump to the reset handler of the user application
	app_reset_handler();
	
}

/* prints formatted string to console over UART */
void printmsg(char *format, ...) {
#ifdef BL_DEBUG_MSG_EN
	char str[80];
	
	/* Extract the argument list using VA APIs */
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
	HAL_UART_Transmit(D_UART, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
	va_end(args);
#endif
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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

/******************** Implementation of Bootloader Command Handle Functions ********************/

/* Helper function to handle BL_GET_VER command */
void bootloader_handle_getver(uint8_t *bl_rx_buffer) {
	uint8_t bl_version;
	
	//1. Verify the Checksum
	printmsg("BL_DEBUG_MSG: bootloader_handle_getver\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the host
	uint32_t host_crc = *((uint32_t *) (bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		//checksum is correct
		bootloader_send_ack(bl_rx_buffer[0], 1);
		bl_version = get_bootloader_version();
		printmsg("BL_DEBUG_MSG: BL_VER: %d %#x\n\r", bl_version, bl_version);
		bootloader_uart_write_data(&bl_version, 1);
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		//checksum is wrong
		bootloader_send_nack();
	}
}

/* Helper function to handle BL_GET_HELP command */
void bootloader_handle_gethelp(uint8_t *bl_rx_buffer) {
	
	printmsg("BL_DEBUG_MSG: bootloader_handle_gethelp\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *) (bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], sizeof(supported_commands));
		bootloader_uart_write_data(supported_commands, sizeof(supported_commands));
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		bootloader_send_nack();
	}
}

/* Helper function to handle BL_GET_CID command */
void bootloader_handle_getcid(uint8_t *bl_rx_buffer) {
	
	uint16_t bl_cid = 0;
	printmsg("BL_DEBUG_MSG: bootloader_handle_getcid\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *) (bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 2);
		bl_cid = get_mcu_chip_id();
		printmsg("BL_DEBUG_MSG: MCU id: #d %#x !!\n\r", bl_cid, bl_cid);
		bootloader_uart_write_data((uint8_t *)&bl_cid, 2);
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		bootloader_send_nack();
	}
}

/* Helper function to handle BL_GET_RDP_STATUS command */
void bootloader_handle_getrdp(uint8_t *bl_rx_buffer) {
	
	uint8_t rdp_level = 0x00;
	printmsg("BL_DEBUG_MSG: bootloader_handle_getrdp\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *) (bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 1);
		rdp_level = get_flash_rdp_level();
		printmsg("BL_DEBUG_MSG: RDP Level: #d %#x !!\n\r", rdp_level, rdp_level);
		bootloader_uart_write_data(&rdp_level, 1);
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		bootloader_send_nack();
	}
}

/* Helper function to handle BL_GO_TO_ADDR command */
void bootloader_handle_go_to_addr(uint8_t *bl_rx_buffer) {
	
	uint32_t go_to_address = 0;
	uint8_t addr_valid = ADDR_VALID;
	uint8_t addr_invalid = ADDR_INVALID;
	
	printmsg("BL_DEBUG_MSG: bootloader_handle_go_to_addr\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *) (bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 1);
		
		//Extract the Go-To Address
		go_to_address = *((uint32_t *)&bl_rx_buffer[2]);
		printmsg("BL_DEBUG_MSG: Go-To Addr: %#x !!\n\r", go_to_address);
		
		if(verify_address(go_to_address) == ADDR_VALID) {
			//Inform host that address is valid
			bootloader_uart_write_data(&addr_valid,1);

			/* jump to "go-to" address.
			The host must ensure that valid code is present at the destination
			It is not the duty of the bootloader to verify that, so just trust it is correct and jump */

			/* Not doing the below line will result in hardfault exception for ARM cortex M */
			//watch : https://www.youtube.com/watch?v=VX_12SjnNhY
			go_to_address += 1; //Make T-bit = 1 otherwise hardfault exception
			
			void (*lets_jump)(void) = (void *)go_to_address;
			printmsg("BL_DEBUG_MSG: now jumping to go-to address!\n\r");
			lets_jump();
			
		} else {
			printmsg("BL_DEBUG_MSG: go-to address invalid! \n\r");
			//tell host that address is invalid
			bootloader_uart_write_data(&addr_invalid, 1);
		}
		
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		bootloader_send_nack();
	}
}

/* Helper function to handle BL_FLASH_ERASE command */
void bootloader_handle_flash_erase(uint8_t *bl_rx_buffer) {
	
	uint8_t erase_status = 0x00;
	printmsg("BL_DEBUG_MSG: bootloader_handle_flash_erase\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *) (bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 1);
		printmsg("BL_DEBUG_MSG: initial_sector: %d no_of_sectors: %d\n\r", bl_rx_buffer[2], bl_rx_buffer[3]);
		
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
		erase_status = execute_flash_erase(bl_rx_buffer[2], bl_rx_buffer[3]);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
		
		printmsg("BL_DEBUG_MSG: flash erase status: %#x\n\r", erase_status);
		
		bootloader_uart_write_data(&erase_status, 1);
		
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		bootloader_send_nack();
	}
}
	
/* Helper function to handle BL_MEM_WRITE command */
void bootloader_handle_mem_write(uint8_t *bl_rx_buffer) {
	
	uint8_t addr_valid = ADDR_VALID;
	uint8_t write_status = 0x00;
	uint8_t chksum = 0, len = 0;
	len = bl_rx_buffer[0];
	uint8_t payload_len = bl_rx_buffer[6];
	
	uint32_t mem_address = *((uint32_t *)(&bl_rx_buffer[2]));
	
	chksum = bl_rx_buffer[len];
	
	printmsg("BL_DEBUG_MSG: bootloader_handle_mem_write\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 1);
		printmsg("BL_DEBUG_MSG: mem write address: %#x\n\r", mem_address);
		
		if(verify_address(mem_address) == ADDR_VALID) {
			printmsg("BL_DEBUG_MSG: valid mem write address\n\r");
			
			//Turn on the LED to indicate that the bootloader is currently writing to memory
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
			
			//Execute mem write
			write_status = execute_mem_write(&bl_rx_buffer[7], mem_address, payload_len);
			
			//Turn off the LED to indicate memory write is over
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
			
			//Inform the Host about the status
			bootloader_uart_write_data(&write_status, 1);
			
		} else {
			printmsg("BL_DEBUG_MSG: invalid mem write address\n\r");
			write_status = ADDR_INVALID;
			//Inform the Host that address is invalid
			bootloader_uart_write_data(&write_status, 1);
		}
		
	} else {
		printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
		bootloader_send_nack();
	}
}

/* Helper function to handle BL_EN_RW_PROTECT command */
void bootloader_handle_en_rw_protect(uint8_t *bl_rx_buffer) {
	
	uint8_t status = 0x00;
	printmsg("BL_DEBUG_MSG: bootloader_handle_en_rw_protect\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 1);
		
		status = configure_flash_sector_rw_protection(bl_rx_buffer[2], bl_rx_buffer[3], 0);
		printmsg("BL_DEBUG_MSG: flash erase status: %#x\n\r", status);
		bootloader_uart_write_data(&status, 1);
		
	} else {
    printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
    bootloader_send_nack();
	}
}

/* Helper function to handle BL_DIS_RW_PROTECT command */
void bootloader_handle_dis_rw_protect(uint8_t *bl_rx_buffer) {
	
	uint8_t status = 0x00;
	printmsg("BL_DEBUG_MSG: bootloader_handle_dis_rw_protect\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 1);
		
		status = configure_flash_sector_rw_protection(0, 0, 1);
		printmsg("BL_DEBUG_MSG: flash erase status: %#x\n\r", status);
		bootloader_uart_write_data(&status, 1);
		
	} else {
    printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
    bootloader_send_nack();
	}
}

/* Helper function to handle BL_MEM_READ command */
void bootloader_handle_mem_read(uint8_t *bl_rx_buffer) {
	
}

/* Helper function to handle BL_READ_SECTOR_P_STATUS command */
void bootloader_handle_read_sector_protection_status(uint8_t *bl_rx_buffer) {
	
	uint16_t status;
	printmsg("BL_DEBUG_MSG: bootloader_handle_read_sector_protection_status\n\r");
	
	//Total length of the command packet
	uint32_t command_packet_len = bl_rx_buffer[0] + 1;
	
	//Extract the CRC32 sent by the Host
	uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));
	
	if(!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
		printmsg("BL_DEBUG_MSG: checksum success!!\n\r");
		bootloader_send_ack(bl_rx_buffer[0], 2);
		
		status = read_OB_rw_protection_status();
		printmsg("BL_DEBUG_MSG: nWRP status: %#x\n\r", status);
		bootloader_uart_write_data((uint8_t *)&status, 2);
		
	} else {
    printmsg("BL_DEBUG_MSG: checksum fail!!\n\r");
    bootloader_send_nack();
	}
}

/* Helper function to handle BL_OTP_READ command */
void bootloader_handle_read_otp(uint8_t *bl_rx_buffer) {
	
}

/* Sends ACK if CRC matches along with "len to follow" */
void bootloader_send_ack(uint8_t command_code, uint8_t follow_len) {
	
	//Send 2 bytes.. 1st byte is ACK and the 2nd byte is "length to follow"
	uint8_t ack_buf[2];
	ack_buf[0] = BL_ACK;
	ack_buf[1] = follow_len;
	HAL_UART_Transmit(C_UART, ack_buf, 2, HAL_MAX_DELAY);
}

void bootloader_send_nack(void) {
	uint8_t nack = BL_NACK;
	HAL_UART_Transmit(C_UART, &nack, 1, HAL_MAX_DELAY);
}

/* Verifies the CRC of the given buffer in pData */
uint8_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host) {
	
	uint32_t uwCRCValue = 0xFF;
	
	for(uint32_t i = 0; i < len; i++) {
		uint32_t i_data = pData[i];
		uwCRCValue = HAL_CRC_Accumulate(&hcrc, &i_data, 1);
	}
	
	/* Reset CRC Calculation Unit */
	__HAL_CRC_DR_RESET(&hcrc);
	
	if(uwCRCValue == crc_host) {
		return VERIFY_CRC_SUCCESS;
	}
	
	return VERIFY_CRC_FAIL;
}

/* Writes Data into C_UART */
void bootloader_uart_write_data(uint8_t *pBuffer, uint32_t len) {
	
	HAL_UART_Transmit(C_UART, pBuffer, len, HAL_MAX_DELAY);
}

uint8_t get_bootloader_version(void) {
	
	return (uint8_t)BL_VERSION;
}

//Read the Chip or Device ID
uint16_t get_mcu_chip_id(void) {
	/*
	 * The STM32F446xx MCUs integrate an MCU ID code. This ID identifies the ST MCU partnumber
	 * and the die revision. It is part of the DBG_MCU component and is mapped on the
	 * external PPB bus (see Section 33.16 on page 1304). This code is accessible using the
	 * JTAG debug pCat.2ort (4 to 5 pins) or the SW debug port (two pins) or by the user software.
	 * It is even accessible while the MCU is under system reset. */
	uint16_t cid;
	cid = (uint16_t)(DBGMCU->IDCODE) & 0x0FFF;
	return cid;
}

uint8_t get_flash_rdp_level(void) {
	
	uint8_t rdp_status = 0;
	
#if 0
	//HAL / ST library APIs
	FLASH_OBProgramInitTypeDef ob_handle;
	HAL_FLASHEx_OBGetConfig(&ob_handle);
	rdp_status = (uint8_t)ob_handle.RDPLevel;
	
#else
	volatile uint32_t *pOB_addr = (uint32_t*) 0x1FFFC000;
	rdp_status = (uint8_t)(*pOB_addr >> 8);
	
#endif
	
	return rdp_status;
}

uint8_t verify_address(uint32_t go_to_address) {
	/* Valid addresses:
	 * System Memory (ROM), SRAM1, SRAM2, Backup SRAM, 
	 * Peripheral Memory (possible but not allowing that feature), 
	 * External Memory */
	
	if((go_to_address >= SRAM1_BASE) && (go_to_address <= SRAM1_END)) {
		return ADDR_VALID;
		
	} else if((go_to_address >= SRAM2_BASE) && (go_to_address <= SRAM2_END)) {
		return ADDR_VALID;
		
	} else if((go_to_address >= FLASH_BASE) && (go_to_address <= FLASH_END)) {
		return ADDR_VALID;
		
	} else if((go_to_address >= BKPSRAM_BASE) && (go_to_address <= BKPSRAM_END)) {
		return ADDR_VALID;
		
	} else {
		return ADDR_INVALID;
	}	
}

uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sectors) {
	//There are 8 sectors in STM32F446RE MCU (Sector[0:7])
	//number_of_sectors must be in the range of 0 to 7
	//if sector_number = 0xFF, that means mass erase
	
	FLASH_EraseInitTypeDef flashErase_handle; 
	uint32_t sectorError;
	HAL_StatusTypeDef status;
	
	if(number_of_sectors > 8)
		return INVALID_SECTOR;
	
	if((sector_number == 0xFF) || (sector_number <= 7)) {
		
		if(sector_number == (uint8_t) 0xFF) {
			flashErase_handle.TypeErase = FLASH_TYPEERASE_MASSERASE;
			
		} else {
			//Calculate how many sectors need to be erased
			uint8_t remaining_sector = 8 - sector_number;
			
			if(number_of_sectors > remaining_sector) {
				number_of_sectors = remaining_sector;
			}
			
			flashErase_handle.TypeErase = FLASH_TYPEERASE_SECTORS;
			flashErase_handle.Sector = sector_number; //The initial sector
			flashErase_handle.NbSectors = number_of_sectors;
		}
		
		flashErase_handle.Banks = FLASH_BANK_1;
		
		//Gain access to modify the FLASH registers
		HAL_FLASH_Unlock();
		flashErase_handle.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		status = (uint8_t) HAL_FLASHEx_Erase(&flashErase_handle, &sectorError);
		HAL_FLASH_Lock();
		
		return status;
	}
	
	return INVALID_SECTOR;
}

/* Writes the contents of pBuffer to "mem_address" byte by byte */
//Note1 : Currently this function supports writing to Flash only.
//Note2 : This function doesn't check whether "mem_address" is a valid address of the Flash memory
uint8_t execute_mem_write(uint8_t *pBuffer, uint32_t mem_address, uint32_t len) {
	
	uint8_t status = HAL_OK;
	
	//Unlock flash module to gain control of registers
	HAL_FLASH_Unlock();
	
	for(uint32_t i = 0; i < len; i++) {
		//Program the flash byte by byte
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, mem_address + i, pBuffer[i]);
	}
	
	HAL_FLASH_Lock();
	
	return status;
}

/*
Modifying user option bytes
To modify the user option value, follow the sequence below:
1. Check that no Flash memory operation is ongoing by checking the BSY bit in the
FLASH_SR register
2. Write the desired option value in the FLASH_OPTCR register.
3. Set the option start bit (OPTSTRT) in the FLASH_OPTCR register
4. Wait for the BSY bit to be cleared.
*/
uint8_t configure_flash_sector_rw_protection(uint8_t sector_details, uint8_t protection_mode, uint8_t disable) {
	//First configure the protection mode
	//protection_mode = 1, means write protect of the user flash sectors
	//protection_mode = 2, means read/write protect of the user flash sectors
	//According to RM of stm32f446xx TABLE 9, We have to modify the address 0x1FFF C008 bit 15(SPRMOD)
	
	//Flash option control register (OPTCR)
	volatile uint32_t *pOPTCR = (uint32_t *) 0x40023C14;
	
	if(disable) {
	//DISABLE all r/w protection on sectors
		//Option byte configuration unlock
		HAL_FLASH_OB_Unlock();
		
		//Wait until no active operations on flash
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
		
		//Clear the 31st bit (default state) - Refer to: Flash Option Control Register (FLASH_OPTCR) in RM
		*pOPTCR &= ~(1 << 31);
		
		//Clear the protection: Make all bits belonging to sectors as 1
		*pOPTCR |= (0xFF << 16);
		
		//Set the Option Start Bit (OPTSTRT) in the FLASH_OPTCR register
		*pOPTCR |= (1 << 1);
		
		//Wait until no active operations on flash
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
		
		HAL_FLASH_OB_Lock();
		
		return 0;
	}
	
	if(protection_mode == (uint8_t) 1) {
	//Put Write Protection on the sectors encoded in sector_details argument
		//Option Byte Configuration unlock
		HAL_FLASH_OB_Unlock();
	
		//Wait until no active operations on flash
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
		
		//Set Write Protection for the sectors
		//Clear the 31st bit (default state) - Refer to: Flash Option Control Register (FLASH_OPTCR) in RM
		*pOPTCR &= ~(1 << 31);
		
		//Set WP on sectors
		*pOPTCR &= ~(sector_details << 16);
		
		//Set the Option Start Bit (OPTSTRT) in the FLASH_OPTCR register
		*pOPTCR |= (1 << 1);
		
		//Wait until no active operations on flash
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);	
		
		HAL_FLASH_OB_Lock();
		
	} else if(protection_mode == (uint8_t) 2) {
	//Put Read and Write Protections on the sectors encoded in sector_details argument
		//Option Byte Configuration unlock
		HAL_FLASH_OB_Unlock();
		
		//Wait until no active operations on flash
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
		
		//Setting Read and Write Protections for the sectors
		//Set the 31st bit - Refer to: Flash Option Control Register (FLASH_OPTCR) in RM
		*pOPTCR |= (1 << 31);
		
		//Set R&W Protections on sectors
		*pOPTCR &= ~(0xFF << 16);
		*pOPTCR |= (sector_details << 16);
		
		//Set the Option Start Bit (OPTSTRT) in the FLASH_OPTCR register
		*pOPTCR |= (1 << 1);
		
		//Wait until no active operations on flash
		while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);	
		
		HAL_FLASH_OB_Lock();
	}
	
	return 0;
}

uint16_t read_OB_rw_protection_status(void) {
	//This structure is defined in ST Flash driver to hold the OB(Option Byte) contents
	FLASH_OBProgramInitTypeDef OBInit;
	
	//Unlock the OB(Option Byte) memory access
	HAL_FLASH_OB_Unlock();
	//Get the OB configuration details
	HAL_FLASHEx_OBGetConfig(&OBInit);
	//Lock the OB memory access
	HAL_FLASH_OB_Lock();
	
	//Only interested in R/W protection status of the sectors
	return (uint16_t)OBInit.WRPSector;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/