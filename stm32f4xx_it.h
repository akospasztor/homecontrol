/**
  *=============================================================================
  * NVIC PriorityGroup = 1 ---> 2 groups
  *=============================================================================
  *			|		Group 0				|		Group 1
  *=============================================================================
  *   0: 	|	SysTick					|		USART2 RX (BT)
  *-----------------------------------------------------------------------------
  *   1: 	|	TIM2 (system timer)		|		USART3 RX (KNX, RS-485)
  *-----------------------------------------------------------------------------
  *   2: 	|							|		EXTI_Line3 for RFM73P IT-s
  *-----------------------------------------------------------------------------
  *   3: 	|							|		
  *-----------------------------------------------------------------------------
  *   4: 	|							|		
  *-----------------------------------------------------------------------------
  *   5: 	|							|		
  *-----------------------------------------------------------------------------
  *   6: 	|							|		USB OTG
  *-----------------------------------------------------------------------------
  *   7: 	|							|		USB FS Low Power  
  *=============================================================================
  *
  * Note: We don't use USB HS IT-s, so they are put in Group2.
  *	Group 2 | Sub 0: USB HS Dedicated IN
  *	Group 2 | Sub 1: USB HS Dedicated OUT
  *	Group 2 | Sub 2: USB HS Low Power
  *
  *=============================================================================  
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);


void EXTI0_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

void OTG_FS_IRQHandler(void);
void OTG_FS_WKUP_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
