/**
 * @file sl_avr_emu_timer.h
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Timer/Counter Header
 * @version 0.1
 * @date 2020-09-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_TIMER_H_
#define _SL_AVR_EMU_TIMER_H_

#include "sl_avr_emu_types.h"

#define SL_AVR_EMU_TIMER_0_TCCR0A 0x24
#define SL_AVR_EMU_TIMER_0_TCCR0B 0x25
#define SL_AVR_EMU_TIMER_0_TCNT0  0x26
#define SL_AVR_EMU_TIMER_0_OCR0A  0x27
#define SL_AVR_EMU_TIMER_0_OCR0B  0x28
#define SL_AVR_EMU_TIMER_0_TIMSK0 0x6E
#define SL_AVR_EMU_TIMER_0_TIFR0  0x15

#define SL_AVR_EMU_TIMER_0_TOIE0  0x0
#define SL_AVR_EMU_TIMER_0_OCIE0A 0x1
#define SL_AVR_EMU_TIMER_0_OCIE0B 0x2
#define SL_AVR_EMU_TIMER_0_TOV0   0x0
#define SL_AVR_EMU_TIMER_0_OCF0A  0x1
#define SL_AVR_EMU_TIMER_0_OCF0B  0x2

/**
 * @brief Checks if 8-bit timer is properly configured
 * 
 * @param timer - Timer to check
 * @return true 
 * @return false 
 */
bool sl_avr_emu_timer_8_configured(sl_avr_emu_timer_8_s *timer);

/**
 * @brief Configures timer counter 0 registers
 * 
 * @param memory - memory containing timer 0's register
 * @param timer  - timer to configure
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_configure_timer0(sl_avr_emu_memory_s * memory, sl_avr_emu_timer_8_s *timer);

/**
 * @brief Simulates tick of 8-bit timer/counter
 * 
 * @param timer Timer to simulate
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_timer_8_tick(sl_avr_emu_timer_8_s *timer);

#endif //_SL_AVR_EMU_TIMER_H_