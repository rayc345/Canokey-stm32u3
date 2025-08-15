/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2024 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File      : stm32u535xx_Vectors.s
Purpose   : Exception and interrupt vectors for stm32u535xx devices.

Additional information:
  Preprocessor Definitions
    __NO_EXTERNAL_INTERRUPTS
      If defined,
        the vector table will contain only the internal exceptions
        and interrupts.
    __VECTORS_IN_RAM
      If defined,
        an area of RAM, large enough to store the vector table,
        will be reserved.

    __OPTIMIZATION_SMALL
      If defined,
        all weak definitions of interrupt handlers will share the
        same implementation.
      If not defined,
        all weak definitions of interrupt handlers will be defined
        with their own implementation.
*/
        .syntax unified

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/

//
// Directly place a vector (word) in the vector table
//
.macro VECTOR Name=
        .section .vectors, "ax"
        .code 16
        .word \Name
.endm

//
// Declare an exception handler with a weak definition
//
.macro EXC_HANDLER Name=
        //
        // Insert vector in vector table
        //
        .section .vectors, "ax"
        .word \Name
        //
        // Insert dummy handler in init section
        //
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b   // Endless loop
.endm

//
// Declare an interrupt handler with a weak definition
//
.macro ISR_HANDLER Name=
        //
        // Insert vector in vector table
        //
        .section .vectors, "ax"
        .word \Name
        //
        // Insert dummy handler in init section
        //
#if defined(__OPTIMIZATION_SMALL)
        .section .init, "ax"
        .weak \Name
        .thumb_set \Name,Dummy_Handler
#else
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b   // Endless loop
#endif
.endm

//
// Place a reserved vector in vector table
//
.macro ISR_RESERVED
        .section .vectors, "ax"
        .word 0
.endm

//
// Place a reserved vector in vector table
//
.macro ISR_RESERVED_DUMMY
        .section .vectors, "ax"
        .word Dummy_Handler
.endm

/*********************************************************************
*
*       Externals
*
**********************************************************************
*/
        .extern __stack_end__
        .extern Reset_Handler
        .extern HardFault_Handler

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*  Setup of the vector table and weak definition of interrupt handlers
*
*/
        .section .vectors, "ax"
        .code 16
        .balign 512
        .global _vectors
_vectors:
        //
        // Internal exceptions and interrupts
        //
        VECTOR __stack_end__
        VECTOR Reset_Handler
        EXC_HANDLER NMI_Handler
        VECTOR HardFault_Handler
#ifdef __ARM_ARCH_6M__
            ISR_RESERVED
            ISR_RESERVED
            ISR_RESERVED
#else
        EXC_HANDLER MemManage_Handler
        EXC_HANDLER BusFault_Handler
        EXC_HANDLER UsageFault_Handler
#endif
        EXC_HANDLER SecureFault_Handler
            ISR_RESERVED
            ISR_RESERVED
            ISR_RESERVED
        EXC_HANDLER SVC_Handler
#ifdef __ARM_ARCH_6M__
            ISR_RESERVED
#else
        EXC_HANDLER DebugMon_Handler
#endif
            ISR_RESERVED
        EXC_HANDLER PendSV_Handler
        EXC_HANDLER SysTick_Handler
        //
        // External interrupts
        //
#ifndef __NO_EXTERNAL_INTERRUPTS
        ISR_HANDLER  WWDG_IRQHandler
        ISR_HANDLER  PVD_PVM_IRQHandler
        ISR_HANDLER  RTC_IRQHandler
        ISR_HANDLER  RTC_S_IRQHandler
        ISR_HANDLER  TAMP_IRQHandler
        ISR_HANDLER  RAMCFG_IRQHandler
        ISR_HANDLER  FLASH_IRQHandler
        ISR_HANDLER  FLASH_S_IRQHandler
        ISR_HANDLER  GTZC_IRQHandler
        ISR_HANDLER  RCC_IRQHandler
        ISR_HANDLER  RCC_S_IRQHandler
        ISR_HANDLER  EXTI0_IRQHandler
        ISR_HANDLER  EXTI1_IRQHandler
        ISR_HANDLER  EXTI2_IRQHandler
        ISR_HANDLER  EXTI3_IRQHandler
        ISR_HANDLER  EXTI4_IRQHandler
        ISR_HANDLER  EXTI5_IRQHandler
        ISR_HANDLER  EXTI6_IRQHandler
        ISR_HANDLER  EXTI7_IRQHandler
        ISR_HANDLER  EXTI8_IRQHandler
        ISR_HANDLER  EXTI9_IRQHandler
        ISR_HANDLER  EXTI10_IRQHandler
        ISR_HANDLER  EXTI11_IRQHandler
        ISR_HANDLER  EXTI12_IRQHandler
        ISR_HANDLER  EXTI13_IRQHandler
        ISR_HANDLER  EXTI14_IRQHandler
        ISR_HANDLER  EXTI15_IRQHandler
        ISR_HANDLER  IWDG_IRQHandler
        ISR_HANDLER  SAES_IRQHandler
        ISR_HANDLER  GPDMA1_Channel0_IRQHandler
        ISR_HANDLER  GPDMA1_Channel1_IRQHandler
        ISR_HANDLER  GPDMA1_Channel2_IRQHandler
        ISR_HANDLER  GPDMA1_Channel3_IRQHandler
        ISR_HANDLER  GPDMA1_Channel4_IRQHandler
        ISR_HANDLER  GPDMA1_Channel5_IRQHandler
        ISR_HANDLER  GPDMA1_Channel6_IRQHandler
        ISR_HANDLER  GPDMA1_Channel7_IRQHandler
        ISR_HANDLER  ADC1_IRQHandler
        ISR_HANDLER  DAC1_IRQHandler
        ISR_HANDLER  FDCAN1_IT0_IRQHandler
        ISR_HANDLER  FDCAN1_IT1_IRQHandler
        ISR_HANDLER  TIM1_BRK_TERR_IERR_IRQHandler
        ISR_HANDLER  TIM1_UP_IRQHandler
        ISR_HANDLER  TIM1_TRG_COM_DIR_IDX_IRQHandler
        ISR_HANDLER  TIM1_CC_IRQHandler
        ISR_HANDLER  TIM2_IRQHandler
        ISR_HANDLER  TIM3_IRQHandler
        ISR_HANDLER  TIM4_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  TIM6_IRQHandler
        ISR_HANDLER  TIM7_IRQHandler
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER  I3C1_EV_IRQHandler
        ISR_HANDLER  I3C1_ER_IRQHandler
        ISR_HANDLER  I2C1_EV_IRQHandler
        ISR_HANDLER  I2C1_ER_IRQHandler
        ISR_HANDLER  I2C2_EV_IRQHandler
        ISR_HANDLER  I2C2_ER_IRQHandler
        ISR_HANDLER  SPI1_IRQHandler
        ISR_HANDLER  SPI2_IRQHandler
        ISR_HANDLER  USART1_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  USART3_IRQHandler
        ISR_HANDLER  UART4_IRQHandler
        ISR_HANDLER  UART5_IRQHandler
        ISR_HANDLER  LPUART1_IRQHandler
        ISR_HANDLER  LPTIM1_IRQHandler
        ISR_HANDLER  LPTIM2_IRQHandler
        ISR_HANDLER  TIM15_IRQHandler
        ISR_HANDLER  TIM16_IRQHandler
        ISR_HANDLER  TIM17_IRQHandler
        ISR_HANDLER  COMP_IRQHandler
        ISR_HANDLER  USB_FS_IRQHandler
        ISR_HANDLER  CRS_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  OCTOSPI1_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  SDMMC1_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  GPDMA1_Channel8_IRQHandler
        ISR_HANDLER  GPDMA1_Channel9_IRQHandler
        ISR_HANDLER  GPDMA1_Channel10_IRQHandler
        ISR_HANDLER  GPDMA1_Channel11_IRQHandler
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER  I2C3_EV_IRQHandler
        ISR_HANDLER  I2C3_ER_IRQHandler
        ISR_HANDLER  SAI1_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  TSC_IRQHandler
        ISR_HANDLER  AES_IRQHandler
        ISR_HANDLER  RNG_IRQHandler
        ISR_HANDLER  FPU_IRQHandler
        ISR_HANDLER  HASH_IRQHandler
        ISR_HANDLER  PKA_IRQHandler
        ISR_HANDLER  LPTIM3_IRQHandler
        ISR_HANDLER  SPI3_IRQHandler
        ISR_HANDLER  I3C2_EV_IRQHandler
        ISR_HANDLER  I3C2_ER_IRQHandler
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER  ICACHE_IRQHandler
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER  LPTIM4_IRQHandler
        ISR_RESERVED
        ISR_HANDLER  ADF1_IRQHandler
        ISR_HANDLER  ADC2_IRQHandler
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER  PWR_IRQHandler
        ISR_HANDLER  PWR_S_IRQHandler
#endif
        //
        .section .vectors, "ax"
_vectors_end:

#ifdef __VECTORS_IN_RAM
        //
        // Reserve space with the size of the vector table
        // in the designated RAM section.
        //
        .section .vectors_ram, "ax"
        .balign 512
        .global _vectors_ram

_vectors_ram:
        .space _vectors_end - _vectors, 0
#endif

/*********************************************************************
*
*  Dummy handler to be used for reserved interrupt vectors
*  and weak implementation of interrupts.
*
*/
        .section .init.Dummy_Handler, "ax"
        .thumb_func
        .weak Dummy_Handler
        .balign 2
Dummy_Handler:
        1: b 1b   // Endless loop


/*************************** End of file ****************************/
