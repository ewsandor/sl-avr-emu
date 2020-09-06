/**
 * @file sl_avr_emu_types.h
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Types Header
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _SL_AVR_EMU_TYPES_HPP_
#define _SL_AVR_EMU_TYPES_HPP_

#include <stdint.h>

/**
 * @brief Common result enum for emulation operations
 * 
 */
typedef enum
{
  SL_AVR_EMU_RESULT_SUCCESS               = 0,
  SL_AVR_EMU_RESULT_FAILURE               = 1,
  SL_AVR_EMU_RESULT_INVALID_DATA_ADDRESS  = 2,
  SL_AVR_EMU_RESULT_INVALID_FLASH_ADDRESS = 3,
  SL_AVR_EMU_RESULT_INVALID_PC            = 4,
  SL_AVR_EMU_RESULT_INVALID_OPCODE        = 5,
  SL_AVR_EMU_RESULT_STACK_OVERFLOW        = 6,
  SL_AVR_EMU_RESULT_STACK_UNDERFLOW       = 7,
  SL_AVR_EMU_RESULT_INVALID_FILE_PATH     = 8,
  SL_AVR_EMU_RESULT_INVALID_FILE_FORMAT   = 9,

} sl_avr_emu_result_e;

/**
 * @brief Emulation byte type
 * 
 */
typedef uint8_t sl_avr_emu_byte_t;

/**
 * @brief Emulation word type
 * 
 */
typedef uint16_t sl_avr_emu_word_t;

/**
 * @brief Emulation standard address type
 * 
 */
typedef uint16_t sl_avr_emu_address_t;
/**
 * @brief Emulation extended address type
 * 
 */
typedef uint32_t sl_avr_emu_extended_address_t;

/**
 * @brief Emulation some number of bytes
 * 
 */
typedef uint32_t sl_avr_emu_num_bytes;

/**
 * @brief Data memory space size
 * 
 */
#define SL_AVR_EMU_DATA_SIZE  (1 << 16)
/**
 * @brief Flash memory space size
 * 
 */
#define SL_AVR_EMU_FLASH_SIZE (1 << 16)

/**
 * @brief Checks if a given Data Address is valid
 * 
 */
#define SL_AVR_EMU_DATA_ADDRESS_VALID(address) \
  ((address) < SL_AVR_EMU_DATA_SIZE)
/**
 * @brief Checks if a given Flash Address is valid
 * 
 */
#define SL_AVR_EMU_FLASH_ADDRESS_VALID(address) \
  ((address) < SL_AVR_EMU_FLASH_SIZE)
/**
 * @brief Checks if a given PC address is valid
 * 
 */
#define SL_AVR_EMU_PC_ADDRESS_VALID(address) \
  ((address) < SL_AVR_EMU_FLASH_SIZE)

/**
 * @brief Determine if extended data addressing is in use
 * 
 */
#define SL_AVR_EMU_EXTENDED_DATA_ADDRESS (SL_AVR_EMU_DATA_SIZE >= (1 << 16))
/**
 * @brief Determine if extended flash addressing is in use
 * 
 */
#define SL_AVR_EMU_EXTENDED_FLASH_ADDRESS (SL_AVR_EMU_FLASH_SIZE >= (1 << 16))
/**
 * @brief Determine if extended PC addressing is in use
 * 
 */
#define SL_AVR_EMU_EXTENDED_PC_ADDRESS (SL_AVR_EMU_FLASH_SIZE >= (1 << 16))

/**
 * @brief SREG Bit Assignments
 * 
 */
typedef enum
{
  SL_AVR_EMU_SREG_CARRY_FLAG      = 0,
  SL_AVR_EMU_SREG_ZERO_FLAG       = 1,
  SL_AVR_EMU_SREG_NEGATIVE_FLAG   = 2,
  SL_AVR_EMU_SREG_OVERFLOW_FLAG   = 3,
  SL_AVR_EMU_SREG_SIGN_FLAG       = 4,
  SL_AVR_EMU_SREG_HALF_CARRY_FLAG = 5,
  SL_AVR_EMU_SREG_BIT_COPY        = 6,
  SL_AVR_EMU_SREG_INTERRUPT_FLAG  = 7,
} sl_avr_emu_sreg_bit_e;

/**
 * @brief IO-Space-Address Data-Space-Address offset (i.e IO Address 0x00 == Data Address 0x20)
 * 
 */
 #define SL_AVR_EMU_IO_ADDRESS_SPACE_OFFSET 0x20
/**
* @brief Special-Register Data-Space Address
* 
*/
#define SL_AVR_EMU_SREG_ADDRESS 0x5F
/**
 * @brief Stack-Pointer Data-Space High Address
 * 
 */
#define SL_AVR_EMU_SPH_ADDRESS 0x5E
/**
 * @brief Stack-Pointer Data-Space Low Address
 * 
 */
#define SL_AVR_EMU_SPL_ADDRESS 0x5D
/**
 * @brief Extended indirect jump/call address
 * 
 */
#define SL_AVR_EMU_EIND_ADDRESS 0x5C
/**
 * @brief RAM Prepend to X pointer address
 * 
 */
#define SL_AVR_EMU_RAMPX_ADDRESS 0x59
/**
 * @brief RAM Prepend to Y pointer address
 * 
 */
#define SL_AVR_EMU_RAMPY_ADDRESS 0x5A
/**
 * @brief RAM Prepend to Z pointer address
 * 
 */
#define SL_AVR_EMU_RAMPZ_ADDRESS 0x5B
/**
 * @brief RAM Prepend to D pointer address
 * 
 */
#define SL_AVR_EMU_RAMPD_ADDRESS 0x58

typedef struct
{
  /* Program Counter */
  sl_avr_emu_extended_address_t pc;

  /* Data Memory Space */
  sl_avr_emu_byte_t data [SL_AVR_EMU_DATA_SIZE];
  /* Flash Memory Space */
  sl_avr_emu_word_t flash[SL_AVR_EMU_FLASH_SIZE];

} sl_avr_emu_memory_s;

/**
 * @brief Number of cycles related to an operation
 * 
 */
typedef uint8_t sl_avr_emu_op_count_t;

/**
 * @brief Main structure for an emulation
 * 
 */
typedef struct
{
  /* Emulation Memory */
  sl_avr_emu_memory_s   memory;

  /* Cycles remaining for current operation.  
     Process as new op on tick if 0, else decrements 1 on tick */
  sl_avr_emu_op_count_t op_cycles_remaining;
  
} sl_avr_emu_emulation_s;

#endif //_SL_AVR_EMU_TYPES_HPP_