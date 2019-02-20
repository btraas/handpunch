#include <stdint.h>
#include "crc_ccitt.h"

static int              crc_tabccitt_init       = 0;
static unsigned short   crc_tabccitt[256];

    /*******************************************************************\
    *                                                                   *
    *   unsigned short update_crc_ccitt( unsigned long crc, char c );   *
    *                                                                   *
    *   The function update_crc_ccitt calculates  a  new  CRC-CCITT     *
    *   value  based  on the previous value of the CRC and the next     *
    *   byte of the data to be checked.                                 *
    *                                                                   *
    \*******************************************************************/

unsigned short update_crc_ccitt( unsigned short crc, char c ) {

    unsigned short tmp, short_c;

    short_c  = 0x00ff & (unsigned short) c;

    if ( ! crc_tabccitt_init ) init_crcccitt_tab();

    tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ crc_tabccitt[tmp];

    return crc;

}  /* update_crc_ccitt */


    /*******************************************************************\
    *                                                                   *
    *   static void init_crcccitt_tab( void );                          *
    *                                                                   *
    *   The function init_crcccitt_tab() is used to fill the  array     *
    *   for calculation of the CRC-CCITT with values.                   *
    *                                                                   *
    \*******************************************************************/
            
static void init_crcccitt_tab( void ) {

    int i, j; 
    unsigned short crc, c;

    for (i=0; i<256; i++) { 
    
        crc = 0; 
        c   = ((unsigned short) i) << 8;

        for (j=0; j<8; j++) {
        
            if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ P_CCITT;
            else                      crc =   crc << 1;
    
            c = c << 1;
        }

        crc_tabccitt[i] = crc;
    }
        
    crc_tabccitt_init = 1 ;

}  /* init_crcccitt_tab */


unsigned short crc_calc(char *data, unsigned long int data_len )
{
  unsigned short crc = 0;
  unsigned long int i;
  for (i=0;i<data_len;i++){
    crc = update_crc_ccitt( crc,*(data+i));
  }
  return crc;
}

int8_t crc_ok(char *data, unsigned long int data_len, unsigned short crc)
{
  if ( crc == crc_calc(data,data_len))
    return 1;

  return 0;
}
