/**
 * @file sl_avr_emu_interrupt.h
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Interrupt Logic
 * @version 0.1
 * @date 2020-09-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>

#include "sl_avr_emu.h"
#include "sl_avr_emu_bitops.h"
#include "sl_avr_emu_interrupt.h"
#include "sl_avr_emu_tick.h"
#include "sl_avr_emu_timer.h"

sl_avr_emu_result_e sl_avr_emu_interrupt(sl_avr_emu_emulation_s *emulation, sl_avr_emu_extended_address_t interrupt_pc)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_INTERRUPT_FLAG);

  result = slf_var_emu_stack_push_pc(emulation, emulation->memory.pc);

  emulation->memory.pc = interrupt_pc;

  SL_AVR_EMU_VERBOSE_LOG(printf("INTERRUPT. PC 0x%06x. SREG 0x%02x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS]));

  return result;
}

/**
 * @brief Handle any pending interrupts
 * 
 * @param emulation 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_interrupt_handling(sl_avr_emu_emulation_s *emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_INTERRUPT_FLAG))
  {
    if(sl_avr_emu_timer_8_configured(&emulation->timer0))
    {
      if(SL_AVR_EMU_CHECK_BIT(*emulation->timer0.timsk, SL_AVR_EMU_TIMER_0_OCIE0A) &&
         SL_AVR_EMU_CHECK_BIT(*emulation->timer0.tifr,  SL_AVR_EMU_TIMER_0_OCF0A))
      {
        SL_AVR_EMU_CLEAR_BIT(*emulation->timer0.tifr,  SL_AVR_EMU_TIMER_0_OCF0A);
        sl_avr_emu_interrupt(emulation, 0x001C);
      }
      else if(SL_AVR_EMU_CHECK_BIT(*emulation->timer0.timsk, SL_AVR_EMU_TIMER_0_OCIE0B) &&
         SL_AVR_EMU_CHECK_BIT(*emulation->timer0.tifr,  SL_AVR_EMU_TIMER_0_OCF0B))
      {
        SL_AVR_EMU_CLEAR_BIT(*emulation->timer0.tifr,  SL_AVR_EMU_TIMER_0_OCF0B);
        sl_avr_emu_interrupt(emulation, 0x001E);
      }
      else if(SL_AVR_EMU_CHECK_BIT(*emulation->timer0.timsk, SL_AVR_EMU_TIMER_0_TOIE0) &&
         SL_AVR_EMU_CHECK_BIT(*emulation->timer0.tifr,  SL_AVR_EMU_TIMER_0_TOV0))
      {
        SL_AVR_EMU_CLEAR_BIT(*emulation->timer0.tifr,  SL_AVR_EMU_TIMER_0_TOV0);
        sl_avr_emu_interrupt(emulation, 0x0020);
      }
    }
  }
  else
  {
    SL_AVR_EMU_VERBOSE_LOG(printf("Interrupts disabled. SREG 0x%02x\n", emulation->memory.pc));
  }

  return result;
}