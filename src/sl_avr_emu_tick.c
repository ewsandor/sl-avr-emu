/**
 * @file sl_avr_emu_tick.cpp
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Tick Logic
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>

#include "sl_avr_emu.h"
#include "sl_avr_emu_bitops.h"
#include "sl_avr_emu_tick.h"

/**
 * @brief Simulates a clock tick for a given emulation
 * 
 * @param emulation            - Pointer to emulation to simulate
 * @return sl_avr_emu_result_e
 */
sl_avr_emu_result_e sl_avr_emu_tick(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(emulation->op_cycles_remaining > 0)
  {
    emulation->op_cycles_remaining--;
    SL_AVR_EMU_VERBOSE_LOG(printf("%u cycles for remaining for current operation\n", emulation->op_cycles_remaining));
  }
  else
  {
    if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc))
    {
      if(emulation->memory.flash[emulation->memory.pc] == 0)
      {
        /* NOP Handling */
        emulation->memory.pc++;
        SL_AVR_EMU_VERBOSE_LOG(printf("NOP. PC 0x%06x\n", emulation->memory.pc));
      }
      else
      {
        /* Unrecognized OPCODE Handling */
        result = SL_AVR_EMU_RESULT_INVALID_OPCODE;
        fprintf(stderr, "Unrecognized OPCODE: 0x%04x. PC Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc], emulation->memory.pc);
      }
    }
    else
    {
      result = SL_AVR_EMU_RESULT_INVALID_PC;
      fprintf(stderr, "Invalid PC Address: 0x%06x\n", emulation->memory.pc);
    }
  }

  return result;
}