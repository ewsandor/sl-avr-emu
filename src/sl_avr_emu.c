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
#include "sl_avr_emu_timer.h"
#include "sl_avr_emu_hex.h"

/* Global flag to enable/disable verbose logging */
bool sl_avr_emu_verbose_logging_enabled = false;

sl_avr_emu_result_e sl_avr_emu_init(sl_avr_emu_emulation_s *emulation)
{
  printf("Initializing AVR Emulation\n");

  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  memset(emulation, 0, sizeof(sl_avr_emu_emulation_s));

  printf("Initializing Timer 0\n");
  result = sl_avr_emu_configure_timer0(&emulation->memory, &emulation->timer0);

  return result;
}

int main(int argc, char *argv[])
{
  uint32_t i;
  sl_avr_emu_result_e    result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_emulation_s emulation;

  sl_avr_emu_init(&emulation);

  for(i = 1; i < argc; i++)
  {
    if(strcmp(argv[i],"-v") == 0)
    {
      printf("Verbose logging enabled.\n");
      sl_avr_emu_verbose_logging_enabled = true;
    }
    else if(strcmp(argv[i],"-h") == 0)
    {
      if((i+1) < argc)
      {
        result = sl_avr_emu_load_hex(&emulation, argv[i+1]);

        if(result != SL_AVR_EMU_RESULT_SUCCESS)
        {
          fprintf(stderr, "Error! Failed to load hex %u at %s\n", result, argv[2]);
          return result;
        }
      }
    }
  }

  while(SL_AVR_EMU_RESULT_SUCCESS == result)
  {
    result = sl_avr_emu_io_tick(&emulation);
    if(result != SL_AVR_EMU_RESULT_SUCCESS)
    {
      fprintf(stderr, "Error! IO Tick result %u\n", result);
    }
    else
    {
      result = sl_avr_emu_tick(&emulation);
      if(result != SL_AVR_EMU_RESULT_SUCCESS)
      {
        fprintf(stderr, "Error! Tick result %u\n", result);
      }
    }
  }

  return result;
}