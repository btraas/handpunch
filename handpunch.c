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
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <time.h>

#include "hgu.h"

//TRUE & FALSE
#ifndef TRUE_FALSE
  #define TRUE  1
  #define FALSE 0
#endif


#define HGU_MAX_UNIT_NUMBER       20
#define CONN_READ_TIMETOUT 	  5
#define SEND_BUF_SIZE 		  1024
#define DEFAULT_SERIAL_BAUD       B9600
#define DEFAULT_TCP_PORT 	  3001 


//Internal behavior
#define MODE_TRY_WAKEUP      1  ///Wake up unit using 0x44 command before to try any operation
#define MODE_CHECK_CRC       2  //Disable crc check on responses
#define MODE_SCANNING        4  //Disable some checks

//How many tries to Wake up unit using 0x44 command 
#define WAKEUP_TRIES         3    // Max. tries

//Actions to execute
#define QUERY_TIME           1
#define QUERY_INFO           2
#define QUERY_EXT_SETUP      3
#define QUERY_STATUS         4
#define QUERY_ENROLLED_USERS 5
#define UNIT_N_SCAN          6
#define DWN_ALL_TRAN         7
#define DWN_ID_VERIF_TRAN    8
#define DWN_MEM_BANKS        9
#define CONF_TIME            10
#define QUERY_USER_REC       11
#define RESTORE_MEM_BANKS    12
#define RESTORE_ENROLL_USERS 13

//Connection mode
#define CONN_MODE_IP         1
#define CONN_MODE_SERIAL     2


//Get ip from name
 char *get_ip(char *host);
//Usage
 void usage(char *my);
//TCP- Create socket
 int create_tcp_socket();
//TCP- Recev data
 char *recev_data(int conn, int8_t conn_type, uint16_t *resp_len, uint16_t max_resp_len );
 //TCP- Send data
 int16_t send_data(int conn, int8_t conn_type , uint16_t data_len, char *data );
 //Open tcp connection
 int open_socket_conn(char *hgu);
 //Open tcp connection
 int open_serial_conn(char *hgu);
 //Send data to unit, then parse and save response 
 int8_t send_query_and_get_resp(int conn, int8_t conn_type,
                                            struct HGU_packet *query, 
                                            struct HGU_packet *resp, 
                                            int    modes);
 //Translate int baud rate into speed_t 
 speed_t ibaud_to_speedt(unsigned int baud);

 //Translate speed_t baud rate into int
 unsigned int speedt_to_ibaud(speed_t speed);

 //Print a message "msg" , and "data" of size "size" in hex representation
 // on file descriptor "f"
 void hexdump_dbg(char *msg, char *data, int size ,FILE* f );


int8_t verb=FALSE;


int main(int argc, char **argv)
{
  int conn=0,i=0,modes=MODE_TRY_WAKEUP | MODE_CHECK_CRC;
  int8_t conn_type=0;
  char *hgu=NULL;
  short int opt,action=0,unumber=0,format=0;
  struct HGU_packet query, resp, prev_resp;
  struct HereIsTime *hit_time=NULL;
  struct HereIsReaderInfo *reader_info;
  struct HereIsUserRecord user_record;
  unsigned long int iurid=0;
  unsigned char char_urid[SIZE_UR_ID];
  int16_t bank=0;
  char *indata=NULL;

  //Options
  while((opt = getopt(argc, argv, "hvsbq:u:n:d:f:c:i:r:m:")) != -1) {
    switch(opt) {
      case 'q': //Query unit ...
        switch((char)*optarg){
          case 't': //unit clock (HereIsMyTime response)
            action=QUERY_TIME;
            break;
          case 'i': //unit info (HereIsReaderInfo response)
            action=QUERY_INFO;
            break;
          case 'e': //unit extended setup (HereIsExtendedSetup response)
            action=QUERY_EXT_SETUP;
            break;
          case 's': //unit status (HereIsStatus)
            action=QUERY_STATUS;
            break;
          case 'u': //query user record (SendUserRecord)
            action=QUERY_USER_REC;
            break;
          case 'l': //query list of enrolled users
            action=QUERY_ENROLLED_USERS;
            break;
          default: //undefined
            fprintf(stderr,"ERROR: \"%s\", undefined query.\n",optarg);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        break;

      case 'd': //Download data ...
        switch((char)*optarg){
          case 'i': //download id verified trans.(users attendance trans.)
            action=DWN_ID_VERIF_TRAN;
            break;
          case 'a': //download all transactinons
            action=DWN_ALL_TRAN;
            break;
          case 'b': //backup memory bancks
            action=DWN_MEM_BANKS;
            break;
          default: //undefined
            fprintf(stderr,"ERROR: \"%s\", undefined download action.\n",optarg);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        break;

      case 'f': //Output format ...
        switch((char)*optarg){
          case 'h': //Human-readable full data
            format=HGU_OUT_F_HUMAN;
            break;
          case 'p': //Parseable util data
            format=HGU_OUT_F_PARSEABLE;
            break;
          default: //undefined
            fprintf(stderr,"ERROR: \"%s\", undefined output format.\n",optarg);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        break;


      case 'c': //Configuration ...
        switch((char)*optarg){
          case 't': //Configure time
            action=CONF_TIME;
            break;
          default: //undefined
            fprintf(stderr,"ERROR: \"%s\", undefined configure option.\n",optarg);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        break;

      case 'r': //Restore unit data ...
        switch((char)*optarg){
          case 'm': //Restore memory banks
            action=RESTORE_MEM_BANKS;
            break;
          case 'l': //Restore enrolled users
            action=RESTORE_ENROLL_USERS;
            break;
          default: //undefined
            fprintf(stderr,"ERROR: \"%s\", undefined restore option.\n",optarg);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        break;

      case 'm': //Change behavior
        switch((char)*optarg){
          case 'c': //Disable CRC checking. 
            modes=modes ^ MODE_CHECK_CRC;
            break;
          case 'w': //Disable wakeup behavior before any operation
            modes=modes ^ MODE_TRY_WAKEUP;
            break;
          default: //undefined
            fprintf(stderr,"ERROR: \"%s\", undefined option.\n",optarg);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        break;

      case 'u': //Hand Geometrical Unit IP or serial port to connect
        if ( '/' != (char)*optarg) {
          conn_type=CONN_MODE_IP;
          hgu=optarg;
        } else {
          conn_type=CONN_MODE_SERIAL;
          hgu=optarg;
        }
        break;
      case 'n': //Hand Geometrical Unit number
        unumber=atoi(optarg);
        break;
      case 'i': //User record ID 
        iurid=atol(optarg);
        break;
      case 'v': //Verbosing
        verb=TRUE;
        break;
      case 's': //scan for unit number 
        action=UNIT_N_SCAN;
        modes= modes & MODE_SCANNING;
        break;
      case 'h': //Help
        usage(argv[0]);
        exit(EXIT_SUCCESS);
        break;

      case '?':
        if (optopt == 'q' || optopt == 'u' || optopt == 'n' || optopt == 'd' ||
            optopt == 'f' || optopt == 'c' || optopt == 'i' || optopt == 'r' ||
            optopt == 'm' )
          fprintf (stderr, "ERROR: -%c option: argument required.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "ERROR: `-%c', unknown option.\n", optopt);
        else
          fprintf (stderr,"ERROR: `\\x%x' unknown character option.\n", optopt);
        usage(argv[0]);
        exit(EXIT_FAILURE);
      break;
      default: // '?' 
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  //Action?
  if (action==0){
    fprintf(stderr,"ERROR: Action required.\n"); 
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  //HGU ip unit? (connection mode: ip only)
  if (NULL==hgu){
    fprintf(stderr,"ERROR: Clock IP unit, not specified (-u).\n"); 
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }


  //Connect by Socket?
  if (hgu != NULL && conn_type == CONN_MODE_IP ){
    if (!(conn=open_socket_conn(hgu))) exit(EXIT_FAILURE);
  //Conect by serial port
  } else if (hgu != NULL && conn_type == CONN_MODE_SERIAL ){
    if (!(conn=open_serial_conn(hgu))) exit(EXIT_FAILURE);
  }

  //Initialize fields common to all queries in packet
  if(!HGU_packet_init(&query)) 
    exit(EXIT_FAILURE);
  //Set unit number in packet (required to dialog)
  if(!HGU_packet_set_unitNumber(&query,unumber)) 
    exit(EXIT_FAILURE); 


  //Before to operate, take the unit out of Idle mode. 
  //REQUIRED on units with firmware compiled after 6/5/00
  i=0; 
  if (action != UNIT_N_SCAN && modes & MODE_TRY_WAKEUP){
    while (i < WAKEUP_TRIES ){
      if (verb) fprintf(stderr,"DEBUG:Trying unit wake up test %d.\n",i+1);
      if(!HGU_packet_set_opType(&query,0x44)) exit(EXIT_FAILURE); // SendStatusCRC command
      if(send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) break;
      i++;
    }
    if (i == WAKEUP_TRIES ){
      fprintf(stderr,"ERROR: No response received in wakeup test.\n");
      exit(EXIT_FAILURE); 
    } else if ( verb && modes & MODE_TRY_WAKEUP )
        fprintf(stderr,"DEBUG:Wake up OK.\n");
  }
  
  //Accion?
  switch (action){
    //--------------------------------------------------------------------------
    case QUERY_TIME: //Query time and date. IDcmd:0x61 desc:SendTimeAndDate
      if(!HGU_packet_set_opType(&query,0x61)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!HGU_packet_show_response(&resp,format,NULL)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case QUERY_INFO: //Query unit conf. and state. IDcmd:0x73 desc:SendStatusCRC
      if(!HGU_packet_set_opType(&query,0x73)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!HGU_packet_show_response(&resp,format,NULL)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case QUERY_EXT_SETUP: //Query extended setup. IDcmd:0x2C desc:SendExtendedSetup

      if(!HGU_packet_set_opType(&query,0x2C)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!HGU_packet_show_response(&resp,format,NULL)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case QUERY_STATUS: //Query Status IDcmd:0x44 desc:SendStatusCRC
      if(!HGU_packet_set_opType(&query,0x44)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!HGU_packet_show_response(&resp,format,NULL)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case DWN_MEM_BANKS: //Backup memory banks.IDcmd:0x48 desc:SendDataBank
    case QUERY_ENROLLED_USERS: //show users en dump of memory banks.IDcmd:0x48 desc:SendDataBank
      //Get memory capacity
      if(!HGU_packet_set_opType(&query,0x73)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!(reader_info=HGU_HereIsReaderInfo_Data_to_st(&resp))) exit(EXIT_FAILURE);

      //Enter in idle mode
      if(!HGU_packet_set_opType(&query,0x45)) exit(EXIT_FAILURE); // IDcmd:0x45 desc:EnterIdleMode
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);

      //Set query type to 0x48
      if(!HGU_packet_set_opType(&query,0x48)) exit(EXIT_FAILURE); // IDcmd:0x48 desc:SendDataBank
 
      //Query memory banks
      for (bank=0; bank<HGU_how_many_mem_banks(reader_info); bank++){
	//Set bank to query
        if(!HGU_packet_set_data(&query,(char*)&bank,2)) {
          //CRITICAL: take it out of idle mode
          if(!HGU_packet_set_opType(&query,0x31)) exit(EXIT_FAILURE); // IDcmd:0x31 desc:Resume
          exit(EXIT_FAILURE);
        }

        //Query bank
        if (verb) fprintf(stderr,"DEBUG:Consulting bank %hd/%hd ...\n",bank,HGU_how_many_mem_banks(reader_info));
        if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) { 
          //CRITICAL: take it out of idle mode
          if(!HGU_packet_set_opType(&query,0x31)) exit(EXIT_FAILURE); // IDcmd:0x31 desc:Resume
          if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
          exit(EXIT_FAILURE);
        }

        //Retourned data is size correct?
        if (resp.data_length != HGU_MEM_BANK_SIZE){
          fprintf(stderr,
                  "ERROR: Memory dump of bank %hd, was %hd bytes length, expected %hd.",
                  bank,resp.data_length,HGU_MEM_BANK_SIZE);
          //CRITICAL: take it out of idle mode
          if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
          exit(EXIT_FAILURE);
        }

 	//You want view the enrolled users in memory banks?
        if ( action == QUERY_ENROLLED_USERS ){
          if(!HGU_packet_show_response(&resp,format,reader_info)) exit(EXIT_FAILURE);
        } else if ( action == DWN_MEM_BANKS ){ // Or you want the dump 
          fwrite(resp.data,HGU_MEM_BANK_SIZE,1,stdout);
        }
      }//End for each bank

      //CRITICAL: take it out of idle mode
      if(!HGU_packet_set_opType(&query,0x31)) exit(EXIT_FAILURE); // IDcmd:0x31 desc:Resume
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case RESTORE_ENROLL_USERS: //Restore enrolled users from stdin IDcmd:0x37 (HereIsUserRecord)

      if (!(indata=malloc(HereIsUserRecord_DUMP_SIZE +2))){//+2: newline + NULL
        perror("malloc");
        exit(EXIT_FAILURE);
      };

      //For each user resource
      i=0; while ( !feof(stdin) && 
              NULL!=fgets(indata, HereIsUserRecord_DUMP_SIZE +2,stdin)){//+2: newline + fgets
        //Give a HereIsUserRecord structure
        if (!HGU_bin_to_HereIsUserRecord_st(indata,&user_record)){
          fprintf(stderr,"ERROR: Reading line %hd. Data:\"%s\"",i+1,indata);
          exit(EXIT_FAILURE);
        }

        //Add user
        if (verb) hexdump_dbg("DEBUG:Sending user ",(char*)&user_record,SIZE_UR_ID,stderr);
        if(!HGU_packet_set_opType(&query,0x37)) exit(EXIT_FAILURE); // IDcmd:0x37 desc:HereIsUserRecord
        if(!HGU_packet_set_data(&query,(char*)&user_record,HereIsUserRecord_ST_SIZE)) exit(EXIT_FAILURE) ;
        if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);

        i++;//next
      }
      free(indata);
      break;

    //--------------------------------------------------------------------------
    case RESTORE_MEM_BANKS: //Restore memory banks IDcmd: Hex: 0x46 (HereIsBankNumber)
                            //                     IDcmd: Hex: 0x47 (HereIsDataBank)

      //Get memory capacity
      if(!HGU_packet_set_opType(&query,0x73)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!(reader_info=HGU_HereIsReaderInfo_Data_to_st(&resp))) exit(EXIT_FAILURE);

      //Enter in idle mode
      if(!HGU_packet_set_opType(&query,0x45)) exit(EXIT_FAILURE); // IDcmd:0x45 desc:EnterIdleMode
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
 
      bank=0;
      if (!(indata=malloc(HGU_MEM_BANK_SIZE))){
        perror("malloc");
        exit(EXIT_FAILURE);
      };

      //For each data bank
      while ( !feof(stdin) && (i=fread(indata, 1, HGU_MEM_BANK_SIZE, stdin) )> 0 ){
        //Data bank size check
        if (i!= HGU_MEM_BANK_SIZE){
          fprintf(stderr,"ERROR: read %d input bytes. Imcomplete memory bank size (%d) ",i,HGU_MEM_BANK_SIZE);
          exit(EXIT_FAILURE);
        }
        if (verb) fprintf(stderr,"DEBUG:Read %d bytes from input file.\n",i);

        // IDcmd:0x46 desc:HereIsBankNumber
        if (verb) fprintf(stderr,"DEBUG:Sending HereIsBankNumber: %hd.\n",bank);
        if(! ( HGU_packet_set_opType(&query,0x46) && 
               HGU_packet_set_data(&query,(char*)&bank,2) && 
               send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) ){
          //CRITICAL: take it out of idle mode
          if(!HGU_packet_set_opType(&query,0x31)) exit(EXIT_FAILURE); // IDcmd:0x31 desc:Resume
          exit(EXIT_FAILURE);
        }

        // IDcmd:0x47 desc:HereIsDataBank
        if (verb) fprintf(stderr,"DEBUG:Sending HereIsDataBank: %hd.\n",bank);
        if(! ( HGU_packet_set_opType(&query,0x47) && 
               HGU_packet_set_data(&query,indata,HGU_MEM_BANK_SIZE) && 
               send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) ){
          //CRITICAL: take it out of idle mode
          if(!HGU_packet_set_opType(&query,0x31)) exit(EXIT_FAILURE); // IDcmd:0x31 desc:Resume
          exit(EXIT_FAILURE);
        }

        //Next bank
        bank++;
      }

      //take it out of idle mode
      if(!HGU_packet_set_opType(&query,0x31)) exit(EXIT_FAILURE); // IDcmd:0x31 desc:Resume
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      free(indata);
      break;

    //--------------------------------------------------------------------------
    case UNIT_N_SCAN: //automatic query scan. IDcmd:0x44 desc:SendStatusCRC 
      if(!HGU_packet_set_opType(&query,0x44)) exit(EXIT_FAILURE);
      for (i=0;i<HGU_MAX_UNIT_NUMBER;i++) {
        if(!HGU_packet_set_unitNumber(&query,i)) exit(EXIT_FAILURE); //set Unit number
        if(verb) fprintf(stderr,"DEBUG:Trying unit number: %d ...\n",i);
        if (send_query_and_get_resp(conn,conn_type,&query,&resp,modes)){
          if(format==HGU_OUT_F_PARSEABLE) printf("%d\n",i);
          else printf("Unit number found: %d. Now try use me whith:   -n %d\n",i,i);
          exit(EXIT_SUCCESS);
        }
      }
      fprintf(stderr,"Tested 0 to %d unit numbers whith failed results."
                     " Check unit configuration.\n",HGU_MAX_UNIT_NUMBER);
      exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case CONF_TIME: //Configure time. IDcmd:0x41 desc:HereIsTime
      if(!HGU_packet_set_opType(&query,0x41)) exit(EXIT_FAILURE);
      if (getenv("HPTIME")!=NULL){
        if(!(hit_time=HGU_get_time_from_st(getenv("HPTIME")))) exit(EXIT_FAILURE);
      }else{
        if(!(hit_time=HGU_get_time_from_time_t(time(NULL)))) exit(EXIT_FAILURE);
      }
      if(!HGU_packet_set_data(&query,(char*)hit_time,ST_SIZE_HereIsTime)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case QUERY_USER_REC: //Download user record. IDcmd:0x38 desc:SendUserRecord
      if (iurid==0){
        fprintf(stderr,"ERROR:Give me a \"user ID\" to query (-i).\n");
        exit(EXIT_FAILURE);
      }
      if(!HGU_packet_set_opType(&query,0x38)) exit(EXIT_FAILURE);
      i2urid(iurid,char_urid);
      if(!HGU_packet_set_data(&query,(char*)char_urid,SIZE_UR_ID)) exit(EXIT_FAILURE);
      if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
      if(!HGU_packet_show_response(&resp,format,NULL)) exit(EXIT_FAILURE);
      break;

    //--------------------------------------------------------------------------
    case DWN_ALL_TRAN: //download id verified transactions
    case DWN_ID_VERIF_TRAN: //download all transactinons
      //While there is transactions to retrieve 
      i=0; while (TRUE) {
        //There is transactions to retrieve : SendStatusCRC command (0x44) 
        if(!HGU_packet_set_opType(&query,0x44)) exit(EXIT_FAILURE); 
        if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) exit(EXIT_FAILURE);
        if(!HGU_pagcket_is_DLOG_RDY_set(resp.data)) {
          if(verb)fprintf(stderr,"There is no more transactions to retrieve.\n");
          break;
        }
        //Get transactions: SendDatalog command (0x4D) 
        if(!HGU_packet_set_opType(&query,0x4D)) exit(EXIT_FAILURE); 
        if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) {
          //if it fails, we need to see if unit walked one datalog. 
          //Get me the previus datalog.  SendPreviousDatalog command (0x6D)
          if(!HGU_packet_set_opType(&query,0x6D)) exit(EXIT_FAILURE);
          if(!send_query_and_get_resp(conn,conn_type,&query,&resp,modes)) {
            fprintf(stderr,"ERROR:failure to recover a failed download.\n");
            exit(EXIT_FAILURE);
          }
          //We have a previous response?
          if (i > 0){
            if (resp.crc==prev_resp.crc) //unit do not walked one datalog. We will try egain
              continue;
            else TRUE;//we have the lost response, in "resp". We can continue
          } else {
            //We have a error in the first datalog query.
            //Now we can not know if the unit walked one datalog.
            //If walked, then we have the lost response, in "resp". We can continue.
            //If no, then we have the another session last datalog. This will be a
            // duplicate datalog if you process it. Sorry. 
            // Another algorithm is begin with previous datalog, but if the unit is 
            //  recently turned, unfortunately, this query return random data with crc error. Ouch!
            TRUE;
          }
        }//End failed query

        if ( action==DWN_ALL_TRAN ||  //all transactions
            (action==DWN_ID_VERIF_TRAN && 
             get_datalog_format_from_bin((struct dataLog*)resp.data)==0x07 //users attendance
            )
           ){
          if(!HGU_packet_show_response(&resp,format,NULL)) exit(EXIT_FAILURE);
        }
        //save the datalog
        memcpy(&prev_resp,&resp,ST_SIZE_HGU_packet);
        i++;

      }//end while
      break;



  }//end case



  //Ending
  close(conn);
  return 0;
}


//TCP- Send data
int16_t send_data(int conn, int8_t conn_type, uint16_t data_len, char *data )
{
  int sent=0, partial_sent=0 ;
  while(sent < data_len )
  { 
    if (conn_type==CONN_MODE_IP)
      partial_sent = send(conn, data+sent, ((data_len-sent<SEND_BUF_SIZE)?(data_len-sent):(SEND_BUF_SIZE)),0);
    else if (conn_type==CONN_MODE_SERIAL)
      partial_sent = write(conn, data+sent, ((data_len-sent<SEND_BUF_SIZE)?(data_len-sent):(SEND_BUF_SIZE)));
    else return FALSE;
    if(partial_sent == -1){
      perror("Can't send query");
      return FALSE;
    }
    sent += partial_sent;
  }
 return sent;
}

//TCP- Recev data
char  *recev_data(int conn, int8_t conn_type, uint16_t *resp_len, uint16_t max_resp_len )
{
  int16_t pbuffer_len=0;
  char *packet_bin=NULL;
  if (max_resp_len==0) max_resp_len=HGU_MAX_DATA_SIZE;

  if (!(packet_bin=malloc(max_resp_len))){
    perror("Can't alloc memory");
    return FALSE;
  }

  //reading response
  *resp_len=0;
  while(TRUE){

    if (conn_type==CONN_MODE_IP)
      pbuffer_len=recv(conn, packet_bin+*resp_len , max_resp_len - *resp_len, 0);
    else if (conn_type==CONN_MODE_SERIAL){
      pbuffer_len=read(conn, packet_bin+*resp_len , max_resp_len - *resp_len);
    } else return FALSE;

    if(pbuffer_len < 0){
      perror("Read");
      return FALSE;
    } else if (pbuffer_len == 0){
      fprintf(stderr,"ERROR:Response read timeout.\n");
      return FALSE;
    }

    *resp_len=*(resp_len)+pbuffer_len;
    if(*resp_len>=max_resp_len) break;//Do not wait for more data

  }
  if (verb) fprintf(stderr,"DEBUG: Read %d bytes from unit.\n",*resp_len);
  return packet_bin;
}

//TCP- Create socket
int create_tcp_socket()
{
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    perror("Can't create TCP socket");
    exit(EXIT_FAILURE);
  }

  struct timeval timeout;
  // sets up time out option for the bound socket 
  timeout.tv_sec = CONN_READ_TIMETOUT;  
  timeout.tv_usec = 0;

  if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
      perror("setsockopt failed\n");

  if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
      perror("setsockopt failed\n");

  return sock;
}

//Send data to unit, then parse and save response 
int8_t send_query_and_get_resp(int conn, int8_t conn_type ,
                                           struct HGU_packet *query, 
                                           struct HGU_packet *resp, 
                                           int modes)
{
  char *packet_bin;
  uint16_t packet_bin_len=0, i=0;

  //Get binary representation to send
  if (!(packet_bin=HGU_packetSt_to_bin(query, &packet_bin_len)))
    return FALSE;

  //Sending data
  if (verb) fprintf(stderr,"DEBUG:Sending %d bytes of data...\n",packet_bin_len);
  if (verb && FALSE ) { 
      fprintf(stderr,"DEBUG:Sending bytes: 0x"); 
      for (i=0;i<packet_bin_len;i++) fprintf(stderr,"%02X",(unsigned char)*(packet_bin+i));
      fprintf(stderr,"\n"); 
    }

  if (FALSE==send_data(conn, conn_type, packet_bin_len, packet_bin )){
    free(packet_bin);
    return FALSE;
  }
  free(packet_bin);

  //Response
  if (verb) fprintf(stderr,"DEBUG:Waiting response...(max. size:%d)\n",
                           HGU_packet_get_max_resp_size(query->op_type));
  if (!(packet_bin=recev_data(conn, conn_type,
                                  &packet_bin_len, 
                                  HGU_packet_get_max_resp_size(query->op_type)
                                  ))) return FALSE;

  //No response data ?
  if (verb) fprintf(stderr,"DEBUG: Response size:%hd)\n",packet_bin_len);
  if (packet_bin_len==0){
    if (modes & MODE_SCANNING ) {
       fprintf(stderr,"ERROR: Unit not responding data."
                   " Probably you're using an incorrect unit number."
                   " Try using \"-s\" for automatic discover.\n");
    }
    free(packet_bin);
    return FALSE;
  }

  if (verb && FALSE) { 
      fprintf(stderr,"DEBUG:Received bytes: 0x"); 
      for (i=0;i<packet_bin_len;i++) fprintf(stderr,"%02X",(unsigned char)*(packet_bin+i));
      fprintf(stderr,"\n"); 
    }

  //Give me a packet structure
  switch (HGU_packetBin_to_st(packet_bin,packet_bin_len,resp)){
    case TRUE: //all ok
      break;
    case 2:    //CRC error
      if (modes & MODE_CHECK_CRC){
        free(packet_bin);
        return FALSE;
      }
      else fprintf(stderr,"WARNING:continuing with crc error.\n"); 
      break;
    case FALSE://another error
    default:
      free(packet_bin);
      return FALSE;
      break;
  }

  free(packet_bin);
  return TRUE;
}

//Usage
void usage(char *my)
{
  fprintf(stderr,"Usage: %s <action> <options>\n"
                 " <action>:\n"
                 "  -ct  : Configure unit time using system time.\n"
                 "  -da  : Download all transactions in reader.\n"
                 "  -di  : Download user attendance transactions.\n"
                 "  -db  : Download user memory banks data.\n"
                 "  -dm  : Backup unit memory banks(enrolled users) to stdout. \n"
                 "  -mc  : Don't check crc on response (view README:FAQ).\n"
                 "  -mw  : Don't try wakeup before operations (view README:FAQ).\n"
                 "  -qe  : Query reader extended setup.\n"
                 "  -qi  : Query reader info.\n"
                 "  -qs  : Query reader status.\n"
                 "  -qt  : Query reader time.\n"
                 "  -qu  : Query user record.\n"
                 "  -ql  : Query list of enrolled users.\n"
                 "  -rl  : Restore enrolled users from stdin (get users with -ql).\n"
                 "  -rm  : Restore unit memory banks(enrolled users) from stdin .\n"
                 "  -s   : Scan unit number in reader.\n"
                 "\n"
                 " <options>:\n"
                 "  -f[h,p] : Output format:h=Human, p=Parseable. Default:h.\n"
                 "  -n N    : Use N as number unit reader. Default 0.\n"
                 "  -u UNIT : Especify UNIT as :\n"
                 "     host[:port]         : Ip or host name and opt. port(Default %d).\n"
                 "     /dev/ttyport[:baud] : Serial port and opt. bauds(Default %d).\n"
                 "  -i ID   : Especify a \"user ID\" for \"-qu\" action.\n"
                 "  -v      : Verbose.\n"
         , my, DEFAULT_TCP_PORT, speedt_to_ibaud(DEFAULT_SERIAL_BAUD)); 
}

//Get ip from name
char *get_ip(char *host)
{
  struct hostent *hent;
  int iplen = INET_ADDRSTRLEN ; //XXX.XXX.XXX.XXX
  char *ip = (char *)malloc(iplen+1);
  if (NULL==ip){
    perror("Can't get memory");
    exit(EXIT_FAILURE);
  }
  memset(ip, 0, iplen+1);
  if((hent = gethostbyname(host)) == NULL)
  {
    perror("Can't get IP");
    exit(EXIT_FAILURE);
  }
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    perror("Can't resolve host");
    exit(EXIT_FAILURE);
  }
  return ip;
}

//Open serial connection
int open_serial_conn(char *hgu)
{
  struct termios tio;
  int conn=0;
  speed_t baud;


  //Baud
  if (NULL==strstr(hgu,":"))
    baud=DEFAULT_SERIAL_BAUD;
  else {
    if (!(baud=ibaud_to_speedt(atoi(1+strstr(hgu,":"))))){
      fprintf(stderr,"ERROR: Baud: %d, not supported.\n",atoi(1+strstr(hgu,":")));
      return FALSE;
    }
    *(strstr(hgu,":"))=0x0; //null character for baud remove
  }

  memset(&tio,0,sizeof(tio));
  tio.c_iflag= PARENB | INLCR | IGNCR | ICRNL | IXON |IXOFF ;
  tio.c_oflag=0;
  tio.c_cflag= CS8 | CREAD | CLOCAL ;           // 8n1, see termios.h for more information
  tio.c_lflag=0;
  tio.c_cc[VMIN]=0; // One input byte is enough to return from read()
  tio.c_cc[VTIME]=CONN_READ_TIMETOUT*10; // Inter-character time-out in  tenths of a second

  if (verb) 
    fprintf(stderr,"DEBUG:Openning serial port : %s using %d baud.\n",hgu,speedt_to_ibaud(baud));

  if ((conn=open(hgu, O_RDWR|O_NOCTTY|O_SYNC, 0))<0){
    perror("Could not open tty device\n");
    return FALSE;
  }
  if(cfsetispeed(&tio, baud) < 0 || cfsetospeed(&tio, baud) < 0) {
    perror("Setting serial port speed error");
    return FALSE;
  }
  tcflush(conn, TCIFLUSH);
  tcsetattr(conn,TCSANOW,&tio);

  // allow the process to receive SIGIO
  fcntl(conn, F_SETOWN, getpid());

  return conn;
}

//Open tcp connection
int open_socket_conn(char *hgu)
{
  int conn=0, tmpres=0;
  uint16_t port=0;
  struct sockaddr_in *remote;
  char *ip;

  //Port
  if (NULL==strstr(hgu,":"))
    port=DEFAULT_TCP_PORT;
  else {
    port=atoi(1+strstr(hgu,":"));
    *(strstr(hgu,":"))=0x0; //null character for port remove
  }

  if (verb) 
    fprintf(stderr,"DEBUG:Connecting to: %s:%u\n",hgu,port);

  conn = create_tcp_socket();
  if (!(ip=get_ip(hgu))){
    return FALSE;
  }
  
  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  if (remote==NULL){
    perror("Can't get memory");
    return FALSE;
  }
 
  remote->sin_family = AF_INET;
  tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
  if( tmpres < 0)  {
    fprintf(stderr,"Can't configure remote ip \"%s\"",ip);
    return FALSE;
  }else if(tmpres == 0) {
    fprintf(stderr, "Incorrect ip: %s.\n", ip);
    return FALSE;
  }
  remote->sin_port = htons(port);
  if(connect(conn, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
    fprintf(stderr,"Can't connect to remote unit.\n");
    return FALSE;
  }

  if (verb) 
    fprintf(stderr,"DEBUG:Connected to: %s:%u\n",hgu,port);
  free(remote);
  free(ip);

  return conn;
}

//Translate int baud rate into speed_t 
speed_t ibaud_to_speedt(unsigned int baud)
{
  switch(baud){
    case   300: return B300; 
    case   600: return B600; 
    case  1200: return B1200;
    case  2400: return B2400;
    case  4800: return B4800;
    case  9600: return B9600;
    case 19200: return B19200;
    default:  return FALSE; 
  }
}

//Translate speed_t baud rate into int
unsigned int speedt_to_ibaud(speed_t speed)
{
  switch(speed ){
    case   B300: return 300 ; 
    case   B600: return 600 ;
    case  B1200: return 1200;
    case  B2400: return 2400;
    case  B4800: return 4800;
    case  B9600: return 9600;
    case B19200: return 19200;
    default:  return FALSE; 
  }
}

//Print a message "msg" , and "data" of size "size" in hex representation
// on file descriptor "f"
void hexdump_dbg(char *msg, char *data, int size ,FILE* f )
{
  int i;
  fprintf(f,"%s",msg);
  for (i=0;i<size;i++)
    fprintf(f,"%02X",(unsigned char)*(data+i));
  fprintf(f,"\n");
}

