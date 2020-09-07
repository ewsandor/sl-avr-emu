/**
 * @file sl_avr_emu_timer.c
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Timer/Counter Logic
 * @version 0.1
 * @date 2020-09-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sl_avr_emu.h"
#include "sl_avr_emu_bitops.h"
#include "sl_avr_emu_timer.h"

/**
 * @brief Checks if 8-bit timer is properly configured
 * 
 * @param timer - Timer to check
 * @return true 
 * @return false 
 */
bool sl_avr_emu_timer_8_configured(sl_avr_emu_timer_8_s *timer)
{
  bool ret_val = true;

  ret_val = (timer        != NULL &&
             timer->tccra != NULL &&
             timer->tccrb != NULL &&
             timer->tcnt  != NULL &&
             timer->ocra  != NULL &&
             timer->ocrb  != NULL &&
             timer->timsk != NULL &&
             timer->tifr  != NULL );

  return ret_val;
}

/**
 * @brief Configures timer counter 0 registers as 8-bit timer
 * 
 * @param memory - memory containing timer 0's register
 * @param timer  - timer to configure
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_configure_timer0(sl_avr_emu_memory_s * memory, sl_avr_emu_timer_8_s *timer)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(timer != NULL)
  {
    memset(timer, 0, sizeof(sl_avr_emu_timer_8_s));

    timer->tccra = &memory->data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(SL_AVR_EMU_TIMER_0_TCCR0A)];
    timer->tccrb = &memory->data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(SL_AVR_EMU_TIMER_0_TCCR0B)];
    timer->tcnt  = &memory->data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(SL_AVR_EMU_TIMER_0_TCNT0) ];
    timer->ocra  = &memory->data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(SL_AVR_EMU_TIMER_0_OCR0A) ];
    timer->ocrb  = &memory->data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(SL_AVR_EMU_TIMER_0_OCR0B) ];
    timer->timsk = &memory->data[SL_AVR_EMU_TIMER_0_TIMSK0];
    timer->tifr  = &memory->data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(SL_AVR_EMU_TIMER_0_TIFR0) ];
  }
  else
  {
    result = SL_AVR_EMU_RESULT_FAILURE;    
  }

  return result;
}

/**
 * @brief Simulates tick of 8-bit timer/counter
 * 
 * @param timer Timer to simulate
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_timer_8_tick(sl_avr_emu_timer_8_s *timer)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_timer_clock_select_e    clock_select;
  sl_avr_emu_timer_wgm_e             timer_wgm;
  sl_avr_emu_timer_prescaler_count_t prescaler_reference;
  sl_avr_emu_byte_t                  tcnt_prev;

  if(sl_avr_emu_timer_8_configured(timer))
  {
    clock_select = (*timer->tccrb & 0x7);
    timer_wgm    = (*timer->tccra & 0x3) | ((*timer->tccrb >> 1) & 0x4);

    if(SL_AVR_EMU_CLOCK_SELECT_NONE != clock_select)
    {
      switch(clock_select)
      {
        case SL_AVR_EMU_CLOCK_SELECT_IO:
        case SL_AVR_EMU_CLOCK_SELECT_T0_FALLING:
        case SL_AVR_EMU_CLOCK_SELECT_T0_RISING:
        {
          prescaler_reference = 1;
          break;
        }
        case SL_AVR_EMU_CLOCK_SELECT_IO_8:
        {
          prescaler_reference = 8;
          break;
        }
        case SL_AVR_EMU_CLOCK_SELECT_IO_64:
        {
          prescaler_reference = 64;
          break;
        }
        case SL_AVR_EMU_CLOCK_SELECT_IO_256:
        {
          prescaler_reference = 256;
          break;
        }
        case SL_AVR_EMU_CLOCK_SELECT_IO_1024:
        {
          prescaler_reference = 1024;
          break;
        }
        default:
        {
          break;
        }
      }

      timer->prescaler_count++;
      if(timer->prescaler_count >= prescaler_reference)
      {
        timer->prescaler_count = 0;
        tcnt_prev=*timer->tcnt;
        *timer->tcnt = *timer->tcnt + 1;
        if(0 == *timer->tcnt && 0xFF == tcnt_prev)
        {
          SL_AVR_EMU_SET_BIT(*timer->tifr, SL_AVR_EMU_TIMER_0_TOV0);
        }
        if(*timer->tcnt == *timer->ocra)
        {
          SL_AVR_EMU_SET_BIT(*timer->tifr, SL_AVR_EMU_TIMER_0_OCF0A);
          if(SL_AVR_EMU_TIMER_WGM_CTC           == timer_wgm ||
             SL_AVR_EMU_TIMER_WGM_PWM_OCRA      == timer_wgm ||
             SL_AVR_EMU_TIMER_WGM_FAST_PWM_OCRA == timer_wgm )
          {
            *timer->tcnt = 0;
          }
        }
        if(*timer->tcnt == *timer->ocrb)
        {
          SL_AVR_EMU_SET_BIT(*timer->tifr, SL_AVR_EMU_TIMER_0_OCF0B);
        }
      }

      SL_AVR_EMU_VERBOSE_LOG(printf("Timer 0 tick, tcnt0 0x%02x, prescaler count %u, tifr 0x%02x\n", *timer->tcnt, timer->prescaler_count, *timer->tifr));
    }
  }
  else
  {
    result = SL_AVR_EMU_RESULT_INVALID_HARDWARE;    
  }

  return result;
}