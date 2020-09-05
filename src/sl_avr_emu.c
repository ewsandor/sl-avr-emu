/**
 * @file sl_avr_emu.cpp
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Main File
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include <string.h>
#include "sl_avr_emu.h"

/* Global flag to enable/disable verbose logging */
bool sl_avr_emu_verbose_logging_enabled = true;

sl_avr_emu_result_e sl_avr_emu_init(sl_avr_emu_emulation_s *emulation)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  memset(emulation, 0, sizeof(sl_avr_emu_emulation_s));

  return result;
}

int main(int argc, char *argv[])
{
  sl_avr_emu_result_e    result;
  sl_avr_emu_emulation_s emulation;

  sl_avr_emu_init(&emulation);

  while(1)
  {
    result = sl_avr_emu_tick(&emulation);
    if(result != SL_AVR_EMU_RESULT_SUCCESS)
    {
      fprintf(stderr, "Error! Tick result %u\n", result);
      break;
    }
  }

  return result;
}