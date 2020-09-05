/**
 * @file sl_avr_emu_bitops.hpp
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Bit Operations Header
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_BITOPS_HPP_
#define _SL_AVR_EMU_BITOPS_HPP_

/**
 * @brief Sets bit at index in mask
 * 
 */
#define SL_AVR_EMU_SET_BIT(mask, index) \
  (mask |= (1 << index))

/**
 * @brief Clears bit at index from mask
 * 
 */
#define SL_AVR_EMU_CLEAR_BIT(mask, index) \
  (mask &= ~(1 << index))

#endif //_SL_AVR_EMU_BITOPS_HPP_