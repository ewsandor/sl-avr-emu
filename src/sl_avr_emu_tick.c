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

#define SL_AVR_EMU_IS_JMP_CALL(opcode)  (((opcode) & 0xFE0C) == 0x940C)

sl_avr_emu_result_e sl_avr_emu_stack_push_byte(sl_avr_emu_emulation_s * emulation, sl_avr_emu_byte_t byte)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_extended_address_t sp = emulation->memory.data[SL_AVR_EMU_SPL_ADDRESS];

  if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
  {
    sp |= (emulation->memory.data[SL_AVR_EMU_SPH_ADDRESS] << 8);
  }

  if(SL_AVR_EMU_DATA_ADDRESS_VALID(sp))
  {
    emulation->memory.data[sp] = byte;
    if(sp > 0)
    {
      sp--;
      emulation->memory.data[SL_AVR_EMU_SPL_ADDRESS] = (sp & 0xFF);
      if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
      {
        emulation->memory.data[SL_AVR_EMU_SPH_ADDRESS] = ((sp >> 8) & 0xFF);
      }
    }
    else 
    {
      result = SL_AVR_EMU_RESULT_STACK_OVERFLOW;
    }
  }
  else 
  {
    result = SL_AVR_EMU_RESULT_INVALID_DATA_ADDRESS;
  }

  return result;
}

sl_avr_emu_result_e slf_var_emu_stack_push_pc(sl_avr_emu_emulation_s * emulation, sl_avr_emu_extended_address_t pc)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  result = sl_avr_emu_stack_push_byte(emulation, (pc & 0xFF));
  if(SL_AVR_EMU_RESULT_SUCCESS == result)
  { 
    result = sl_avr_emu_stack_push_byte(emulation, ((pc >> 8) & 0xFF));
  }
  if(SL_AVR_EMU_EXTENDED_PC_ADDRESS && SL_AVR_EMU_RESULT_SUCCESS == result)
  {
    result = sl_avr_emu_stack_push_byte(emulation, ((pc >> 16) & 0xFF));
  }
  
  return result;
}

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

sl_avr_emu_result_e sl_avr_emu_opcode_jmp_call(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_extended_address_t pc_prev;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc + 1))
  {
    pc_prev = emulation->memory.pc;
    emulation->memory.pc = emulation->memory.flash[pc_prev+1];
    emulation->memory.pc |= ((emulation->memory.flash[pc_prev] & 0x1  ) << 16);
    emulation->memory.pc |= ((emulation->memory.flash[pc_prev] & 0x1F0) << 13);

    if(0 != (emulation->memory.flash[emulation->memory.pc] & 0x2))
    {
      result = slf_var_emu_stack_push_pc(emulation, (pc_prev + 2));
      emulation->op_cycles_remaining = 2;
      SL_AVR_EMU_VERBOSE_LOG(printf("CALL. PC 0x%06x\n", emulation->memory.pc));
    }
    else
    {
      emulation->op_cycles_remaining = 2;
      SL_AVR_EMU_VERBOSE_LOG(printf("JMP. PC 0x%06x\n", emulation->memory.pc));
    }
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

  if( SL_AVR_EMU_IS_JMP_CALL(emulation->memory.flash[emulation->memory.pc]) )
  {
    result = sl_avr_emu_opcode_jmp_call(emulation);
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