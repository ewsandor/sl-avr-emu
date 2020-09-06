/**
 * @file sl_avr_emu_bitops.h
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Bit Operations Header
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_BITOPS_H_
#define _SL_AVR_EMU_BITOPS_H_

#include "sl_avr_emu_types.h"

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
/**
 * @brief Clears bit at index from mask
 * 
 */
#define SL_AVR_EMU_CHECK_BIT(mask, index) \
  ((mask & (1 << index))?1:0)


/**
 * @brief Sets bit at index in SREG
 * 
 */
#define SL_AVR_EMU_SET_SREG_BIT(emulation, index)   SL_AVR_EMU_SET_BIT((emulation).memory.data[SL_AVR_EMU_SREG_ADDRESS], index)
/**
 * @brief Clears bit at index in SREG
 * 
 */
#define SL_AVR_EMU_CLEAR_SREG_BIT(emulation, index) SL_AVR_EMU_CLEAR_BIT((emulation).memory.data[SL_AVR_EMU_SREG_ADDRESS], index)
/**
 * @brief Check bit at index in SREG
 * 
 */
#define SL_AVR_EMU_CHECK_SREG_BIT(emulation, index) SL_AVR_EMU_CHECK_BIT((emulation).memory.data[SL_AVR_EMU_SREG_ADDRESS], index)


#endif //_SL_AVR_EMU_BITOPS_H_