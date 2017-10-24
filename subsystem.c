//This file will cover all stack up events
#include <msp430.h>
#include <ctl.h>
#include <ARCbus.h>
#include <Error.h>
#include <SDlib.h>
#include "subsystem.h"

 #define ASCIIOUT_STR  "%c "

CTL_EVENT_SET_t SYS_evt; // This creates the event struct,change SYS to actual subsystem

int SUB_parseCmd(unsigned char src,unsigned char cmd,unsigned char *dat,unsigned short len,unsigned char flags);
//links SUB_parseCmd to arclib to allow you to decode non-common commands that are only specific to your system. 
CMD_PARSE_DAT SUB_parse={SUB_parseCmd,CMD_PARSE_ADDR1,BUS_PRI_NORMAL,NULL};

//handle subsystem specific commands - this is for I2C commands on the BUS that are not SUB events, so system specific commands.
int SUB_parseCmd(unsigned char src,unsigned char cmd,unsigned char *dat,unsigned short len, unsigned char flags){
  switch(cmd){
    case CMD_LEDL_READ_BLOCK: 
    ctl_events_set_clear(&SYS_evt,SYS_EV_1,0);
    P7OUT^=0x01;
    break;

    default:
      return ERR_UNKNOWN_CMD;
  }
}


//parse subsystem events
void sub_events(void *p) __toplevel{
  unsigned int e,len;
  int i;
  unsigned char buf[10],*ptr;
  extern unsigned char async_addr;

  for(;;){
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&SUB_events,SUB_EV_ALL|SUB_EV_ASYNC_OPEN|SUB_EV_ASYNC_CLOSE,CTL_TIMEOUT_NONE,0);
    if(e&SUB_EV_PWR_OFF){  //**************************************** Sent when the subsystem receives the power off command **********************************
      //print message
      puts("System Powering Down\r");
       P7OUT^=0x02;
    }
    if(e&SUB_EV_PWR_ON){ //***************************************** Sent when the subsystem receives the power on command ************************************
      //print message
      puts("System Powering Up\r");
    }
    if(e&SUB_EV_SEND_STAT){ // ************************************* Sent when the subsystem receives the send status command  ********************************
      //send status
      puts("Sending status\r");
      //setup packet 
      //ptr=BUS_cmd_init(buf,CMD_SYS_STAT); // replace CMD_SYS_STAT with actual subsystem ie CMD_COMM_STAT
      //send command
      //BUS_cmd_tx(BUS_ADDR_CDH,buf,0,0); //sending status contained in "buf" CDH 

      // FOR TESTING SEND_I2C command!!

    }
    if(e&SUB_EV_SPI_DAT){ // ************************************** Sent when SPI data is received correctly **************************************************
      puts("SPI data rx\r");
    }
    if(e&SUB_EV_SPI_ERR_CRC){ // ********************************* Sent when SPI transaction was rejected because of busy buffer *****************************
      puts("SPI bad CRC\r");
    }
    if(e&SUB_EV_SPI_ERR_BUSY){
      puts("SPI Busy\r");
    }
    if(e&SUB_EV_ASYNC_OPEN){ // ********************************** An async connection was opened remotely **************************************************
      //close async connection, not supported
      puts("Async open called");
    }
    if(e&SUB_EV_ASYNC_CLOSE){ // ********************************** An async connection was closed remotely  *************************************************
     //close async connection, not supported
     puts("Async close called");
    }
    if(e&SUB_EV_INT_0){ // ************************************** Event on interrupt bus pin 0  *************************************************************  
    puts("EV 0 has been called.");
    }
  }
}

void sys_events(void *p) __toplevel{
  unsigned int e;
  ctl_events_init(&SYS_evt,0); //Initialize Event
  
  for(;;){
    // wait for events
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&SYS_evt,SYS_EVT_ALL,CTL_TIMEOUT_NONE,0);
//*************************************************** Events to be processed for system ****************************************************************
    if(e&SYS_EV_1){
      
      char buffer[512];
      int mmcReturnValue, result , i,resp;
      puts("SYS_EV_1 called");
      mmcInit_msp();  // Sets up the interface for the card
      mmc_pins_on();  //Sets up MSP to talk to SD card
      mmcReturnValue=mmcInit_card();

      if (mmcReturnValue==MMC_SUCCESS){ // check good initialization 
        printf("\rCard initalized Sucessfully\r\n");
      }
      else{
        printf("Check SD card.\r\nInitalized failed.\r\n Error %i\r\n",mmcReturnValue);
      }
      resp=mmcReadBlock(100,buffer);
       //print response from SD card
        printf("%s\r\n",SD_error_str(resp));

        for(i=0;i<9;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

        if(i<8){
          printf(ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }

        else{
          printf(ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR ASCIIOUT_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }
        }
        
    }
    if(e&SYS_EV_2){
    puts("SYS_EV_2 called");
    }
    if(e&SYS_EV_3){
    puts("SYS_EV_3 called");
    }
    if(e&SYS_EV_4){
    puts("SYS_EV_4 called");
    }
  }
}

