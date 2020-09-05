/**
 * @file sl_avr_emu_hex.h
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Hex Loading Header
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

 #ifndef _SL_AVR_EMU_HEX_H_
 #define _SL_AVR_EMU_HEX_H_

#include "sl_avr_emu.h"

/**
 * @brief Loads a .hex file into emulation's flash
 * 
 * @param emulation 
 * @param file_path 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_load_hex(sl_avr_emu_emulation_s *emulation, char * file_path);

 #endif //_SL_AVR_EMU_HEX_H_