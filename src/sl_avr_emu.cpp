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
#include "sl_avr_emu.hpp"

int main(int argc, char *argv[])
{
  sl_avr_emu_emulation_s emulation;

  memset(&emulation, 0, sizeof(sl_avr_emu_emulation_s));

  printf("PC 0x%x\n", emulation.memory.pc);
}