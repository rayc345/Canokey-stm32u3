// SPDX-License-Identifier: Apache-2.0
#include "main.h"
#include <admin.h>
#include <device.h>

/* This file overrides functions defined in canokey-core/src/device.c */

const uint32_t TOUCH_GAP_TIME = TOUCH_EXPIRE_TIME; /* Gap period (in ms) between two consecutive touch events */
const uint32_t MIN_LONG_TOUCH_TIME = 500;
const uint32_t MIN_TOUCH_TIME = 20;

extern TIM_HandleTypeDef htim6;
extern SPI_HandleTypeDef hspi1;

static volatile uint32_t blinking_until;
static void (*tim_callback)(void);

void device_delay(int ms) { HAL_Delay(ms); }

uint32_t device_get_tick(void) { return HAL_GetTick(); }

void device_set_timeout(void (*callback)(void), uint16_t timeout)
{
  const uint32_t prescaler = 4000;
  uint32_t counting_freq = 48000000;
  // DBG_MSG("counting_freq=%u\n", counting_freq);
  if (timeout == 0)
  {
    HAL_TIM_Base_Stop_IT(&htim6);
    return;
  }
  tim_callback = callback;
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = prescaler - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = counting_freq / 1000 * timeout - 1;
  if (htim6.Init.Period > 65535)
  {
    ERR_MSG("TIM6 period %u overflow!\n", htim6.Init.Period);
    htim6.Init.Period = 65535;
  }
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    Error_Handler();
  LL_TIM_ClearFlag_UPDATE(htim6.Instance);
  LL_TIM_SetCounter(htim6.Instance, 0);
  HAL_TIM_Base_Start_IT(&htim6);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim6)
  {
    HAL_TIM_Base_Stop_IT(&htim6);
    tim_callback();
  }
}

static GPIO_PinState GPIO_Touched(void)
{
  return HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin);
}

void led_on(void) { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); }

void led_off(void) { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); }

void device_periodic_task(void)
{
  enum
  {
    TOUCH_STATE_IDLE,
    TOUCH_STATE_DOWN,
    TOUCH_STATE_ASSERT,
    TOUCH_STATE_DEASSERT,
  };
  static uint32_t event_tick, fsm = TOUCH_STATE_IDLE;
  uint32_t tick = HAL_GetTick();
  switch (fsm)
  {
  case TOUCH_STATE_IDLE:
    if (GPIO_Touched())
    {
      fsm = TOUCH_STATE_DOWN;
      event_tick = tick;
    }
    break;
  case TOUCH_STATE_DOWN:
    if (!GPIO_Touched() || tick - event_tick > MIN_LONG_TOUCH_TIME)
    {
      if (tick - event_tick > MIN_TOUCH_TIME)
      {
        set_touch_result(tick - event_tick > MIN_LONG_TOUCH_TIME ? TOUCH_LONG : TOUCH_SHORT);
        fsm = TOUCH_STATE_ASSERT;
        event_tick = tick;
      }
      else
        fsm = TOUCH_STATE_IDLE;
    }
    break;
  case TOUCH_STATE_ASSERT:
    if (tick - event_tick >= TOUCH_GAP_TIME)
    {
      set_touch_result(TOUCH_NO);
      fsm = TOUCH_STATE_DEASSERT;
    }
    break;
  case TOUCH_STATE_DEASSERT:
    if (!GPIO_Touched())
    {
      fsm = TOUCH_STATE_IDLE;
    }
    break;
  default:
    break;
  }
  device_update_led();
}

// void fm_nss_low(void) { HAL_GPIO_WritePin(FM_SSN_GPIO_Port, FM_SSN_Pin, GPIO_PIN_RESET); }

// void fm_nss_high(void) { HAL_GPIO_WritePin(FM_SSN_GPIO_Port, FM_SSN_Pin, GPIO_PIN_SET); }

// void fm_transmit(uint8_t *buf, uint8_t len) { HAL_SPI_Transmit(&hspi1, buf, len, 1000); }

// void fm_receive(uint8_t *buf, uint8_t len) { HAL_SPI_Receive(&hspi1, buf, len, 1000); }

int device_atomic_compare_and_swap(volatile uint32_t *var, uint32_t expect, uint32_t update)
{
  int status = 0;
  do
  {
    if (__LDREXW(var) != expect)
      return -1;
    status = __STREXW(update, var); // Try to set
  } while (status != 0); // retry until updated
  __DMB(); // Do not start any other memory access
  return 0;
}

// ARM Cortex-M Programming Guide to Memory Barrier Instructions,	Application Note 321

int device_spinlock_lock(volatile uint32_t *lock, uint32_t blocking)
{
  // Note: __LDREXW and __STREXW are CMSIS functions
  int status = 0;
  do
  {
    while (__LDREXW(lock) != 0)
      if (!blocking)
        return -1;
      else
      {
      } // Wait until
    // lock is free
    status = __STREXW(1, lock); // Try to set
    // lock
  } while (status != 0); // retry until lock successfully
  __DMB(); // Do not start any other memory access
  // until memory barrier is completed
  return 0;
}

void device_spinlock_unlock(volatile uint32_t *lock)
{
  // Note: __LDREXW and __STREXW are CMSIS functions
  __DMB(); // Ensure memory operations completed before
  // releasing lock
  *lock = 0;
}
