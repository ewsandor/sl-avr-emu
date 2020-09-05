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

#define SL_AVR_EMU_IS_JMP(opcode)  (((opcode) & 0xFE0C) == 0x940C)
#define SL_AVR_EMU_IS_CALL(opcode) (((opcode) & 0xFE0C) == 0x940E)

sl_avr_emu_result_e sl_avr_emu_opcode_unrecognized(sl_avr_emu_emulation_s * emulation)
{
  /* Unrecognized OPCODE Handling */
  fprintf(stderr, "Unrecognized OPCODE: 0x%04x. PC Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc], emulation->memory.pc);
  if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc + 1))
  {
    fprintf(stderr, "Next OPCODE: 0x%04x. PC+1 Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc+1], emulation->memory.pc+1);
  }

  return SL_AVR_EMU_RESULT_INVALID_OPCODE;
}

/**
 * @brief Opcodes with 0b00 prefix handling
 * 
 * @param emulation 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_opcode_0(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
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
  return result;
}
/**
 * @brief Opcodes with 0b01 prefix handling
 * 
 * @param emulation 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_opcode_1(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  if(0)
  {
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }
  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_jmp(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc + 1))
  {
    emulation->memory.pc = emulation->memory.flash[emulation->memory.pc+1];
    SL_AVR_EMU_VERBOSE_LOG(printf("JMP. PC 0x%06x\n", emulation->memory.pc));
  }
  else
  {
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }
  return result;
}

/**
 * @brief Opcodes with 0b10 prefix handling
 * 
 * @param emulation 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_opcode_2(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if( SL_AVR_EMU_IS_JMP(emulation->memory.flash[emulation->memory.pc]) )
  {
    result = sl_avr_emu_opcode_jmp(emulation);
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = SL_AVR_EMU_RESULT_INVALID_OPCODE;
    fprintf(stderr, "Unrecognized OPCODE: 0x%04x. PC Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc], emulation->memory.pc);
  }
  return result;
}
/**
 * @brief Opcodes with 0b11 prefix handling
 * 
 * @param emulation 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_opcode_3(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  if(0)
  {
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = SL_AVR_EMU_RESULT_INVALID_OPCODE;
    fprintf(stderr, "Unrecognized OPCODE: 0x%04x. PC Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc], emulation->memory.pc);
  }
  return result;
}

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
      switch (emulation->memory.flash[emulation->memory.pc] >> (16-2)) {
        case 0b00:
        {
          result = sl_avr_emu_opcode_0(emulation);
          break;
        }
        case 0b01:
        {
          result = sl_avr_emu_opcode_1(emulation);
          break;
        }
        case 0b10:
        {
          result = sl_avr_emu_opcode_2(emulation);
          break;
        }
        case 0b11:
        {
          result = sl_avr_emu_opcode_3(emulation);
          break;
        }
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