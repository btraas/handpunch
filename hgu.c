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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "crc_ccitt.h"


//My lib
#include "hgu.h"

//Is a valid operation type value for command packet?
int8_t valid_command_opType (char op_type)
{
  struct opType op_type_info = get_opType_info( op_type, 'C' //C=command
                                                             );

  if ( op_type_info.type ) return TRUE;
  return FALSE;


}

//Is a valid operation type value for response packet?
int8_t valid_response_opType (char op_type)
{

  struct opType op_type_info = get_opType_info( op_type, 'R' //R=response
                                                             );
  if ( op_type_info.type ) return TRUE;
  return FALSE;
}

//Get max. response size by opType command
int16_t HGU_packet_get_max_resp_size(char op_type_cmd)
{
  struct opType op_type_info = get_opType_info( op_type_cmd, 'C' //C=command
                                                             );
  if (! op_type_info.type ) return 0;

  // 4105=length(struct  HGU_packet).I discard sizeof() because mem. arrangement
  return 4105 -  HGU_MAX_DATA_SIZE    + //dataless packet
         op_type_info.comp_max_length -  // data response size
         ((op_type_info.comp_max_length<255)?(1):(0)) //data_length field on 
                                                      // packet is 1byte less 
                                                      // if data is short
         ;
}



//Get info about especific opType command or response
struct opType get_opType_info(char op_type, char type)
{
  struct opType ot_info={0x0,0x0,0,0};


  //Responses OpTypes
  if (type=='R'){
    switch(op_type){
      case 0x24: //HereAreTimeZones
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=768;
        break;
      case 0x30: //HereIsStatus
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=3;
        break;
      case 0x31: //HereIsExtendedUserRecord
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=77;
        break;
      case 0x32: //HereIsUserRecord
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=16;
        break;
      case 0x33: //HereIsCalibrationData
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=6;
        break;
      case 0x34: //HereIsCardData
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=HGU_MAX_DATA_SIZE;
        break;
      case 0x35: //HereAreResults
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=7;
        break;
      case 0x36: //HereIsDataBank
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=4096;
        break;
      case 0x37: //HereIsTemplateVector
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=11;
        break;
      case 0x38: //HereIsNextDatalog
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=18;
        break;
      case 0x39: //HereIsSetupData
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=128;
        break;
      case 0x41: //HereIsExtendedSetup
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=128;
        break;
      case 0x4F: //HereIsOEMCode
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=2;
        break;
      case 0x53: //HereIsReaderInfo
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=102;
        break;
      case 0x58: //UnableToCompleteCommand 
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=3;
        break;
      case 0x61: //HereIsMyTime
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=6;
        break;
      case 0x62: //HereIsKeypadData
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=HGU_MAX_DATA_SIZE;
        break;
    }
  //Commands OpTypes
  } else if (type=='C'){ 
    switch(op_type){
      case 0x23: //LEDControl (added 6/5/00) (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=1;
        ot_info.comp_max_length=3;
        break;
      case 0x24: //SendTimeZones (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=0;
        ot_info.comp_max_length=768;
        break;
      case 0x2B: //HereIsExtendedSetup (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=128;
        ot_info.comp_max_length=3;
        break;
      case 0x2C: //SendExtendedSetup (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=128;
        break;
      case 0x31: //Resume (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x32: //Abort (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x33: //Special Command (Undocumented Command)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x34: //Take Pictures (Special Commands)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=3;
        break;
      case 0x35: //DAQ (Undocumented Command)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x36: //CalculateTemplate (Special Commands)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=11;
        break;
      case 0x37: //HereIsUserRecord (User Database Management)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=16;
        ot_info.comp_max_length=3;
        break;
      case 0x38: //SendUserRecord (User Database Management)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=5;
        ot_info.comp_max_length=16;
        break;
      case 0x39: //SendSpecialStatus (Special Commands)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x3A: //Calibrate (Maintenance)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x3B: //SendStatusChecksum (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x3C: //SendCalibrationData (Maintenance)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=6;
        break;
      case 0x3D: //HereIsSetup (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=128;
        ot_info.comp_max_length=3;
        break;
      case 0x3E: //ClearUserDatabase (User Database Management)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x3F: //RemoveUserRecord (User Database Management)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=5;
        ot_info.comp_max_length=3;
        break;
      case 0x40: //SendLastUserRecord (Results)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=16;
        break;
      case 0x41: //HereIsTime (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=6;
        ot_info.comp_max_length=3;
        break;
      case 0x42: //EmitKeypadTestData (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=5;
        ot_info.comp_max_length=3;
        break;
      case 0x43: //SendResults (Results)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=7;
        break;
      case 0x44: //SendStatusCRC (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x45: //EnterIdlemode (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x46: //HereIsBankNumber (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=2;
        ot_info.comp_max_length=3;
        break;
      case 0x47: //HereIsDataBank (User Database Backup)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=4096;
        ot_info.comp_max_length=3;
        break;
      case 0x48: //SendDataBank (User Database Backup)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=2;
        ot_info.comp_max_length=4096;
        break;
      case 0x49: //EnrollUser (Enrollment)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=2;
        ot_info.comp_max_length=3;
        break;
      case 0x4A: //VerifyOnExternalData (Hand Verification)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=11;
        ot_info.comp_max_length=3;
        break;
      case 0x4B: //SendTemplate (Results)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=11;
        break;
      case 0x4C: //UpdateNVRAM (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x4D: //SendDatalog (Transactions Log Management)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=18;
        break;
      case 0x4E: //SendSetup (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=128;
        break;
      case 0x4F: //OutputControl (Hardware Control)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=1;
        ot_info.comp_max_length=3;
        break;
      case 0x50: //HereIsDisplayMessage (Hardware Control)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=37;
        ot_info.comp_max_length=3;
        break;
      case 0x51: //VerifyOnInternalData (Hand Verification)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=5;
        ot_info.comp_max_length=3;
        break;
      case 0x52: //NextMessageIsTimeZones (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x53: //HereAreTimeZones (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=768;
        ot_info.comp_max_length=3;
        break;
      case 0x54: //Fill Memory With Test Pattern (Undocumented Command)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x55: //Validate Test Pattern (Undocumented Command)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x56: //DisplayCodedMessage (Hardware Control)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=2;
        ot_info.comp_max_length=3;
        break;
      case 0x57: //NextMessageIsBellSchedule Table (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HP|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x58: //HereIsBellScheduleTable (Configuration)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HP|F_HP|HP4K);
        ot_info.length=300;
        ot_info.comp_max_length=3;
        break;
      case 0x5A: //Protected Subcommands (Undocumented Command)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x5C: //SendInternalErrorLog (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=51;
        break;
      case 0x61: //SendTimeAndDate (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=6;
        break;
      case 0x62: //Beep (added 6/5/00) (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=2;
        ot_info.comp_max_length=3;
        break;
      case 0x63: //TB Results (Undocumented Command)
        ot_info.type=op_type;
        ot_info.sup_models=0;
        ot_info.length=-1;
        ot_info.comp_max_length=-1;
        break;
      case 0x64: //HereIsDecisionMenuData (Function Key Menus)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=4096;
        ot_info.comp_max_length=3;
        break;
      case 0x65: //EnterIdleMode2 (added 6/5/00) (Ancillary Command)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
      case 0x66: //SetUserData (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=24;
        ot_info.comp_max_length=3;
        break;
      case 0x67: //SendCardData (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=HGU_MAX_DATA_SIZE;
        break;
      case 0x68: //SendKeypadData (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=1;
        ot_info.comp_max_length=HGU_MAX_DATA_SIZE;
        break;
      case 0x69: //HereIsSmartCardRecord (Enrollment)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=16;
        ot_info.comp_max_length=3;
        break;
      case 0x6D: //SendPreviousDatalog (Transactions Log Management)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=18;
        break;
      case 0x6F: //SendOEMCode (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=2;
        break;
      case 0x70: //PrinterPassThrough (Hardware Control)
        ot_info.type=op_type;
        ot_info.sup_models=(E_HK|E_HP|F_HK|F_HP|HP4K);
        ot_info.length=-1;
        ot_info.comp_max_length=3;
        break;
      case 0x73: //SendReaderInfo (Status)
        ot_info.type=op_type;
        ot_info.sup_models=(F_HK|F_HP|HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=102;
        break;
      case 0x74: //SendExtendedUserRecord (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=5;
        ot_info.comp_max_length=77;
        break;
      case 0x75: //SetExtendedUserData (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=61;
        ot_info.comp_max_length=3;
        break;
      case 0x76: //AddUserMessage (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=37;
        ot_info.comp_max_length=3;
        break;
      case 0x77: //RemoveUserMessages (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=5;
        ot_info.comp_max_length=3;
        break;
      case 0x78: //HereIsExtendedUserRecord (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=77;
        ot_info.comp_max_length=3;
        break;
      case 0x79: //ClearUserMessages (HP-4000 Only)
        ot_info.type=op_type;
        ot_info.sup_models=(HP4K);
        ot_info.length=0;
        ot_info.comp_max_length=3;
        break;
    }
  }


  return ot_info;
}

//HGU_packet initializator
int8_t HGU_packet_init(struct HGU_packet *p)
{
  p->pre_frame  = HGU_PACKET_PRE_POST_FRAME;
  p->start_frame=HGU_PACKET_START_FRAME;
  p->address    =1; //Default HGU unit address
  p->op_type    =0; //Default operation type
  p->data_length=0; //Default operation type
  p->data[0]    =0; //Default data
  p->crc        =0; //Default crc
  p->post_frame =HGU_PACKET_PRE_POST_FRAME;
  return TRUE;
}
 

//Set operation type in HGU packet structure
int8_t HGU_packet_set_opType(struct HGU_packet *p,char op_type)
{
  if (!valid_command_opType(op_type)){
    fprintf(stderr,"ERROR: HGU_packet_set_opType: Invalid command op_type %02X\n",op_type);
    return FALSE;
  }
  p->op_type=op_type;
  return TRUE;
}

//Set data in HGU packet structure
int8_t HGU_packet_set_data(struct HGU_packet *p,char *data, uint16_t data_size )
{
  memcpy(p->data,data,data_size);
  p->data_length=data_size;
  return TRUE;
}

//Set unit number in packet structure
int8_t HGU_packet_set_unitNumber(struct HGU_packet *p,uint8_t un)
{
  p->address=un;
  return TRUE;
}

//Convert a HGU_packet_structure, to binary data, ready to send 
char *HGU_packetSt_to_bin(struct HGU_packet *p, uint16_t *bin_data_length)
{
  char *bin_data;
  int16_t offset=0;
  //if data length is greater than 255, then length field is 2bytes long
  int8_t length_field_size=((p->data_length>255)?(2):(1));

  //update op_type, if length field is 2bytes long
  if (length_field_size==2) 
    p->op_type=p->op_type|0x80;

  *bin_data_length=
           1  //pre_frame 
         + 1  //start_frame 
         + 1  //address 
         + 1  //op_type
         + length_field_size // length of data
         + p->data_length           //Data 
         + 2   //crc
         + 1   //post_frame 
         ;
  if ((bin_data=malloc(*bin_data_length))==NULL){
    perror("malloc");
    return FALSE;
  }
  memcpy(bin_data+offset,p,4); // pre_frame,start_frame,address,op_type
  offset+=4;
  memcpy(bin_data+offset,&p->data_length,length_field_size); // data_length
  offset+=length_field_size;
  memcpy(bin_data+offset,p->data,p->data_length); // data
  offset+=p->data_length;

  //Update crc
  //-2 and +2 :pre_frame and start_frame are not in crc
  p->crc=crc_calc(bin_data+2,offset-2);
  memcpy(bin_data+offset,&p->crc,2); //crc
  offset+=2;
  memcpy(bin_data+offset,&p->post_frame,1); //post_frame 
  return bin_data;
}

//Copy HereIsReaderInfo data response to appropiate structre
struct HereIsReaderInfo *HGU_HereIsReaderInfo_Data_to_st(struct HGU_packet *resp)
{
  struct HereIsReaderInfo *reader_info;

  if (resp->data_length != ST_SIZE_HereIsReaderInfo){
    fprintf(stderr,"ERROR: HereIsReaderInfo received data, is %hd length. Expected %hd .",resp->data_length,ST_SIZE_HereIsReaderInfo);
    return FALSE;
  }

  if (!(reader_info=malloc(ST_SIZE_HereIsReaderInfo))){
    perror("malloc");
    return FALSE;
  }

  memcpy(reader_info,resp->data,ST_SIZE_HereIsReaderInfo);
  return reader_info;
}
 
//check val of char arg.  for [0-9,A-F]
uint8_t is_hex_char(char c)
{
    if ( ( c >= 0x30 && c<= 0x39) || ( c >= 0x41 && c<= 0x46) )
      return TRUE;
    else return FALSE;
}

//copy chars en double-byte format from char*src to simple byte char*dst with dst_size long. 
int8_t hexchar_to_char_cpy (char *src, char* dst, int16_t dst_size)
{
  int16_t i=0;
  for (i=0;i<dst_size*2;i+=2){
    if (!is_hex_char(*(src+i)) || !is_hex_char(*(src+i+1)) ){
      fprintf(stderr,"ERROR: Incorrect HEX char \"%c%c\".\n",*(src+i),*(src+i+1));
      return FALSE;
    }
    sscanf(src+i,"%2X", (unsigned int *)(dst+i/2));
  }
  return TRUE;
}

//Parse bin protocol data, and save its in HGU_packet structure
int8_t HGU_bin_to_HereIsUserRecord_st(char *bin_data, struct HereIsUserRecord *u)
{
  uint8_t offset=0;

 
   //user record id vield
   if (!hexchar_to_char_cpy (bin_data+offset, (char*)u->urid, SIZE_UR_ID))
     return FALSE;
   offset+=2*SIZE_UR_ID; //skip field

   //space
   if(*(bin_data+offset)!=0x20){
     fprintf(stderr,"ERROR: Incorrect char \"%c\".\n",*(bin_data+offset));
     return FALSE;
   }
   offset+=1 ; //skip space

   //template field
   if (!hexchar_to_char_cpy (bin_data+offset,(char*)u->tpl, SIZE_UR_TEMPLATE))
     return FALSE;
   offset+=2*SIZE_UR_TEMPLATE; //skip field

   //space
   if(*(bin_data+offset)!=0x20){
     fprintf(stderr,"ERROR: Incorrect char \"%c\".\n",*(bin_data+offset));
     return FALSE;
   }
   offset+=1;//skip space

   //authority field
   if (!hexchar_to_char_cpy (bin_data+offset,(char*)&u->authority, 1))
     return FALSE;
   offset+=2; //skip field

   //space
   if(*(bin_data+offset)!=0x20){
     fprintf(stderr,"ERROR: Incorrect char \"%c\".\n",*(bin_data+offset));
     return FALSE;
   }
   offset+=1 ; //skip space

   //timezone field
   if (!hexchar_to_char_cpy (bin_data+offset,(char*)&u->timezone, 1))
     return FALSE;
   
  return TRUE;
}

//Parse bin protocol data, and save its in HGU_packet structure
int8_t HGU_packetBin_to_st(char *bin_data, int16_t bin_data_length , struct HGU_packet *p)
{
  int16_t offset=0;
  int8_t data_length_field_width=1;

  //min length = ST_SIZE_HGU_packet -
  //             1 -                          // length_data field is 2 bytes. min. 1byte
  //             HGU_MAX_DATA_SIZE -          // without data
  //             1                            // post_frame is optional
  if (bin_data_length< ST_SIZE_HGU_packet-1-HGU_MAX_DATA_SIZE-1 ){
    fprintf (stderr,"ERROR: incomplete packet received. Min. expected:%d, received:%hd\n",
             (short int)sizeof(struct HGU_packet) -1-HGU_MAX_DATA_SIZE-1,bin_data_length);
    return 3;
  }

  //pre_frame,start_frame,address and op_type fields
  memcpy(p,bin_data,4); offset=4; 

  //heighth bit in op_type, indicates if length field is 2 bytes long
  if (p->op_type & 0x80){
    p->op_type=p->op_type & (~0x80);
    data_length_field_width=2;
  }
  
  //valid op_type?
  if (!valid_response_opType(p->op_type)){
    fprintf(stderr,"ERROR: Invalid operation type in received packet(%02X).\n",p->op_type);
    return FALSE;
  }


  //data length field 
  p->data_length=0;
  memcpy(&p->data_length,bin_data+offset,data_length_field_width);
  offset+=data_length_field_width;
  if (p->data_length > HGU_MAX_DATA_SIZE){
    fprintf(stderr,"ERROR: Invalid data length field value:%hd  . Max:%hd .\n",p->data_length, HGU_MAX_DATA_SIZE);
    return 3;
  }

  //data field
  if (offset+p->data_length > bin_data_length ){ 
    fprintf (stderr,
             "ERROR: length data corruption in received packet. Data length:%d exceeds total length:%d\n",
             offset+p->data_length,bin_data_length);
    return 3;
  }
  memcpy(p->data,bin_data+offset,p->data_length);
  offset+=p->data_length;

  //crc. 
  if (offset+2 > bin_data_length){
    fprintf (stderr,"ERROR: length data corruption in received packet.\n");
    return 3;
  }
  //-2 and +2 :pre_frame and start_frame are not in crc
  p->crc=crc_calc(bin_data+2,offset-2);
  //Check crc
  if(0!=memcmp(&p->crc,bin_data+offset,2)){
    fprintf (stderr,"ERROR: CRC error in received packet:received %02X%02X, expected %04X .\n",
             (unsigned char)*(bin_data+offset+1),(unsigned char)*(bin_data+offset),p->crc);
    return 2;
  }

  //Last byte: post_frame, agree specification, is optional
  return TRUE;
}

//Get how many memory banks have, if you know reader info
int8_t HGU_how_many_mem_banks(struct HereIsReaderInfo *reader_info)
{
  return reader_info->UCap * HGU_get_record_size_by_model(reader_info->model)/HGU_MEM_BANK_SIZE;
}



//Get user record size, model based
int8_t HGU_get_record_size_by_model(int8_t model)
{
  switch (model){
    case 0: return 16;// HP-2000
    case 1: return 16;// HP-3000
    case 2: return 77;// HP-4000
    case 3: return 16;// HK-CR
    case 4: return 16;// HK-2
  }
  return 0; //undefined
}

//Return substring from string pointer
char *substr(char *str, int size){
  char *sub;

  if (!(sub=malloc(size))){
    perror("malloc");
    return FALSE;
  }
  memcpy(sub,str,size);
  return sub;
}

//Translate codified baud rate in real value
uint16_t transl_baud(char index,char model)
{
  switch(index){
    case 0:  
      if (model=='E') return 38400;
      if (model=='F') return 28800;
    case 1: return 19200;
    case 2: return 9600;
    case 3: return 4800;
    case 4: return 2400;
    case 5: return 1200;
    case 6: return 600;
    case 7: return 300;
  }
  return 0;
}

//Translate codified date format to human value
char *transl_datef(char index)
{
  switch(index){
    case 0x0: return "United States Default (05/29/98)";
    case 0x1: return "United States Default (05/29/98)";
    case 0x2: return "International Alpha Dash (29-MAY-98)";
    case 0x3: return "International Numeric Dash (29-05-98)";
    case 0x4: return "International Numeric Slash (29/05/98)";
    case 0x5: return "United States Numeric Dash (05-29-98)";
    case 0x6: return "Year 2000 Compliant (29MAY2000)";
  }
  return "Invalid date format.";
}

//Translate codified lang to human value
char *transl_lang(char index)
{
  switch(index){
    case 0x0: return "English";
    case 0x1: return "UK English";
    case 0x2: return "Japanese";
    case 0x3: return "French";
    case 0x4: return "Italian";
    case 0x5: return "Spanish";
    case 0x6: return "German";
    case 0x7: return "Russian";
    case 0x8: return "Indonesian";
    case 0x9: return "Portuguese";
    case 0x10: return "Polish";
    case 0x11: return "Dutch";
    case 0x12: return "Slovak";
  }
  return "Invalid language.";
}

//Translate codified model number to human value
char *transl_modeln(char index)
{
  switch(index){
    case 0x0: return "HP-2000";
    case 0x1: return "HP-3000";
    case 0x2: return "HP-4000";
    case 0x3: return "HK-CR";
    case 0x4: return "HK-2";
  }
  return "Invalid model number.";
}

//Translate codified memory to human value
char *transl_memory(char index)
{
  switch(index){
    case 0x0: return "128K";
    case 0x1: return "256K";
    case 0x2: return "640K";
  }
  return "Invalid memory value.";
}

//Translate serial number prefix codified to human value
char *transl_serialn_prefix(char index)
{
  switch(index){
    case 0x0: return "";
    case 0x1: return "E6-";
    case 0x2: return "E6HP-";
  }
  return "";
}

//Translate adaptor codified to human value
char *transl_adaptor(char index)
{
  switch(index){
    case 0x0: return "NONE";
    case 0x1: return "MODEM";
    case 0x2: return "ETHERNET";
  }
  return "";
}

//Translate style PROM codified to human value
char *transl_stylePROM(char index)
{
  switch(index){
    case 0x0: return "MAINSTREAM_ONE";
    case 0x1: return "A-STYLE";
    case 0x2: return "B-STYLE";
    case 0x5: return "E-STYLE";
  }
  return "";
}

//Translate timezone id, to human value
char *transl_Timezone(int8_t index)
{  
  if (index < 0 || index > 61 ) return "undefined";
  switch(index){
    case 0: return "Always";
    case 61: return "Never";
    default: return "Timezone restricted.";
  }
}

//Translate authority byte in 0x32 (HereIsUserRecord) response, to human value
char *transl_Authority(int8_t index)
{
  if (index < 0 || index > 61 ) return "undefined";
  switch(index){
    case 0: return "Always";
    case 61: return "Never";
    default: return "Timezone restricted.";
  }
}


//Translate Format in Datalog response to human value
char *transl_DatalogFormat(char index)
{
  switch(index){
    case 0x0: return "Transaction Buffer Empty";
    case 0x1: return "User Enrolled";
    case 0x3: return "Identity Unknown";
    case 0x4: return "Exit Granted";
    case 0x6: return "Access Denied";
    case 0x7: return "Identity Verified";
    case 0x8: return "User Removed";
    case 0x9: return "Command Mode Entered";
    case 0xA: return "Command Mode Exited";
    case 0xB: return "System Re-calibrated";
    case 0xD: return "Door Forced Open";
    case 0xE: return "Tamper Activated";
    case 0xF: return "Supervisor Override";
    case 0x11: return "Auxiliary Input ON";
    case 0x12: return "Request To Exit Activated";
    case 0x13: return "Auxiliary Output ON";
    case 0x14: return "Baud Rate Changed";
    case 0x15: return "Messages Read";
    case 0x16: return "Unit Address Changed";
    case 0x17: return "Site Code Changed";
    case 0x18: return "Time and Date Set";
    case 0x19: return "Lock Setup";
    case 0x1A: return "Passwords Changed";
    case 0x1B: return "Reject Threshold Set";
    case 0x1C: return "Aux Out Setup Changed";
    case 0x1D: return "Printer Setup Changed";
  }
  return "undefined";

}

//Display HereIsReaderInfo response in comprensible mode
int8_t view_HereIsReaderInfo(char *data,short int format)
{

  if (format == HGU_OUT_F_PARSEABLE )
    printf(
     "Model:\"%s\"\n"  
     "Memory:\"%s\"\n"
     "PromDate:\"%20s\"\n"
     "ModelName:\"%17s\"\n"
     "UCap:%hu\n"
     "TCap:%hu\n"
     "UNum:%hu\n"
     "TNum:%hu\n"
    ,transl_modeln(*(data)) //model
    ,transl_memory(*(data+1)) //memory
    ,substr(data+2,20)//Prom date
    ,substr(data+22,17) //Model name
    ,TO_INT16(*(data+45),*(data+44)) //Max. user capacity
    ,TO_INT16(*(data+47),*(data+46)) //Max. trans. capacity
    ,TO_INT16(*(data+49),*(data+48)) //Current Enrolled users
    ,TO_INT16(*(data+51),*(data+50)) //Current Transactions
    );
  else
    printf(
     "HereIsReaderInfo:\n" 
     "----------------\n"
     "          Model number:\"%s\"\n"  
     "               Memory :\"%s\"\n"
     "             PROM Date:\"%20s\"\n"
     "            Model Name:\"%17s\"\n"
     "  Max. user capacity  :%hu\n"
     "  Max. trans. capacity:%hu\n"
     "Current Enrolled users:%hu\n"
     "  Current Transactions:%hu\n"
     "   LCD Line 1(Startup):\"%14s\"\n"
     "     Adaptor installed:%s\n" 
     "            Style PROM:%s\n" 
     //"Serian number:%s%d\n"
    ,transl_modeln(*(data)) //model
    ,transl_memory(*(data+1)) //memory
    ,substr(data+2,20)//Prom date
    ,substr(data+22,17) //Model name
    //,? //Serial
    ,TO_INT16(*(data+45),*(data+44)) //Max. user capacity
    ,TO_INT16(*(data+47),*(data+46)) //Max. trans. capacity
    ,TO_INT16(*(data+49),*(data+48)) //Current Enrolled users
    ,TO_INT16(*(data+51),*(data+50)) //Current Transactions
    ,substr(data+52,14) //LCD Line 1(Startup)
    ,transl_adaptor(*(data+66)) //Adaptor installed
    ,transl_stylePROM(*(data+67)) //Style PROM
    );
  return TRUE;
}

int8_t view_HereIsDataBank(char *data, short int format,
                           struct HereIsReaderInfo *reader_info )
{
  int16_t offset=0;
 
  for(offset=0; 
      offset< HGU_MEM_BANK_SIZE; 
      offset+=HGU_get_record_size_by_model(reader_info->model)) {
      if (!view_HereIsUserRecord((struct HereIsUserRecord*)(data+offset), format)) 
        return FALSE;
  }
  return TRUE;
}

//Display HereIsUserRecord in comprensible mode
int8_t view_HereIsUserRecord( struct HereIsUserRecord *ur , short int format)
{
  unsigned short int i=0;
  char urs[HereIsUserRecord_DUMP_SIZE+1]; //+1:null
  int16_t offset=0;

  char urid[SIZE_UR_ID*2+1];
  char tpl[SIZE_UR_TEMPLATE*2+1];

  //urid with only 0xFF is a empty record 
  for (i=0;i<SIZE_UR_ID; i++) if (ur->urid[i]!=0xFF)  break ; 
  if (i==SIZE_UR_ID) return TRUE;

  //urid
  for(i=0;i<SIZE_UR_ID;i++) 
    sprintf(urs+i*2, "%02X", ur->urid[i]);
  offset+=SIZE_UR_ID*2;

  //space
  *(urs+offset)=0x20; offset++;

  //template
  for(i=0;i<SIZE_UR_TEMPLATE;i++) 
    sprintf(urs+offset+i*2, "%02X", ur->tpl[i]);
  offset+=SIZE_UR_TEMPLATE*2;

  //space
  *(urs+offset)=0x20; offset++;

  //authority
  sprintf(urs+offset, "%02X", ur->authority);
  offset+=2;

  //space
  *(urs+offset)=0x20; offset++;

  //timezone
  sprintf(urs+offset, "%02X", ur->timezone);

  printf("%s\n",urs);
  return TRUE;
}

//Display HereIsNextDatalog in comprensible mode
int8_t view_HereIsNextDatalog(char *data, short int format)
{
  struct dataLog dl=dataLog_Bin_to_st(data);
  struct tm tm_time=get_tm_from_HereIsMyTime((char*)dl.timestamp);
  char time_st[20];
  if (0==strftime(time_st, 20, "%Y.%m.%d %H:%M:%S", &tm_time)){
    fprintf(stderr,"Error parsing date:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
       dl.timestamp[0],dl.timestamp[1],dl.timestamp[2],dl.timestamp[3],dl.timestamp[4]);
    return FALSE;
  }

  if (format == HGU_OUT_F_PARSEABLE ) {
    printf( "%s %02X%02X%02X%02X%02X\n"
      ,time_st //time
      ,dl.data1[0],dl.data1[1],dl.data1[2],dl.data1[3],dl.data1[4] //data1
    );
  } else
    printf(
      "Address: %02X\n"
      "   Time: %s\n"
      " Format: %02X (%s)\n"
      "    ID : %02X%02X%02X%02X%02X\n"
      ,dl.address  //address
      ,time_st   //time
      ,dl.format,transl_DatalogFormat(dl.format) //format
      ,dl.data1[0],dl.data1[1],dl.data1[2],dl.data1[3],dl.data1[4] //data1
    );

  return TRUE;
}

//Display HereIsExtendedSetup response in comprensible mode
int8_t view_HereIsExtendedSetup(char *data, short int format)
{
  if (format == HGU_OUT_F_PARSEABLE ) {
    printf( 
      "ReadyString:\"%14s\"\n"
      "SerialMode:\"%s\"\n"      
      "LanguageType:\"%s\"\n"      
      "DateFormat:\"%s\"\n"      
      "LanguageType2:\"%s\"\n"      
      ,substr(data,14)
      ,((data+14)?("RS-232"):("RS-422"))
      ,transl_lang(*(data+32))
      ,transl_datef(*(data+33))
      ,transl_lang(*(data+40))
    );
  } else
    printf( 
      "HereIsExtendedSetup info: (relevant information)\n"
      "------------------------\n"
      "           Ready String:\"%14s\"\n"
      "            Serial Mode:\"%s\"\n"      
      "Language (text display):\"%s\"\n"      
      "            Date Format:\"%s\"\n"      
      " Language (LCD display):\"%s\"\n"      
      ,substr(data,14)
      ,((data+14)?("RS-232"):("RS-422"))
      ,transl_lang(*(data+32))
      ,transl_datef(*(data+33))
      ,transl_lang(*(data+40))
    );
  return TRUE;
}

//Get "format" field from SendNextDatalog/SendPreviousDatalog binary response
char get_datalog_format_from_bin(struct dataLog *dl)
{
  return dl->format; 
}

//Copy bin data from HereIsNextDatalog response, to dataLog structure
struct dataLog dataLog_Bin_to_st(char *data)
{
  struct dataLog dl;
  struct opType op_type_info = get_opType_info( 0x38,//HereIsNextDatalog
                                                'R'); //R=response
  memcpy(&dl,data,op_type_info.length);
  return dl;
}

//Convert "unsigned long int" user record id, to "char[]" hgu compatible format
int8_t i2urid(unsigned long int iurid,  unsigned char *urid )
{
  char dec[SIZE_UR_ID*2+1];
  int8_t i;

  sprintf(dec,"%010ld",iurid);
  for (i=0;i<SIZE_UR_ID;i++)
    urid[i]=((int8_t)dec[2*i]-48) * 16 + (int8_t)dec[2*i+1]-48;
  return TRUE;
}

//Translate HereIsMyTime time format to struct tm
struct tm get_tm_from_HereIsMyTime( char *data)
{
  int year=0;
  //2digit year to 4digit ( the value 80 is mi opinion, because I can't find info 
  // in manuals)
  if ((unsigned)*(data+5)==100 || (unsigned)*(data+5)==0 ) year=2000;
  else if ((unsigned)*(data+5)<80) year=2000+(unsigned int)*(data+5);
  else year=1900+(unsigned int)*(data+5);
  
  return (struct tm){
                  (unsigned) *data,       //sec
                  (unsigned) *(data+1),   //min
                  (unsigned) *(data+2),   //hour
                  (unsigned) *(data+3),   //day
                  (unsigned) *(data+4)-1, //month
                  year-1900, //year
                          0, //day of the week
                          0, //day in the year 
                          0 //daylight saving time
                 };
}

//Display HereIsMyTime response in comprensible mode
int8_t view_HereIsMyTime(char *data,short int format)
{
  struct tm tm_time=get_tm_from_HereIsMyTime(data);
  char time_st[20];
  if (0==strftime(time_st, 20, "%Y.%m.%d %H:%M:%S", &tm_time)){
    fprintf(stderr,"Error parsing date:0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
       data[0],data[1],data[2],data[3],data[4]);
    return FALSE;
  }
  return printf("%s\n",time_st);
}

//Check if DLOG_RDY is set in a HereIsStatus response(0x30)
//The DLOG_RDY is set whenever de datalog buffer is not empty (you can 
// retrieving transactions).
int8_t HGU_pagcket_is_DLOG_RDY_set(char *data)
{
    if (*(data+1) & 0x10)
      return TRUE;
    return FALSE;
}


//Display HereIsStatus_response in comprensible mode
int8_t view_HereIsStatus_response(char *data,short int format)
{

  //To do: output format dependent
  printf("HereIsStatus info (do you understand it?):\n-----------------\n");
  //BYTE 1
  printf("H_READ    :%s\n",
    ((*data & 0x01)?("Trying to read a hand"):
                    ("Fingers are in proper position on the platen.")));
  printf("LED1      :%s\n",
    ((*data & 0x02)?("Hand reading."):
                    ("Little finger is closed to the guide pin.")));
  printf("LED2      :%s\n",
    ((*data & 0x04)?("Hand reading."):
                    ("Ring finger is closed to the guide pin.")));
  printf("LED3      :%s\n",
    ((*data & 0x08)?("Hand reading."):
                    ("Middle finger is closed to the guide pin.")));
  printf("LED4      :%s\n",
    ((*data & 0x10)?("Hand reading."):
                    ("Index finger is closed to the guide pin.")));
  printf("ANY_KEY   :%s\n",
    ((*data & 0x20)?("Key is pressed on the keypad."):
                    ("SendStatus command is received.")));
  printf("AUX_OUT_1 :%s\n",
    ((*data & 0x40)?("(Model F) AuxOut1 is activated (low)."):
                    ("(Model F) AuxOut1 is disactivated (hight).")));
  printf("AUX_OUT_2 :%s\n",
    ((*data & 0x80)?("(Model F) AuxOut2 is activated (low)."):
                    ("(Model F) AuxOut2 is disactivated (hight).")));
  //BYTE 2
  printf("RES_SYS   :%s\n",
    ((*(data+1) & 0x01)?("Unit is initialized."):
                      ("Any command other than SendStatus is received.")));
  printf("VERIFY_RDY:%s\n",
    ((*(data+1) & 0x02)?("A hand has been read due to PIN entry at the unit."):
                      ("SendResults or SendTemplate command is received.")));
  printf("RSLTS_RDY :%s\n",
    ((*(data+1) & 0x04)?("Enroll, VerifyOnExternalData, or VerifyOnInternalData command has completed."):
                      ("SendResults or SendTemplate command is received.")));
  printf("FAILED_CMD:%s\n",
    ((*(data+1) & 0x08)?("Enroll, VerifyOnExternalData, or VerifyOnInternalData command fails to complete."):
                      ("HereIsStatus message is sent from the unit to the host.")));
  printf("DLOG_RDY  :%s\n",
    ((*(data+1) & 0x10)?("The datalog buffer is not empty."):
                      ("Datalog buffer empties.")));
  printf("ID_NIM    :%s\n",
    ((*(data+1) & 0x20)?("An ID entered for verification is not in memory."):
                      ("The SendResults command is received.")));
  printf("CMD_BUSY  :%s\n",
    ((*(data+1) & 0x40)?("Enroll, VerifyOnExternalData, or VerifyOnInternalData command is in progress."):
                      ("?")));
  printf("KP_ID     :%s\n",
    ((*(data+1) & 0x80)?("An ID is entered by the keypad."):
                      ("An ID is entered by a card reader.")));
  //BYTE 3
  printf("TAMPER    :%s\n",
    ((*(data+2) & 0x01)?("OK."):
                      ("No tamper.")));
  printf("AUX_IN_1  :%s\n",
    ((*(data+2) & 0x02)?("(Model F) Hight."):
                      ("(Model F) Low.")));
  printf("DOOR_SW   :%s\n",
    ((*(data+2) & 0x04)?("Hight."):
                      ("Low.")));
  printf("AUX_IN_0  :%s\n",
    ((*(data+2) & 0x08)?("(Model F) Hight."):
                      ("(Model F) Low.")));
  printf("REX       :%s\n",
    ((*(data+2) & 0x10)?("Request to exit status Hight."):
                      ("Request to exit status Low.")));
  //Not used 
  //printf(":%s\n", ((*(data+2) & 0x20)?("."): ("."))); 
  printf("AUX_OUT_0 :%s\n",
    ((*(data+2) & 0x40)?("AuxOut0 or Bell status: Hight."):
                      ("AuxOut0 or Bell status: Low.")));
  printf("LOCK      :%s\n",
    ((*(data+2) & 0x80)?("Lock status: Hight."):
                      ("Lock status: Low.")));
  return TRUE;

}

//Display different responses  in comprensible mode
int8_t HGU_packet_show_response(struct HGU_packet *p, short int format, 
                                struct HereIsReaderInfo *reader_info )
{
  switch(p->op_type){
    case 0x30: //HereIsStatus
      if (!view_HereIsStatus_response(p->data,format)) return FALSE;
      break;
    case 0x32: //HereIsUserRecord
      if (!view_HereIsUserRecord((struct HereIsUserRecord*)p->data,format)) return FALSE;
      break;
    case 0x36: //HereIsDataBank
      if (!view_HereIsDataBank(p->data,format, reader_info)) return FALSE;
      break;
    case 0x38: //HereIsNextDatalog
      if (!view_HereIsNextDatalog(p->data,format)) return FALSE;
      break;
    case 0x41: //HereIsExtendedSetup
      if (!view_HereIsExtendedSetup(p->data,format)) return FALSE;
      break;
    case 0x53: //HereIsReaderInfo
      if (!view_HereIsReaderInfo(p->data,format)) return FALSE;
      break;
    case 0x61: //HereIsMyTime
      if (!view_HereIsMyTime(p->data,format)) return FALSE;
      break;
    default:
      fprintf(stderr,"ERROR: HGU_packet_show_response: Operation type %02X not defined.\n",p->op_type);
      return FALSE;
    break;
  }
  return TRUE;
}

int8_t is_num(char c)
{
  if (c>47 && c<58) return TRUE;
  return FALSE;
}

//Check if char* arg verified "YYYY.MM.DD HH:MM:SS" format
int8_t time_format_ok(char *st_time){

  //size?
  if (strlen(st_time)!=19 || 
      !is_num(*st_time)   ||
      !is_num(*(st_time+1)) ||
      !is_num(*(st_time+2)) ||
      !is_num(*(st_time+3)) ||
      *(st_time+4)!='.'     ||
      !is_num(*(st_time+5)) ||
      !is_num(*(st_time+6)) ||
      *(st_time+7)!='.'     ||
      !is_num(*(st_time+8)) ||
      !is_num(*(st_time+9)) ||
      *(st_time+10)!=' '    ||
      !is_num(*(st_time+11))||
      !is_num(*(st_time+12))||
      *(st_time+13)!=':'    ||
      !is_num(*(st_time+14))||
      !is_num(*(st_time+15))||
      *(st_time+16)!=':'    ||
      !is_num(*(st_time+17))||
      !is_num(*(st_time+18))   
     ){
    fprintf(stderr,"ERROR: bad format time. Format: \"YYYY.MM.DD HH:MM:SS\"\n");
    return FALSE;
  }
  return TRUE;
}

//Returns a structure set to the value of time string arg.
struct HereIsTime *HGU_get_time_from_st(char *st_time)
{
  struct HereIsTime *HIT_time=NULL;
  if (!time_format_ok(st_time)) return FALSE;

  if (!(HIT_time=malloc(ST_SIZE_HereIsTime))){
    perror("malloc");
    return FALSE;
  }
  HIT_time->year =(uint8_t)atoi(st_time+2); //Only 2bytes from year
  HIT_time->month=(uint8_t)atoi(st_time+5);
  HIT_time->day  =(uint8_t)atoi(st_time+8);
  HIT_time->hour =(uint8_t)atoi(st_time+11);
  HIT_time->min  =(uint8_t)atoi(st_time+14);
  HIT_time->sec  =(uint8_t)atoi(st_time+17);
  return HIT_time;
}


//Returns a structure set to the value of a time_t arg, or system local time
// if it is NULL
struct HereIsTime *HGU_get_time_from_time_t(time_t timer)
{
  struct tm *tm_time=localtime(&timer);
  struct HereIsTime *HIT_time=NULL;
  if (!(HIT_time=malloc(ST_SIZE_HereIsTime))){
    perror("malloc");
    return FALSE;
  }
  HIT_time->sec  =(int8_t)tm_time->tm_sec;
  HIT_time->min  =(int8_t)tm_time->tm_min;
  HIT_time->hour =(int8_t)tm_time->tm_hour;
  HIT_time->day  =(int8_t)tm_time->tm_mday;
  HIT_time->month=(int8_t)tm_time->tm_mon+1;
  HIT_time->year =(int8_t)tm_time->tm_year;

  return HIT_time;
}


