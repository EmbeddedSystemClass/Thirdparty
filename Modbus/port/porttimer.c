/*
 * FreeModbus Libary: Atmel AT91SAM3S Demo Application
 * Copyright (C) 2010 Christian Walter <cwalter@embedded-solutions.at>
 *
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * IF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id$
 */

/* ----------------------- System includes ----------------------------------*/
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_TIMER_DEBUG                      ( 0 )
#define MB_TIMER_PRESCALER                  ( 128UL )
#define MB_TIMER_TICKS                      ( BOARD_MCK / MB_TIMER_PRESCALER )
#define MB_50US_TICKS                       ( 20000UL )

#define TCX                                 ( TC0 )
#define TCXIRQ                              ( TC0_IRQn )
#define TCCHANNEL                           ( 0 )
#define TCX_IRQHANDLER                      TC0_IrqHandler

#define TC_CMRX_WAVE                        ( 0x1 << 15 )
#define TC_CMRX_TCCLKS_TIMER_DIV4_CLOCK     ( 0x3 << 0 )
#define TC_CMRX_CPCSTOP                     ( 0x1 << 6 )
#define TC_CMRX_WAVESEL_UP_RC               ( 0x2 << 13 )

#define TC_IERX_CPCS                        ( 0x1 << 4 )
#define TC_IERX_CPAS                        ( 0x1 << 2 )
#define TC_SRX_CPAS                         ( 0x1 << 2 )
#if MB_TIMER_DEBUG == 1
#define TIMER_PIN { 1 << 6, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT }
#endif

/* ----------------------- Static variables ---------------------------------*/
extern TIM_HandleTypeDef htim7;
uint16_t timeout = 0;
uint16_t downcounter = 0;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
	TIM_MasterConfigTypeDef sMasterConfig;
  
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 50 - 1;
  
  timeout = usTim1Timerout50us;

  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    return FALSE;
  }
  
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    return FALSE;
  }
  
  return TRUE;
}

void
vMBPortTimerClose( void )
{
    //NVIC_DisableIRQ( &htim7 );
    //PMC_DisablePeripheral( ID_TC0 );
	HAL_TIM_Base_DeInit(&htim7);
}

void
vMBPortTimersEnable(  )
{
/* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
  downcounter = timeout;
  HAL_TIM_Base_Start_IT(&htim7);

}

void
vMBPortTimersDisable(  )
{
  /* Disable any pending timers. */
  HAL_TIM_Base_Stop_IT(&htim7);
}

void
vMBPortTimersDelay( USHORT usTimeOutMS )
{
    vTaskDelay( usTimeOutMS / portTICK_RATE_MS );
}

/*void
TCX_IRQHANDLER( void )
{
    uint32_t        xTCX_IMRX = TCX->TC_CHANNEL[TCCHANNEL].TC_IMR;
    uint32_t        xTCX_SRX = TCX->TC_CHANNEL[TCCHANNEL].TC_SR;
    uint32_t        uiSRMasked = xTCX_SRX & xTCX_IMRX;
    BOOL            bTaskWoken = FALSE;

    vMBPortSetWithinException( TRUE );

    if( ( uiSRMasked & TC_SRX_CPAS ) > 0 )
    {
#if MB_TIMER_DEBUG == 1
        PIO_Clear( &xTimerDebugPins[0] );
#endif
        ( void )pxMBPortCBTimerExpired(  );
    }
    vMBPortSetWithinException( FALSE );

    portEND_SWITCHING_ISR( bTaskWoken ? pdTRUE : pdFALSE );
}
*/
