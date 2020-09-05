/**
 * @file sl_avr_emu.hpp
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Main Header
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_HPP_
#define _SL_AVR_EMU_HPP_

#include "sl_avr_emu_tick.hpp"
#include "sl_avr_emu_types.hpp"

extern bool sl_avr_emu_verbose_logging_enabled;

#define SL_AVR_EMU_VERBOSE_LOG(log_command) { if(sl_avr_emu_verbose_logging_enabled) {log_command;} }

#endif  //_SL_AVR_EMU_HPP_