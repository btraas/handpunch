/*
################################################################################
handpunch try to implement some basic functionality for HandKey and HandPunch
 devices agree to the protocol specification binded by the manufacturer.
Copyright (C) 2011 "liandrosg @ gmail.com"

This file is part of handpunch.

handpunch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

handpunch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with handpunch.  If not, see <http://www.gnu.org/licenses/>.
################################################################################
*/

//TRUE & FALSE
#ifndef TRUE_FALSE
   #define TRUE  1
   #define FALSE 0
#endif

#ifndef HGU
 #define HGU

 //Memory bank size  in bytes
 #define HGU_MEM_BANK_SIZE         4096

 //Communication protocol. Fixed fields in packet
 #define HGU_PACKET_PRE_POST_FRAME 0xFF
 #define HGU_PACKET_START_FRAME    0x0A
 //Communication protocol info.
 #define HGU_MAX_DATA_SIZE         4096 //Max. data size field in packet (by specification in manual)
 //HGU models
 #define E_HK                      0x1
 #define E_HP                      0x2
 #define F_HK                      0x4
 #define F_HP                      0x8
 #define HP4K                      0x16

 //Output format
 #define HGU_OUT_F_HUMAN           1
 #define HGU_OUT_F_PARSEABLE       2
 
 //REFERENCE 2 bytes as int16
 #define TO_INT16(a,b) (0x0000|a<<8)|b

 //Packet structure
 struct HGU_packet{
  char     pre_frame  ; //:FF . Specification say: are'nt guaranteed to be present
  char     start_frame; //Always is HGU_PACKET_START_FRAME
  char     address    ; //HGU unit address (you can link HGU units)
  char     op_type    ; //0-6 low bits:Command type(for query), or response type
                       //   7 bit: size of length field, 0=1byte,1=2bytes 
  uint16_t data_length; //Length of data in packet
  char     data[HGU_MAX_DATA_SIZE] ; // data
  uint16_t crc        ; //CCITT crc, on address, op_type, data_length and data
  char     post_frame ; //:FF . Specification say: are'nt guaranteed to be present
 };
 #define ST_SIZE_HGU_packet 4105

 //Operation type structure to get info about it
 struct opType{
   char    type;      //Op. type id
   char    sup_models;//Supported models(up to 8 models using bits of char type)
   int16_t length;    //Data length of command or response
   int16_t comp_max_length;//Max. data length of complementary op. type :
                           //if is a command, max data length of complementary response
 };

 //HereIsNextDatalog/SendPreviousDatalog response structure
 struct dataLog{
   unsigned char address;
   unsigned char timestamp[6];
   unsigned char format;
   unsigned char data1[5];
   unsigned char data2[5];
 };
 
 //HereIstime structure
 struct HereIsTime{
   unsigned char sec;
   unsigned char min;
   unsigned char hour;
   unsigned char day;
   unsigned char month;
   unsigned char year;
 };
 #define ST_SIZE_HereIsTime 6

 //HereIsUserRecord structure
 #define SIZE_UR_ID 5
 #define SIZE_UR_TEMPLATE 9
 struct HereIsUserRecord{ 
   unsigned char urid[SIZE_UR_ID];//User id
   unsigned char tpl[SIZE_UR_TEMPLATE];        //Template
   unsigned char authority;
   unsigned char timezone;
 };
 #define HereIsUserRecord_ST_SIZE SIZE_UR_ID+SIZE_UR_TEMPLATE+1+1
 #define HereIsUserRecord_DUMP_SIZE SIZE_UR_ID*2 +       /*2 record_id (*2 because hexa rep.)*/\
                                    1 +                  /*space between fields */\
                                    SIZE_UR_TEMPLATE*2 + /*fingerprint (*2 because hexa rep.)*/\
                                    1   +                /*space between fields */\
                                    1*2 +                /*authority */\
                                    1   +                /*space between fields */\
                                    1*2                  /*timezone (*2 because hexa rep.) */

 //HereIsReaderInfo structure
 struct HereIsReaderInfo{ 
   unsigned char model;
   unsigned char memory;
   unsigned char promDate[20];
   unsigned char modelName[17];
   unsigned char serialNumber[4];
   unsigned char serialNumberPrefix[1];
   uint16_t UCap;   // User capacity
   uint16_t TCap;   // Transaction capacity
   uint16_t UNum;   // Current number of users enrolled
   uint16_t TNum;   // Current number of transactions
   unsigned char Config[14]; // Displayed on LCD line 1 at startup
   unsigned char ME1;        // Modem/Ethernet adaptor flag. 
   unsigned char altProm;    // Alternate PROM style
   unsigned char reserved[34];
 };
 #define ST_SIZE_HereIsReaderInfo 102



 //Is a valid operation type value for command packet?
 int8_t valid_command_opType (char op_type);

 //Is a valid operation type value for response packet?
 int8_t valid_response_opType (char op_type);

 //Get max. response size by opType command
 int16_t HGU_packet_get_max_resp_size(char op_type_cmd);

 //Get info about especific opType command or response
 struct opType get_opType_info(char op_type, char type);

 //HGU_packet initializator
 int8_t HGU_packet_init(struct HGU_packet *p);

 //Set operation type in packet structure
 int8_t HGU_packet_set_opType(struct HGU_packet *p,char op_type);

 //Set data in HGU packet structure
 int8_t HGU_packet_set_data(struct HGU_packet *p,char *data, uint16_t data_size );

 //Set unit number in packet structure
 int8_t HGU_packet_set_unitNumber(struct HGU_packet *p,uint8_t un);

 //Convert a HGU_packet_structure, to binary data, ready to send 
 char *HGU_packetSt_to_bin(struct HGU_packet *p, uint16_t *bin_data_length);

 //Copy HereIsReaderInfo data response to appropiate structre
 struct HereIsReaderInfo *HGU_HereIsReaderInfo_Data_to_st(struct HGU_packet *resp);

 //check val of char arg.  for [0-9,A-F]
 uint8_t is_hex_char(char c);

 //copy chars en double-byte format from char*src to simple byte char*dst with dst_size long. 
 int8_t hexchar_to_char_cpy (char *src, char* dst, int16_t dst_size);

 //Parse bin protocol data, and save its in HGU_packet structure
 int8_t HGU_bin_to_HereIsUserRecord_st(char *bin_data, struct HereIsUserRecord *u);
	
 //Parse bin protocol data, and save its in HGU_packet structure
 int8_t HGU_packetBin_to_st(char *bin_data, int16_t bin_data_length, struct HGU_packet *p);

 //Get how many memory banks have, if you know reader info
 int8_t HGU_how_many_mem_banks(struct HereIsReaderInfo *reader_info);

 //Get user record size, model based
 int8_t HGU_get_record_size_by_model(int8_t model);

 //Return substring from string pointer
 char *substr(char *str, int size);

 //Translate codified baud rate in real value
 uint16_t transl_baud(char index,char model);

 //Translate codified date format to human value
 char *transl_datef(char index);

 //Translate codified lang to human value
 char *transl_lang(char index);

 //Translate codified model number to human value
 char *transl_modeln(char index);

 //Translate codified memory to human value
 char *transl_memory(char index);

 //Translate serial number prefix codified to human value
 char *transl_serialn_prefix(char index);

 //Translate adaptor codified to human value
 char *transl_adaptor(char index);

 //Translate style PROM codified to human value
 char *transl_stylePROM(char index);

 //Display HereIsReaderInfo response in comprensible mode
 int8_t view_HereIsReaderInfo(char *data,short int format);

 //Display HereIsUserRecord in comprensible mode
 int8_t view_HereIsUserRecord(struct HereIsUserRecord *ur, short int format);

 //Display HereIsNextDatalog in comprensible mode
 int8_t view_HereIsNextDatalog(char *data,short int format);

 //Display HereIsExtendedSetup response in comprensible mode
 int8_t view_HereIsExtendedSetup(char *data,short int format);

 //Get "format" field from SendNextDatalog/SendPreviousDatalog binary response
 char get_datalog_format_from_bin(struct dataLog *dl);

 //Copy bin data from HereIsNextDatalog response, to dataLog structure
 struct dataLog dataLog_Bin_to_st(char *data);

 //Convert "unsigned long int" user record id, to "char[]" hgu compatible format
 int8_t i2urid(unsigned long int iurid, unsigned char *urid );

 //Translate HereIsMyTime time format to struct tm
 struct tm get_tm_from_HereIsMyTime( char *data);

 //Check if DLOG_RDY is set in a HereIsStatus response(0x30)
 //The DLOG_RDY is set whenever de datalog buffer is not empty (you can 
 // retrieving transactions).
 int8_t HGU_pagcket_is_DLOG_RDY_set(char *data);

 //Display HereIsMyTime response in comprensible mode
 int8_t view_HereIsMyTime(char *data,short int format);

 //Display HereIsStatus_response in comprensible mode
 int8_t view_HereIsStatus_response(char *data,short int format);

 //Display different responses  in comprensible mode
 int8_t HGU_packet_show_response(struct HGU_packet *p, short int format, 
                                 struct HereIsReaderInfo *reader_info );

 //Returns a structure set to the value of a time_t arg, or system local time
 // if it is NULL
 struct HereIsTime *HGU_get_time_from_time_t(time_t timer);

//Returns a structure set to the value of time string arg. 
struct HereIsTime *HGU_get_time_from_st(char *st_time);

#endif
