/**
 * @file sl_avr_emu_hex.c
 * @author Ed Sandor (ed@ewsandor.com)
 * @brief Sandor Labs AVR Emulator Hex Loading Logic
 * @version 0.1
 * @date 2020-09-05
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sl_avr_emu_hex.h"

/**
 * @brief Converts a hex string to integer
 * 
 * @param str String representing integer in hex
 * @param n   Maximum number of string characters to parse
 * @return uint32_t 
 */
uint32_t sl_avr_emu_hex_str_to_int(char * str, size_t n)
{
  size_t i;
  uint32_t ret_val = 0;

  if(str != NULL)
  {
    for(i = 0; i < n && 0 != str[i]; i++)
    {
      ret_val <<= 4;
      if(str[i] >= '0' && str[i] <= '9')
      {
        ret_val += str[i] - '0';
      }
      else if(str[i] >= 'a' && str[i] <= 'f')
      {
        ret_val += (str[i] - 'a') + 10;
      }
      else if(str[i] >= 'A' && str[i] <= 'F')
      {
        ret_val += (str[i] - 'A') + 10;
      }
    }
  }

  return ret_val;
}

/**
 * @brief Loads a .hex file into emulation's flash
 * 
 * @param emulation 
 * @param file_path 
 * @return sl_avr_emu_result_e 
 */
sl_avr_emu_result_e sl_avr_emu_load_hex(sl_avr_emu_emulation_s *emulation, char * file_path)
{
  sl_avr_emu_result_e result = SL_AVR_EMU_RESULT_SUCCESS;
  FILE         *fp;
  const size_t  line_buffer_size = 1024;
  char          line[line_buffer_size];
  size_t        line_len;
  size_t        line_i;
  uint32_t      line_num = 0;
  uint32_t      i;

  uint8_t                       num_bytes  = 0;
  uint16_t                      address    = 0;
  uint8_t                       operation  = 0;
  /* Max data is 16*16 bytes (two hex chars in file)*/
  sl_avr_emu_byte_t             checksum, checksum_answer;
  sl_avr_emu_byte_t             data[16*16];
  sl_avr_emu_word_t             flash_word = 0;
  sl_avr_emu_extended_address_t flash_address, flash_address_temp = 0;
  sl_avr_emu_extended_address_t flash_address_prefix = 0;

  if(file_path != NULL)
  {
    printf("Loading flash from .hex file: %s\n", file_path);
    fp = fopen(file_path, "r");
    if(fp != NULL)
    {
      while( (SL_AVR_EMU_RESULT_SUCCESS == result) && fgets(line, line_buffer_size, fp) )
      {
        line_num++;
        line_len = strlen(line);
        /* Each line should start with ':' */
        if(line_len >= 9 && ':' == line[0])
        {
          checksum = 0;
          /* First two nibbles are number of bytes */
          num_bytes = sl_avr_emu_hex_str_to_int(&line[1], 2);
          checksum += num_bytes; 
          /* Next 4 nibbles are the byte address */
          address   = sl_avr_emu_hex_str_to_int(&line[3], 4);
          checksum += (address & 0xFF); 
          checksum += (address >> 8); 
          /* Next 2 nibbles are the operation */
          operation = sl_avr_emu_hex_str_to_int(&line[7], 2);
          checksum += operation; 

          /* If line is long enough for prefix + number of nibbles + checksum */
          if(line_len >= (9 + 2*num_bytes + 2))
          {
            for(i = 0; i < num_bytes; i++)
            {
              data[i] = sl_avr_emu_hex_str_to_int(&line[9+2*i], 2);
              checksum += data[i];
            }            
            checksum = ((~checksum) & 0xFF) + 1;
            checksum_answer = sl_avr_emu_hex_str_to_int(&line[9+2*num_bytes], 2);
            if(checksum == checksum_answer)
            {
              if(0 == operation)
              {
                /* Flash address (word address) from hex file address (byte) */
                flash_address = ((flash_address_prefix << 16) | address)/2;
                for(i = 0; i < num_bytes && (SL_AVR_EMU_RESULT_SUCCESS == result); i++)
                {
                  flash_address_temp = flash_address + (i/2);
                  if(SL_AVR_EMU_FLASH_ADDRESS_VALID(flash_address_temp))
                  {
                    if( 0 == (i % 2) )
                    {
                      emulation->memory.flash[flash_address_temp] = data[i];
                    }
                    else 
                    {
                      emulation->memory.flash[flash_address_temp] |= (data[i] << 8);
                    }
                  }
                  else 
                  {
                    result = SL_AVR_EMU_RESULT_INVALID_FLASH_ADDRESS;
                  }
                }
              }
              else if(1 == operation)
              {
                /* end of file */
                break;
              }
              else if(4 == operation)
              {
                flash_address_prefix = (data[0] << 8) | data[1];
              }
              else
              {
                fprintf(stderr, "Unrecognized operation %u\n", operation);
                fprintf(stderr, "0x%02x, 0x%04x, 0x%02x\n", num_bytes, address, operation);
                fprintf(stderr, "Line %u: %s\n", line_num, line);
              }
            }
            else 
            {
              fprintf(stderr, "Checksum failure: checksum %x, answer %x\n", checksum, checksum_answer);
              fprintf(stderr, "Line %u: %s\n", line_num, line);
              result = SL_AVR_EMU_RESULT_INVALID_FILE_FORMAT;
              break;
            }
          }
          else 
          {
            fprintf(stderr, "Malformed line, aborting\n");
            fprintf(stderr, "Line %u: %s\n", line_num, line);
            result = SL_AVR_EMU_RESULT_INVALID_FILE_FORMAT;
            break;
          }
        }
      }

      fclose(fp);
    }
    else 
    {
      result = SL_AVR_EMU_RESULT_INVALID_FILE_PATH;
    }
  }
  else 
  {
    result = SL_AVR_EMU_RESULT_INVALID_FILE_PATH;
  }

  return result;
}