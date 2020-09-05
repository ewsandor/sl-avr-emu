/**
 * @file sl_avr_emu_tick.hpp
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Tick Logic Header
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_TICK_HPP_
#define _SL_AVR_EMU_TICK_HPP_

#include "sl_avr_emu_types.hpp"

/**
 * @brief Simulates a clock tick for a given emulation
 * 
 * @param emulation            - Pointer to emulation to simulate
 * @return sl_avr_emu_result_e
 */
sl_avr_emu_result_e sl_avr_emu_tick(sl_avr_emu_emulation_s * emulation);

#endif //_SL_AVR_EMU_TICK_HPP_