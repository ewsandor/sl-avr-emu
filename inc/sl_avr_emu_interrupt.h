/**
 * @file sl_avr_emu_interrupt.h
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Interrupt Header
 * @version 0.1
 * @date 2020-09-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_INTERRUPT_H_
#define _SL_AVR_EMU_INTERRUPT_H_

#include "sl_avr_emu_types.h"

/**
 * @brief Handle any pending interrupts
 * 
 * @param emulation 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_interrupt_handling(sl_avr_emu_emulation_s *emulation);

#endif //_SL_AVR_EMU_INTERRUPT_H_