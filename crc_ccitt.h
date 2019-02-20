/*
    ***********************************************************************
    *   De      :http://www.lammertbies.nl/comm/info/crc-calculation.html * 
    *   Library : lib_crc                                                 *
    *   File    : lib_crc.c                                               *
    *   Author  : Lammert Bies  1999-2008                                 *
    *   E-mail  : info@lammertbies.nl                                     *
    *   Language: ANSI C                                                  *
    ***********************************************************************
*/


#ifndef CRC_CCITT

  #define CRC_CCITT
  #define                 P_CCITT     0x1021

    /* ******************************************************************
    *                                                                   *
    *   unsigned short update_crc_ccitt( unsigned long crc, char c );   *
    *                                                                   *
    *   The function update_crc_ccitt calculates  a  new  CRC-CCITT     *
    *   value  based  on the previous value of the CRC and the next     *
    *   byte of the data to be checked.                                 *
    *                                                                   *
    ******************************************************************* */
  unsigned short update_crc_ccitt( unsigned short crc, char c ) ;

    /* ******************************************************************
    *                                                                   *
    *   static void init_crcccitt_tab( void );                          *
    *                                                                   *
    *   The function init_crcccitt_tab() is used to fill the  array     *
    *   for calculation of the CRC-CCITT with values.                   *
    *                                                                   *
    ******************************************************************* */
  static void init_crcccitt_tab( void ) ;


  int8_t crc_ok(char *data, unsigned long int data_len, unsigned short crc);
  unsigned short crc_calc(char *data, unsigned long int data_len );

#endif
