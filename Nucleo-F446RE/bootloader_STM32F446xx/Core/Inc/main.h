/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* Bootloader function prototypes */
void bootloader_uart_read_data(void);
void bootloader_jump_to_user_app(void);

void bootloader_handle_getver(uint8_t *bl_rx_buffer);
void bootloader_handle_gethelp(uint8_t *bl_rx_buffer);
void bootloader_handle_getcid(uint8_t *bl_rx_buffer);
void bootloader_handle_getrdp(uint8_t *bl_rx_buffer);
void bootloader_handle_go_to_addr(uint8_t *bl_rx_buffer);
void bootloader_handle_flash_erase(uint8_t *bl_rx_buffer);
void bootloader_handle_mem_write(uint8_t *bl_rx_buffer);
void bootloader_handle_en_rw_protect(uint8_t *bl_rx_buffer);
void bootloader_handle_dis_rw_protect(uint8_t *bl_rx_buffer);
void bootloader_handle_mem_read(uint8_t *bl_rx_buffer);
void bootloader_handle_read_sector_protection_status(uint8_t *bl_rx_buffer);
void bootloader_handle_read_otp(uint8_t *bl_rx_buffer);

void bootloader_send_ack(uint8_t command_code, uint8_t follow_len);
void bootloader_send_nack(void);

uint8_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host);
void bootloader_uart_write_data(uint8_t *pBuffer, uint32_t len);
uint8_t get_bootloader_version(void);
uint16_t get_mcu_chip_id(void);
uint8_t get_flash_rdp_level(void);
uint8_t verify_address(uint32_t go_to_address);
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sectors);
uint8_t execute_mem_write(uint8_t *pBuffer, uint32_t mem_address, uint32_t len);
uint8_t configure_flash_sector_rw_protection(uint8_t sector_details, uint8_t protection_mode, uint8_t disable);
uint16_t read_OB_rw_protection_status(void);

/* Bootloader Commands */
#define BL_GET_VER										0x51 	//Read Bootloader Version from MCU
#define BL_GET_HELP										0x52	//Print Commands Supported by the Bootloader
#define BL_GET_CID										0x53  //Read the MCU Chip ID Number
#define BL_GET_RDP_STATUS							0x54  //Read the FLASH Read Protection Level
#define BL_GO_TO_ADDR									0x55  //Jump Bootloader to Specified Address
#define BL_FLASH_ERASE								0x56  //Mass Erase or Sector Erase of User Flash
#define BL_MEM_WRITE									0x57  //Write Data into Different Memories of MCU
#define BL_EN_RW_PROTECT							0x58  //Enable Read/Write Protect on Different Sectors of User Flash
#define BL_MEM_READ										0x59  //Read Data from Different Memories of the MCU
#define BL_READ_SECTOR_P_STATUS				0x5A  //Used to Read All the Sector Protection Statuses
#define BL_OTP_READ										0x5B  //Used to Read the OTP Contents
#define BL_DIS_RW_PROTECT							0x5C  //Disable all sector read/write protection 

/* ACK and NACK bytes */
#define BL_ACK 												0xA5
#define BL_NACK 											0x7F

#define VERIFY_CRC_SUCCESS						0
#define VERIFY_CRC_FAIL								1

#define ADDR_VALID										0x00
#define ADDR_INVALID									0x01

#define INVALID_SECTOR 								0x04

#define SRAM1_SIZE										112*1024		//STM32F446RE: 112kB of SRAM1
#define SRAM1_END											(SRAM1_BASE + SRAM1_SIZE)
#define SRAM2_SIZE										16*1024			//STM32F446RE: 16kB of SRAM2
#define SRAM2_END											(SRAM2_BASE + SRAM2_SIZE)
#define FLASH_SIZE										512*1024		//STM32F446RE: 512kB of FLASH
#define BKPSRAM_SIZE									4*1024		  //STM32F446RE: 4kB of Backup SRAM
#define BKPSRAM_END										(BKPSRAM_BASE + BKPSRAM_SIZE)		


//Version 1.0
#define BL_VERSION										0x10

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin 												GPIO_PIN_13
#define B1_GPIO_Port 									GPIOC
#define USART_TX_Pin 									GPIO_PIN_2
#define USART_TX_GPIO_Port 						GPIOA
#define USART_RX_Pin 									GPIO_PIN_3
#define USART_RX_GPIO_Port 						GPIOA
#define LD2_Pin 											GPIO_PIN_5
#define LD2_GPIO_Port 								GPIOA
#define TMS_Pin 											GPIO_PIN_13
#define TMS_GPIO_Port									GPIOA
#define TCK_Pin 											GPIO_PIN_14
#define TCK_GPIO_Port									GPIOA
#define SWO_Pin 											GPIO_PIN_3
#define SWO_GPIO_Port 								GPIOB

#define FLASH_SECTOR2_BASE_ADDRESS 		0x08008000U

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
