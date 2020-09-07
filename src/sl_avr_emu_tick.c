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

#define SL_AVR_EMU_IS_ADD(opcode)        (((opcode) & 0xEC00) == 0x0C00)
#define SL_AVR_EMU_IS_AND(opcode)        (((opcode) & 0xFC00) == 0x2000)
#define SL_AVR_EMU_IS_BRBS_BRBC(opcode)  (((opcode) & 0xF800) == 0xF000)
#define SL_AVR_EMU_IS_CP_CPC(opcode)     (((opcode) & 0xEC00) == 0x0400)
#define SL_AVR_EMU_IS_CPI(opcode)        (((opcode) & 0xF000) == 0x3000)
#define SL_AVR_EMU_IS_CPSE(opcode)       (((opcode) & 0xFC00) == 0x1000)
#define SL_AVR_EMU_IS_EOR(opcode)        (((opcode) & 0xFC00) == 0x2400)
#define SL_AVR_EMU_IS_IN_OUT(opcode)     (((opcode) & 0xF000) == 0xB000)
#define SL_AVR_EMU_IS_JMP_CALL(opcode)   (((opcode) & 0xFE0C) == 0x940C)
#define SL_AVR_EMU_IS_PUSH_POP(opcode)   (((opcode) & 0xFC0F) == 0x900F)
#define SL_AVR_EMU_IS_LD_ST(opcode)      ((((opcode) & 0xFC0C) == 0x900C) && !SL_AVR_EMU_IS_PUSH_POP(opcode))
#define SL_AVR_EMU_IS_LDI(opcode)        (((opcode) & 0xF000) == 0xE000)
#define SL_AVR_EMU_IS_LDS_STS(opcode)    (((opcode) & 0xFC0F) == 0x9000)
#define SL_AVR_EMU_IS_LPM_ELPM(opcode)   (((opcode) & 0xFFEF) == 0x95C8)
#define SL_AVR_EMU_IS_LPMZ_ELPMZ(opcode) (((opcode) & 0xFE0C) == 0x9004)
#define SL_AVR_EMU_IS_MOV(opcode)        (((opcode) & 0xFC00) == 0x2C00)
#define SL_AVR_EMU_IS_MOVW(opcode)       (((opcode) & 0xFF00) == 0x0100)
#define SL_AVR_EMU_IS_OR(opcode)         (((opcode) & 0xFC00) == 0x2800)
#define SL_AVR_EMU_IS_ORI(opcode)        (((opcode) & 0xF000) == 0x6000)
#define SL_AVR_EMU_IS_RET(opcode)        (((opcode) & 0xFFFF) == 0x9508)
#define SL_AVR_EMU_IS_RJMP_RCALL(opcode) (((opcode) & 0xE000) == 0xC000)
#define SL_AVR_EMU_IS_SBIC_SBIS(opcode)  (((opcode) & 0xFD00) == 0x9900)
#define SL_AVR_EMU_IS_SEX_CLX(opcode)    (((opcode) & 0xFF0F) == 0x9408)
#define SL_AVR_EMU_IS_SUBI_SBCI(opcode)  (((opcode) & 0xE000) == 0x4000)

#define SL_AVR_EMU_IS_TWO_WORD_OPCODE(opcode) (SL_AVR_EMU_IS_JMP_CALL(opcode) || SL_AVR_EMU_IS_LDS_STS(opcode))

sl_avr_emu_extended_address_t sl_avr_emu_get_x_address(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_extended_address_t result = 0;

  result = emulation->memory.data[SL_AVR_EMU_XL_ADDRESS] | emulation->memory.data[SL_AVR_EMU_XH_ADDRESS] << 8;

  if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
  {
    result |= emulation->memory.data[SL_AVR_EMU_RAMPX_ADDRESS] << 16;
  }

  return result;
}
sl_avr_emu_result_e sl_avr_emu_set_x_address(sl_avr_emu_emulation_s * emulation, sl_avr_emu_extended_address_t address)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  emulation->memory.data[SL_AVR_EMU_XL_ADDRESS] = address & 0xFF;
  emulation->memory.data[SL_AVR_EMU_XH_ADDRESS] = (address >> 8) & 0xFF;

  if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
  {
    emulation->memory.data[SL_AVR_EMU_RAMPX_ADDRESS] = (address >> 16) & 0xFF;
  }

  return result;
}

sl_avr_emu_extended_address_t sl_avr_emu_get_z_address(sl_avr_emu_emulation_s * emulation, bool extended)
{
  sl_avr_emu_extended_address_t result = 0;

  result = emulation->memory.data[SL_AVR_EMU_ZL_ADDRESS] | emulation->memory.data[SL_AVR_EMU_ZH_ADDRESS] << 8;
  if(extended)
  {
    result |= emulation->memory.data[SL_AVR_EMU_RAMPZ_ADDRESS] << 16;
  }

  return result;
}
sl_avr_emu_result_e sl_avr_emu_set_z_address(sl_avr_emu_emulation_s * emulation, sl_avr_emu_extended_address_t address, bool extended)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  emulation->memory.data[SL_AVR_EMU_ZL_ADDRESS] = address & 0xFF;
  emulation->memory.data[SL_AVR_EMU_ZH_ADDRESS] = (address >> 8) & 0xFF;
  if(extended)
  {
    emulation->memory.data[SL_AVR_EMU_RAMPX_ADDRESS] = (address >> 16) & 0xFF;
  }

  return result;
}


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

sl_avr_emu_result_e sl_avr_emu_stack_pop_byte(sl_avr_emu_emulation_s * emulation, sl_avr_emu_byte_t *byte)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_extended_address_t sp;

  if(byte != NULL)
  {
    sp = emulation->memory.data[SL_AVR_EMU_SPL_ADDRESS];

    if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
    {
      sp |= (emulation->memory.data[SL_AVR_EMU_SPH_ADDRESS] << 8);
    }

    if(SL_AVR_EMU_DATA_ADDRESS_VALID(sp))
    {
      if(sp < SL_AVR_EMU_DATA_SIZE-1)
      {
        sp++;
        emulation->memory.data[SL_AVR_EMU_SPL_ADDRESS] = (sp & 0xFF);
        if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
        {
          emulation->memory.data[SL_AVR_EMU_SPH_ADDRESS] = ((sp >> 8) & 0xFF);
        }

        *byte = emulation->memory.data[sp];
      }
      else 
      {
        result = SL_AVR_EMU_RESULT_STACK_UNDERFLOW;
      }
    }
    else 
    {
      result = SL_AVR_EMU_RESULT_INVALID_DATA_ADDRESS;
    }
  }
  else
  {
    result = SL_AVR_EMU_RESULT_FAILURE;
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

sl_avr_emu_result_e slf_var_emu_stack_pop_pc(sl_avr_emu_emulation_s * emulation, sl_avr_emu_extended_address_t *pc)
{
  sl_avr_emu_byte_t byte;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(pc != NULL)
  {
    *pc = 0;
    if(SL_AVR_EMU_EXTENDED_PC_ADDRESS)
    {
      result = sl_avr_emu_stack_pop_byte(emulation, &byte);
      *pc |= byte << 16;
    }
    if(SL_AVR_EMU_RESULT_SUCCESS == result)
    { 
      result = sl_avr_emu_stack_pop_byte(emulation, &byte);
      *pc |= byte << 8;
    }
    if(SL_AVR_EMU_RESULT_SUCCESS == result)
    { 
      result = sl_avr_emu_stack_pop_byte(emulation, &byte);
      *pc |= byte;
    }
  }
  else
  {
    result = SL_AVR_EMU_RESULT_FAILURE;
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
sl_avr_emu_result_e sl_avr_emu_opcode_unsupported(sl_avr_emu_emulation_s * emulation)
{
  /* Unrecognized OPCODE Handling */
  fprintf(stderr, "Unsupported OPCODE: 0x%04x. PC Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc], emulation->memory.pc);
  if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc + 1))
  {
    fprintf(stderr, "Next OPCODE: 0x%04x. PC+1 Address: 0x%06x\n", emulation->memory.flash[emulation->memory.pc+1], emulation->memory.pc+1);
  }

  return SL_AVR_EMU_RESULT_UNSUPPORTED_OPCODE;
}

sl_avr_emu_result_e sl_avr_emu_opcode_add(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  bool                 with_carry  = 0;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;
  sl_avr_emu_byte_t    d_data, r_data, sum;

  source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);
  with_carry  = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 12);

  r_data = emulation->memory.data[source];
  d_data = emulation->memory.data[destination];
  sum = r_data+d_data;
  if(with_carry && SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG))
  {
    sum++;
  }

  emulation->memory.data[destination] = sum;

  if(  (SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(r_data, 3))  ||
      (!SL_AVR_EMU_CHECK_BIT(sum, 3)     && SL_AVR_EMU_CHECK_BIT(r_data, 3)) ||
      (!SL_AVR_EMU_CHECK_BIT(sum, 3)     && SL_AVR_EMU_CHECK_BIT(d_data, 3)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }

  if(  (SL_AVR_EMU_CHECK_BIT(d_data, 7) &&   SL_AVR_EMU_CHECK_BIT(r_data, 7) && !SL_AVR_EMU_CHECK_BIT(sum, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7) &&  !SL_AVR_EMU_CHECK_BIT(r_data, 7) &&  SL_AVR_EMU_CHECK_BIT(sum, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }

  if( SL_AVR_EMU_CHECK_BIT(d_data, 7) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }

  if(0 == sum)
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if(  (SL_AVR_EMU_CHECK_BIT(d_data, 7) && SL_AVR_EMU_CHECK_BIT(r_data, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(sum, 7)    && SL_AVR_EMU_CHECK_BIT(r_data, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(sum, 7))   && SL_AVR_EMU_CHECK_BIT(d_data, 7))
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }

  if(SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG) ^ SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG))
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("%s. PC 0x%06x. dest 0x%04x, r_data 0x%02x, d_data 0x%02x, sum 0x%02x, sreg 0x%02x\n", (with_carry)?"ADC":"ADD", emulation->memory.pc, destination, r_data, d_data, sum, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_and(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;

  source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);

  emulation->memory.data[destination] &= emulation->memory.data[source];

  if(0 == emulation->memory.data[destination])
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( 0 != (emulation->memory.data[destination] & 0x80) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("AND. PC 0x%06x. dest 0x%04x, r_data 0x%04x\n", emulation->memory.pc, destination, emulation->memory.data[destination]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_cp_cpc(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e  result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_byte_t    compare, d_data, r_data;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;
  bool with_carry;

  source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);
  with_carry  = !SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 12);

  r_data = emulation->memory.data[source];
  d_data = emulation->memory.data[destination];

  compare = d_data - r_data;

  if(with_carry && SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG))
  {
    compare--;
  }

  if( (!SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(r_data, 3))  ||
       (SL_AVR_EMU_CHECK_BIT(compare, 3) && SL_AVR_EMU_CHECK_BIT(r_data, 3)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(compare, 3)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }

  if(  (SL_AVR_EMU_CHECK_BIT(d_data, 7) && !SL_AVR_EMU_CHECK_BIT(r_data, 7) && !SL_AVR_EMU_CHECK_BIT(compare, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7) &&  SL_AVR_EMU_CHECK_BIT(r_data, 7) &&  SL_AVR_EMU_CHECK_BIT(compare, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }

  if( SL_AVR_EMU_CHECK_BIT(d_data, 7) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }

  if(0 == compare && (!with_carry || (with_carry && SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG))) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( (!SL_AVR_EMU_CHECK_BIT(d_data, 7)  && SL_AVR_EMU_CHECK_BIT(r_data, 7)) ||
       (SL_AVR_EMU_CHECK_BIT(compare, 7) && SL_AVR_EMU_CHECK_BIT(r_data, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7)  && SL_AVR_EMU_CHECK_BIT(compare, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }

  if(SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG) ^ SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG))
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;

  if(with_carry)
  {
    SL_AVR_EMU_VERBOSE_LOG(printf("CPC. PC 0x%06x. sreg 0x%04x, d_data 0x%04x, r_data 0x%02x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], d_data, r_data));
  }
  else 
  {
    SL_AVR_EMU_VERBOSE_LOG(printf("CP. PC 0x%06x. sreg 0x%04x, d_data 0x%04x, r_data 0x%02x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], d_data, r_data));
  }

  return result;
}


sl_avr_emu_result_e sl_avr_emu_opcode_cpi(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e  result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_byte_t    compare, d_data, k_data;
  sl_avr_emu_address_t destination = 0;

  k_data      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF0);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF) | 0x10;

  d_data = emulation->memory.data[destination];
  compare = d_data - k_data;

  if( (!SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(k_data, 3))  ||
       (SL_AVR_EMU_CHECK_BIT(compare, 3) && SL_AVR_EMU_CHECK_BIT(k_data, 3)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(compare, 3)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }

  if(  (SL_AVR_EMU_CHECK_BIT(d_data, 7) && !SL_AVR_EMU_CHECK_BIT(k_data, 7) && !SL_AVR_EMU_CHECK_BIT(compare, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7) &&  SL_AVR_EMU_CHECK_BIT(k_data, 7) &&  SL_AVR_EMU_CHECK_BIT(compare, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }

  if( SL_AVR_EMU_CHECK_BIT(d_data, 7) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }

  if(0 == compare)
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( (!SL_AVR_EMU_CHECK_BIT(d_data, 7) && SL_AVR_EMU_CHECK_BIT(k_data, 7)) ||
       (SL_AVR_EMU_CHECK_BIT(compare, 7) && SL_AVR_EMU_CHECK_BIT(k_data, 7))                                       ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7) && SL_AVR_EMU_CHECK_BIT(compare, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }

  if(SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG) ^ SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG))
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("CPI. PC 0x%06x. sreg 0x%04x, data 0x%04x, k_data 0x%02x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], emulation->memory.data[destination], k_data));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_cpse(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e  result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;

  source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

  if(emulation->memory.data[source] == emulation->memory.data[destination])
  {
    if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc+1))
    {
      if(SL_AVR_EMU_IS_TWO_WORD_OPCODE(emulation->memory.flash[emulation->memory.pc+1]))
      {
        if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
        {
          result = sl_avr_emu_opcode_unsupported(emulation);
        }
        else
        {
          emulation->memory.pc += 3;
          emulation->op_cycles_remaining = 2;
        }
      }
      else 
      {
        emulation->memory.pc += 2;
        emulation->op_cycles_remaining = 1;
      }
    }
    else 
    {
      result = SL_AVR_EMU_RESULT_INVALID_PC;
    }
  }
  else 
  {
    emulation->memory.pc++;
  }

  SL_AVR_EMU_VERBOSE_LOG(printf("CPSE. PC 0x%06x. r_data 0x%02x, d_data 0x%02x\n", emulation->memory.pc, emulation->memory.data[source], emulation->memory.data[destination]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_eor(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;

  source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

  emulation->memory.data[destination] ^= emulation->memory.data[source];

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);

  if(0 == emulation->memory.data[destination])
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( 0 != (emulation->memory.data[destination] & 0x80) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("EOR. PC 0x%06x. dest 0x%04x, data 0x%04x\n", emulation->memory.pc, destination, emulation->memory.data[destination]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_mov(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;

  if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
  {
    result = sl_avr_emu_opcode_unsupported(emulation);
  }
  else 
  {
    source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
    destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

    emulation->memory.data[destination]   = emulation->memory.data[source];

    emulation->memory.pc++;
    SL_AVR_EMU_VERBOSE_LOG(printf("MOV. PC 0x%06x. source 0x%04x, dest 0x%04x, data 0x%02x\n", emulation->memory.pc, source, destination, emulation->memory.data[destination]));
  }

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_movw(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;

  if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
  {
    result = sl_avr_emu_opcode_unsupported(emulation);
  }
  else 
  {
    source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) << 1;
    destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF) << 1;

    emulation->memory.data[destination]   = emulation->memory.data[source];
    emulation->memory.data[destination+1] = emulation->memory.data[source+1];

    emulation->memory.pc++;
    SL_AVR_EMU_VERBOSE_LOG(printf("MOVW. PC 0x%06x. source 0x%04x, dest 0x%04x, data 0x%02x%02x\n", emulation->memory.pc, source, destination, emulation->memory.data[destination+1], emulation->memory.data[destination]));
  }

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_subi_sbci(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e  result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_byte_t    difference, d_data, k_data;
  sl_avr_emu_address_t destination = 0;
  bool with_carry;

  k_data      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF0);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF) | 0x10;
  with_carry  = !SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 12);

  d_data = emulation->memory.data[destination];

  difference = d_data - k_data;

  if(with_carry && SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG))
  {
    difference--;
  }

  if( (!SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(k_data, 3))  ||
       (SL_AVR_EMU_CHECK_BIT(difference, 3) && SL_AVR_EMU_CHECK_BIT(k_data, 3)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 3)  && SL_AVR_EMU_CHECK_BIT(difference, 3)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_HALF_CARRY_FLAG);
  }

  if(  (SL_AVR_EMU_CHECK_BIT(d_data, 7) && !SL_AVR_EMU_CHECK_BIT(k_data, 7) && !SL_AVR_EMU_CHECK_BIT(difference, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7) &&  SL_AVR_EMU_CHECK_BIT(k_data, 7) &&  SL_AVR_EMU_CHECK_BIT(difference, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);
  }

  if( SL_AVR_EMU_CHECK_BIT(d_data, 7) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
  }

  if(0 == difference && (!with_carry || (with_carry && SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG))) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( (!SL_AVR_EMU_CHECK_BIT(d_data, 7)  && SL_AVR_EMU_CHECK_BIT(k_data, 7)) ||
       (SL_AVR_EMU_CHECK_BIT(difference, 7) && SL_AVR_EMU_CHECK_BIT(k_data, 7)) ||
      (!SL_AVR_EMU_CHECK_BIT(d_data, 7)  && SL_AVR_EMU_CHECK_BIT(difference, 7)) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_CARRY_FLAG);
  }

  if(SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG) ^ SL_AVR_EMU_CHECK_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG))
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("%s. PC 0x%06x. dest 0x%04x, d_data 0x%02x, k_data 0x%02x, difference 0x%02x, sreg 0x%02x\n", (with_carry)?"SBCI":"SUBI", emulation->memory.pc, destination, d_data, k_data, difference, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_or(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t source      = 0;
  sl_avr_emu_address_t destination = 0;

  source      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x10);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

  emulation->memory.data[destination] |= emulation->memory.data[source];

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);

  if(0 == emulation->memory.data[destination])
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( 0 != (emulation->memory.data[destination] & 0x80) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("OR. PC 0x%06x. dest 0x%04x, data 0x%04x\n", emulation->memory.pc, destination, emulation->memory.data[destination]));

  return result;
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
  else if(SL_AVR_EMU_IS_ADD(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_add(emulation);
  }
  else if(SL_AVR_EMU_IS_AND(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_and(emulation);
  }
  else if(SL_AVR_EMU_IS_CP_CPC(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_cp_cpc(emulation);
  }
  else if(SL_AVR_EMU_IS_CPI(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_cpi(emulation);
  }
  else if(SL_AVR_EMU_IS_CPSE(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_cpse(emulation);
  }
  else if(SL_AVR_EMU_IS_EOR(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_eor(emulation);
  }
  else if(SL_AVR_EMU_IS_MOV(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_mov(emulation);
  }
  else if(SL_AVR_EMU_IS_MOVW(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_movw(emulation);
  }
  else if(SL_AVR_EMU_IS_OR(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_or(emulation);
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }
  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_ori(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t destination = 0;
  sl_avr_emu_byte_t    k_data      = 0;

  k_data      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF0);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF) | 0x10;

  emulation->memory.data[destination] |= k_data;

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);

  if(0 == emulation->memory.data[destination])
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_ZERO_FLAG);
  }

  if( 0 != (emulation->memory.data[destination] & 0x80) )
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_SET_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_NEGATIVE_FLAG);
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_SIGN_FLAG);
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("ORI. PC 0x%06x. sreg 0x%02x dest 0x%02x, R_data 0x%02x, k_data 0x%02x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], destination, emulation->memory.data[destination], k_data));

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
  if(SL_AVR_EMU_IS_ORI(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_ori(emulation);
  }
  else if(SL_AVR_EMU_IS_SUBI_SBCI(emulation->memory.flash[emulation->memory.pc]))
  {
    sl_avr_emu_opcode_subi_sbci(emulation);
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }
  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_in_out(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  sl_avr_emu_address_t io_address  = 0;
  sl_avr_emu_address_t destination = 0;

  io_address  = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 5) & 0x30);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);

  if(SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 11))
  {
    emulation->memory.pc++;
    emulation->memory.data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(io_address)] = emulation->memory.data[destination];
    SL_AVR_EMU_VERBOSE_LOG(printf("OUT. PC 0x%06x.   io 0x%04x, data 0x%04x\n", emulation->memory.pc, io_address, emulation->memory.data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(io_address)]));
  }
  else {
    emulation->memory.pc++;
    emulation->memory.data[destination] = emulation->memory.data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(io_address)];
    SL_AVR_EMU_VERBOSE_LOG(printf("IN. PC 0x%06x. dest 0x%04x, data 0x%04x\n", emulation->memory.pc, destination, emulation->memory.data[destination]));
  }

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_jmp_call(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_extended_address_t pc_prev;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
  {
    result = sl_avr_emu_opcode_unsupported(emulation);
  }
  else if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc + 1))
  {
    pc_prev = emulation->memory.pc;
    emulation->memory.pc = emulation->memory.flash[pc_prev+1];
    emulation->memory.pc |= ((emulation->memory.flash[pc_prev] & 0x1  ) << 16);
    emulation->memory.pc |= ((emulation->memory.flash[pc_prev] & 0x1F0) << 13);

    if(0 != (emulation->memory.flash[pc_prev] & 0x2))
    {
      result = slf_var_emu_stack_push_pc(emulation, (pc_prev + 2));
      emulation->op_cycles_remaining = (SL_AVR_EMU_VERSION_AVRE == emulation->version)?3:2;
      if(SL_AVR_EMU_EXTENDED_PC_ADDRESS)
      {
        emulation->op_cycles_remaining++;
      }
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

sl_avr_emu_result_e sl_avr_emu_opcode_ld_st(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  bool inc,dec,store;
  sl_avr_emu_address_t destination = 0;
  sl_avr_emu_extended_address_t x_address = sl_avr_emu_get_x_address(emulation);

  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F);

  inc   = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 0);
  dec   = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 1);
  store = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 9);

  if(inc && dec)
  {
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }
  else 
  {
    if(dec)
    {
      x_address--;
      result = sl_avr_emu_set_x_address(emulation, x_address);
    }

    if(SL_AVR_EMU_RESULT_SUCCESS == result && SL_AVR_EMU_DATA_ADDRESS_VALID(x_address))
    {
      emulation->memory.pc++;
      if(store)
      {
        emulation->memory.data[x_address] = emulation->memory.data[destination];
        SL_AVR_EMU_VERBOSE_LOG(printf("ST. PC 0x%06x. x 0x%06x, data 0x%04x\n", emulation->memory.pc, x_address, emulation->memory.data[destination]));
      }
      else 
      {
        emulation->memory.data[destination] = emulation->memory.data[x_address];
        SL_AVR_EMU_VERBOSE_LOG(printf("LD. PC 0x%06x. x 0x%06x, data 0x%04x\n", emulation->memory.pc, x_address, emulation->memory.data[destination]));
      }

      if(inc)
      {
        x_address++;
        result = sl_avr_emu_set_x_address(emulation, x_address);
      }

      if(SL_AVR_EMU_VERSION_AVRXM == emulation->version && dec)
      {
        emulation->op_cycles_remaining = 2;
      }
      else if(SL_AVR_EMU_VERSION_AVRXM == emulation->version && !inc && !dec)
      {
        emulation->op_cycles_remaining = 0;
      }
      else 
      {
        emulation->op_cycles_remaining = 1;
      }
    }
    else 
    {
      result = SL_AVR_EMU_RESULT_INVALID_DATA_ADDRESS;
    }
  }

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_lds_sts(sl_avr_emu_emulation_s * emulation)
{
  bool set;
  sl_avr_emu_extended_address_t destination;
  sl_avr_emu_extended_address_t ram_address;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
  {
    /* TODO RC version support */
    result = sl_avr_emu_opcode_unsupported(emulation);
  }
  else if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc + 1))
  {
    set = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 9);
    destination = (emulation->memory.flash[emulation->memory.pc] >> 4) & 0x1F;
    ram_address = emulation->memory.flash[emulation->memory.pc + 1];
    if(SL_AVR_EMU_EXTENDED_DATA_ADDRESS)
    {
      ram_address |= (emulation->memory.data[SL_AVR_EMU_RAMPD_ADDRESS] << 16);
    }

    emulation->memory.pc += 2;
    if(set)
    {
      emulation->memory.data[ram_address] = emulation->memory.data[destination];
      SL_AVR_EMU_VERBOSE_LOG(printf("STS. PC 0x%06x. ram_address 0x%06x, dest 0x%02x, r_data 0x%02x\n", emulation->memory.pc, ram_address, destination, emulation->memory.data[destination]));

      emulation->op_cycles_remaining = 1;
    }
    else
    {
      emulation->memory.data[destination] = emulation->memory.data[ram_address];
      SL_AVR_EMU_VERBOSE_LOG(printf("LDS. PC 0x%06x. ram_address 0x%06x, dest 0x%02x, r_data 0x%02x\n", emulation->memory.pc, ram_address, destination, emulation->memory.data[destination]));

      emulation->op_cycles_remaining = (SL_AVR_EMU_VERSION_AVRE == emulation->version)?1:2;
    }
  }
  else
  {
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_lpm_elpm(sl_avr_emu_emulation_s * emulation)
{
  bool extended, inc;
  sl_avr_emu_extended_address_t destination;
  sl_avr_emu_extended_address_t z_pointer;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  if(SL_AVR_EMU_IS_LPM_ELPM(emulation->memory.flash[emulation->memory.pc]))
  {
    inc = 0;
    extended = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 4);
    destination = 0x0;
  }
  else 
  {
    inc      = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 0);
    extended = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 1);
    destination = (emulation->memory.flash[emulation->memory.pc]>>4) & 0x1F;
  }

  if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
  {
    result = sl_avr_emu_opcode_unsupported(emulation);
  }
  else 
  {
    emulation->op_cycles_remaining = 2;
  }

  z_pointer = sl_avr_emu_get_z_address(emulation, extended);

  if(SL_AVR_EMU_RESULT_SUCCESS == result && SL_AVR_EMU_FLASH_ADDRESS_VALID(z_pointer))
  {
    emulation->memory.data[destination] = emulation->memory.flash[z_pointer];

    if(inc)
    {
      z_pointer++;
      result = sl_avr_emu_set_z_address(emulation, z_pointer, extended);
    }
  }
  else
  {
    result = SL_AVR_EMU_RESULT_INVALID_FLASH_ADDRESS;
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("%s. PC 0x%06x. z_address 0x%06x, dest 0x%02x, r_data 0x%02x\n", (extended)?"ELPM":"LPM", emulation->memory.pc, z_pointer, destination, emulation->memory.data[destination]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_push_pop(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_extended_address_t destination;
  bool                push;

  push = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 9);
  destination = (emulation->memory.flash[emulation->memory.pc]>>4) & 0x1F;
  
  if(push)
  {
    result = sl_avr_emu_stack_push_byte(emulation, emulation->memory.data[destination]);
    if(SL_AVR_EMU_VERSION_AVRE == emulation->version)
    {
      emulation->op_cycles_remaining = 1;
    }
  }
  else 
  {
    result = sl_avr_emu_stack_pop_byte(emulation, &emulation->memory.data[destination]);

    if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
    {
      emulation->op_cycles_remaining = 2;
    }
    else 
    {
      emulation->op_cycles_remaining = 1;
    }
  }

  emulation->memory.pc++;
  SL_AVR_EMU_VERBOSE_LOG(printf("%s. PC 0x%06x. d_data 0x%02x\n", (push)?"PUSH":"POP", emulation->memory.pc, emulation->memory.data[destination]));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_ret(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  
  result = slf_var_emu_stack_pop_pc(emulation, &emulation->memory.pc);

  if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
  {
    emulation->op_cycles_remaining = 5;
  }
  else 
  {
    emulation->op_cycles_remaining = 3;
  }

  if(SL_AVR_EMU_EXTENDED_PC_ADDRESS)
  { 
    if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
    {
      result = sl_avr_emu_opcode_unsupported(emulation);
    }
    else
    {
      emulation->op_cycles_remaining++;
    }
  }

  SL_AVR_EMU_VERBOSE_LOG(printf("RET. PC 0x%06x.\n", emulation->memory.pc));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_sbic_sbis(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e  result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_address_t io_address = 0;
  sl_avr_emu_address_t io_bit     = 0;
  bool skip, if_set;

  io_bit     = (emulation->memory.flash[emulation->memory.pc] & 0x7);
  io_address = ((emulation->memory.flash[emulation->memory.pc] >> 3) & 0x1F);
  if_set = SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 9);

  skip = SL_AVR_EMU_CHECK_BIT(emulation->memory.data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(io_address)], io_bit);

  if(!if_set)
  {
    skip = !skip;
  }

  if(skip)
  {
    if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc+1))
    {
      if(SL_AVR_EMU_IS_TWO_WORD_OPCODE(emulation->memory.flash[emulation->memory.pc+1]))
      {
        if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
        {
          result = sl_avr_emu_opcode_unsupported(emulation);
        }
        else
        {
          emulation->memory.pc += 3;
          emulation->op_cycles_remaining = 2;
        }
      }
      else 
      {
        emulation->memory.pc += 2;
        emulation->op_cycles_remaining = 1;
      }
    }
    else 
    {
      result = SL_AVR_EMU_RESULT_INVALID_PC;
    }
  }
  else 
  {
    emulation->memory.pc++;
  }

  if(SL_AVR_EMU_VERSION_AVRXM == emulation->version)
  {
    emulation->op_cycles_remaining++;
  }

  SL_AVR_EMU_VERBOSE_LOG(printf("%s. PC 0x%06x. io_address 0x%04x io_data 0x%02x io_bit 0x%x\n", (if_set)?"SBIS":"SBIC", emulation->memory.pc, io_address, emulation->memory.data[SL_AVR_EMU_IO_TO_DATA_ADDRESS(io_address)], io_bit));

  return result;
}


sl_avr_emu_result_e sl_avr_emu_opcode_sex_clx(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  bool                   set;
  sl_avr_emu_bit_index_t mod_bit;

  mod_bit = emulation->memory.flash[emulation->memory.pc] >> 4 & 0x7;
  set = !SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 7);

  emulation->memory.pc++;
  if(set)
  {
    SL_AVR_EMU_SET_SREG_BIT(*emulation, mod_bit);
    SL_AVR_EMU_VERBOSE_LOG(printf("SEx. PC 0x%06x. sreg 0x%02x, mod_bit 0x%01x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], mod_bit));
  }
  else 
  {
    SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, mod_bit);
    SL_AVR_EMU_VERBOSE_LOG(printf("CLx. PC 0x%06x. sreg 0x%02x, mod_bit 0x%01x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], mod_bit));
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

  if( SL_AVR_EMU_IS_IN_OUT(emulation->memory.flash[emulation->memory.pc]) )
  {
    result = sl_avr_emu_opcode_in_out(emulation);
  }
  else if( SL_AVR_EMU_IS_JMP_CALL(emulation->memory.flash[emulation->memory.pc]) )
  {
    result = sl_avr_emu_opcode_jmp_call(emulation);
  }
  else if(SL_AVR_EMU_IS_LD_ST(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_ld_st(emulation);
  }
  else if(SL_AVR_EMU_IS_LDS_STS(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_lds_sts(emulation);
  }
  else if(SL_AVR_EMU_IS_LPM_ELPM(emulation->memory.flash[emulation->memory.pc]) ||
          SL_AVR_EMU_IS_LPMZ_ELPMZ(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_lpm_elpm(emulation);
  }
  else if(SL_AVR_EMU_IS_PUSH_POP(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_push_pop(emulation);
  }
  else if(SL_AVR_EMU_IS_RET(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_ret(emulation);
  }
  else if(SL_AVR_EMU_IS_SBIC_SBIS(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_sbic_sbis(emulation);
  }
  else if(SL_AVR_EMU_IS_SEX_CLX(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_sex_clx(emulation);
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = sl_avr_emu_opcode_unrecognized(emulation);
  }
  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_ldi(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  sl_avr_emu_byte_t    k_data  = 0;
  sl_avr_emu_address_t destination = 0;

  k_data      = (emulation->memory.flash[emulation->memory.pc] & 0xF) | ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF0);
  destination = ((emulation->memory.flash[emulation->memory.pc] >> 4) & 0xF) | 0x10;

  SL_AVR_EMU_CLEAR_SREG_BIT(*emulation, SL_AVR_EMU_SREG_OVERFLOW_FLAG);

  emulation->memory.pc++;
  emulation->memory.data[destination] = k_data;
  SL_AVR_EMU_VERBOSE_LOG(printf("LDI. PC 0x%06x. dest 0x%04x, data 0x%04x\n", emulation->memory.pc, destination, k_data));

  return result;
}

sl_avr_emu_result_e sl_avr_emu_opcode_brbs_brbc(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_extended_address_t pc_prev;
  sl_avr_emu_extended_address_t pc_relative;
  bool                          check_set, check;
  sl_avr_emu_bit_index_t        check_bit;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  pc_prev     = emulation->memory.pc;
  pc_relative = (emulation->memory.flash[emulation->memory.pc] >> 3) & 0x7F;
  check_bit   =  emulation->memory.flash[emulation->memory.pc] & 0x7;
  check_set   = !SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[emulation->memory.pc], 10);

  check = (SL_AVR_EMU_CHECK_SREG_BIT(*emulation, check_bit) == check_set);

  if(check)
  {
    if(SL_AVR_EMU_CHECK_BIT(pc_relative, 6))
    { 
      SL_AVR_EMU_SET_BIT(pc_relative, 7);
      emulation->memory.pc += (1 + (int8_t) pc_relative);
    }
    else
    {
      emulation->memory.pc += (1 + pc_relative);
    }
    emulation->op_cycles_remaining = 1;
  }
  else 
  {
    emulation->memory.pc++;
  }

  if(check_set)
  {
    SL_AVR_EMU_VERBOSE_LOG(printf("BRBS. PC 0x%06x. sreg 0x%04x, check %d, relative_pc 0x%x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], check, pc_relative));
  }
  else 
  {
    SL_AVR_EMU_VERBOSE_LOG(printf("BRBC. PC 0x%06x. sreg 0x%04x, check %d, relative_pc 0x%x\n", emulation->memory.pc, emulation->memory.data[SL_AVR_EMU_SREG_ADDRESS], check, pc_relative));
  }

  return result;
}


sl_avr_emu_result_e sl_avr_emu_opcode_rjmp_rcall(sl_avr_emu_emulation_s * emulation)
{
  sl_avr_emu_extended_address_t pc_prev;
  sl_avr_emu_extended_address_t pc_relative;
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  pc_prev     = emulation->memory.pc;
  pc_relative = emulation->memory.flash[emulation->memory.pc] & 0x0FFF;

  if(SL_AVR_EMU_CHECK_BIT(pc_relative, 11))
  { 
    pc_relative |= 0xF000;
    emulation->memory.pc += (1 + (int16_t) pc_relative);
  }
  else
  {
    emulation->memory.pc += (1 + pc_relative);
  }

  if(SL_AVR_EMU_CHECK_BIT(emulation->memory.flash[pc_prev], 12))
  {
    result = slf_var_emu_stack_push_pc(emulation, (pc_prev + 1));
    emulation->op_cycles_remaining = (SL_AVR_EMU_VERSION_AVRE == emulation->version || SL_AVR_EMU_VERSION_AVRRC == emulation->version)?2:1;
    if(SL_AVR_EMU_EXTENDED_PC_ADDRESS)
    {
      if(SL_AVR_EMU_VERSION_AVRRC == emulation->version)
      {
        result = sl_avr_emu_opcode_unsupported(emulation);
      }
      else
      {
        emulation->op_cycles_remaining++;
      }
    }
    SL_AVR_EMU_VERBOSE_LOG(printf("RCALL. PC 0x%06x\n", emulation->memory.pc));
  }
  else
  {
    emulation->op_cycles_remaining = 1;
    SL_AVR_EMU_VERBOSE_LOG(printf("RJMP. PC 0x%06x\n", emulation->memory.pc));
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

  if(SL_AVR_EMU_IS_BRBS_BRBC(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_brbs_brbc(emulation);
  }
  else if(SL_AVR_EMU_IS_LDI(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_ldi(emulation);
  }
  else if(SL_AVR_EMU_IS_RJMP_RCALL(emulation->memory.flash[emulation->memory.pc]))
  {
    result = sl_avr_emu_opcode_rjmp_rcall(emulation);
  }
  else
  {
    /* Unrecognized OPCODE Handling */
    result = sl_avr_emu_opcode_unrecognized(emulation);
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
  
  emulation->tick_count++;

  if(emulation->op_cycles_remaining > 0)
  {
    emulation->op_cycles_remaining--;
    SL_AVR_EMU_VERBOSE_LOG(printf("tick %u: %u cycles for remaining for current operation\n", emulation->tick_count, emulation->op_cycles_remaining));
  }
  else
  {
    if(SL_AVR_EMU_PC_ADDRESS_VALID(emulation->memory.pc))
    {
      SL_AVR_EMU_VERBOSE_LOG(printf("tick %u: PC 0x%06x. OP 0x%04x.\n", emulation->tick_count, emulation->memory.pc, emulation->memory.flash[emulation->memory.pc]));
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
      result = sl_avr_emu_opcode_unrecognized(emulation);
    }
  }

  return result;
}