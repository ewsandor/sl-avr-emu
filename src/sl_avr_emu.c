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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sl_avr_emu.h"
#include "sl_avr_emu_timer.h"
#include "sl_avr_emu_hex.h"

/* Global flag to enable/disable verbose logging */
bool sl_avr_emu_verbose_logging_enabled = false;

sl_avr_emu_result_e sl_avr_emu_init(sl_avr_emu_emulation_s *emulation);

void *sl_avr_emu_clock_thread(void *);
void *sl_avr_emu_cpu_thread  (void *);
void *sl_avr_emu_io_thread   (void *);

int main(int argc, char *argv[])
{
  char input_buffer[1024];
  uint32_t i;
  sl_avr_emu_result_e    result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_emulation_s emulation;
  pthread_t tick_thread;

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

  printf("Starting CPU thread\n");
  pthread_create(&tick_thread, NULL, sl_avr_emu_cpu_thread, &emulation);
  printf("Starting IO thread\n");
  pthread_create(&tick_thread, NULL, sl_avr_emu_io_thread, &emulation);
  printf("Starting Clock thread\n");
  pthread_create(&tick_thread, NULL, sl_avr_emu_clock_thread, &emulation);

  while(SL_AVR_EMU_RESULT_SUCCESS == result)
  {
    scanf("%s", input_buffer);
  }

  return result;
}

sl_avr_emu_result_e sl_avr_emu_init(sl_avr_emu_emulation_s *emulation)
{
  printf("Initializing AVR Emulation\n");

  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;

  memset(emulation, 0, sizeof(sl_avr_emu_emulation_s));

  printf("Initializing Timer 0\n");
  result = sl_avr_emu_configure_timer0(&emulation->memory, &emulation->timer0);

  return result;
}

void *sl_avr_emu_clock_thread(void *emulation_ptr)
{
  sl_avr_emu_emulation_s *emulation = (sl_avr_emu_emulation_s *) emulation_ptr;
  time_t host_last, host_now, host_start, elapsed_time;
  sl_avr_emu_tick_count_t cpu_diff, io_diff;

  if(emulation)
  {
    time(&host_start);
    host_last = host_start;
    while(1)
    {
      time(&host_now);

      emulation->clock_tick_count += (host_now-host_last) * SL_AVR_EMU_CLOCKS_PER_SEC/2;
      host_last = host_now;
      elapsed_time = host_now-host_start;

      cpu_diff = (emulation->clock_tick_count - emulation->cpu_tick_count);
      io_diff  = (emulation->clock_tick_count - emulation->io_tick_count);

      if(cpu_diff > 1)
      {
        fprintf(stderr, "WARNING! CPU thread lagging %lu ticks.  Seconds %lu.  Ticks lagged per second %lu\n", cpu_diff, elapsed_time, cpu_diff/elapsed_time);
      }
      if(io_diff > 1)
      {
        fprintf(stderr, "WARNING!  IO thread lagging %lu ticks.  Seconds %lu.  Ticks lagged per second %lu\n", io_diff, elapsed_time, io_diff/elapsed_time);
      }
    }
  }
  else
  {
    fprintf(stderr, "NULL emulation pointer!\n");
    exit(1);
  }
}

void *sl_avr_emu_cpu_thread(void *emulation_ptr)
{
  sl_avr_emu_result_e     result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_emulation_s *emulation = (sl_avr_emu_emulation_s *) emulation_ptr;
  sl_avr_emu_tick_count_t tick_lag;

  if(emulation)
  {
    while(SL_AVR_EMU_RESULT_SUCCESS == result)
    {
      tick_lag = emulation->clock_tick_count - emulation->cpu_tick_count;

      if(tick_lag > 0)
      {
        result = sl_avr_emu_cpu_tick(emulation);
      }
    }

    fprintf(stderr, "Error! CPU thread result %u\n", result);
    exit(result);
  }
  else
  {
    fprintf(stderr, "NULL emulation pointer!\n");
    exit(1);
  }
}

void *sl_avr_emu_io_thread(void *emulation_ptr)
{
  sl_avr_emu_result_e     result = SL_AVR_EMU_RESULT_SUCCESS;
  sl_avr_emu_emulation_s *emulation = (sl_avr_emu_emulation_s *) emulation_ptr;
  sl_avr_emu_tick_count_t tick_lag;

  if(emulation)
  {
    while(SL_AVR_EMU_RESULT_SUCCESS == result)
    {
      tick_lag = emulation->clock_tick_count - emulation->io_tick_count;

      if(tick_lag > 0)
      {
        result = sl_avr_emu_io_tick(emulation);
      }
    }

    fprintf(stderr, "Error! CPU thread result %u\n", result);
    exit(result);
  }
  else
  {
    fprintf(stderr, "NULL emulation pointer!\n");
    exit(1);
  }
}