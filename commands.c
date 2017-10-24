/**********************************************************************************************************************************************
The commands.c file is for commands that will be displayed through the serial terminal. 
In order to add a command you must create a function as seen below.
Then function must be added to the "const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd}" table at the end of the file.
**********************************************************************************************************************************************/
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <terminal.h>
#include <commandLib.h>
#include <stdlib.h>
#include <ARCbus.h>
#include <SDlib.h>
#include <i2c.h>

extern sys_addr;
//*********************************************************** passing arguments over the terminal *********************************************
int example_command(char **argv,unsigned short argc){
  int i,j;
  //TODO replace printf with puts ? 
  printf("This is an example command that shows how arguments are passed to commands.\r\n""The values in the argv array are as follows : \r\n");
  for(i=0;i<=argc;i++){
    printf("argv[%i] = 0x%p\r\n\t""string = \"%s\"\r\n",i,argv[i],argv[i]);
    //print out the string as an array of hex values
    j=0;
    printf("\t""hex = {");
    do{
      //check if this is the first element
      if(j!='\0'){
        //print a space and a comma
        printf(", ");
      }
      //print out value
      printf("0x%02hhX",argv[i][j]);
    }while(argv[i][j++]!='\0');
    //print a closing bracket and couple of newlines
    printf("}\r\n\r\n");
  }
  return 0;
}


/*********************************************************** Using the Timer_A1 ***************************************************************
* DONT USE TIMER0_Ax_VECTOR !!! this interrupt is use in library code and will cause a collision 
* Use TIMERx_Ay_VECTOR x=2,3 & y=0,1
* TIMER0_Ax_VECTOR used in ARClib ? 
* TIMER1_Ax_VECTOR used in ????
**********************************************************************************************************************************************/
int example_timer_IR(char **argv,unsigned short argc){
  int timer_check;
  WDTCTL = WDTPW+WDTHOLD;                                   // Stop WDT
  P7DIR |= 0xFF;                                            // Setting port 7 to drive LED's (0xFF ==1111 1111)
  P7OUT = 0x00;                                             // Set all LED's on port 7 to start all off
//************************************ Set up clock [0] 
  TA2CTL |= TASSEL__ACLK | MC_2;                            // Setting Timer_A to ACLK(TASSEL_1) to continuous mode(MC_2)

//*********************************** Set timer interrupt enable [1] 
  TA2CCTL0 |= CCIE;                                          // Capture/compare interrupt enable #0
  TA2CCTL1 |= CCIE;                                          // Capture/compare interrupt enable #1

//*********************************** Set the timer count IR value [2] 
  TA2CCR0 = 10000;                                           // Timer0_A3 Capture/Compare @ 10000 counts
  TA2CCR1 = 1000;                                            // TA0IV_1 Capture/Compare @ 1000 counts

   while (1)                                                // poll in while loop until a key press
   {
      if ((timer_check=getchar()) != EOF)
     {
      break;                                                 // break out of loop if a key is pressed
     }
    ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS,0, 1<<15, CTL_TIMEOUT_DELAY, 1000); // wait in loop 
   }
  reset(0,ERR_SRC_CMD,CMD_ERR_RESET,0);                     // reset clock registers 
  return 0;
}

// ********************************* Timer_A0 interrupt code ***********************************************************************************
//NOTE: to use interrupts you usually need need to enable global interrupts using _ENIR();
//      This is not necessary when using ARClib code as the enable is called in ARClib
//NOTE: For P1IV you still need to enable P1IE registers 
//**********************************************************************************************************************************************
void Timer_A2_A0(void)__interrupt[TIMER2_A0_VECTOR]{     // Timer A0 interrupt service routine TA0IV_TA0IFG. 
  P7OUT^=BIT0; // toggle LEDs when IR is called
}

void Timer_A2_A1(void)__interrupt[TIMER2_A1_VECTOR]{     // Timer A0 interrupt service routine for capture comp 1 and 2
        P7OUT^=BIT1; // light LEDs
}

//******************************************* Using the SD card *******************************************************************************

int SD_write(char **argv,unsigned short argc){
char buff[512];
int mmcReturnValue, result , i;

  mmcInit_msp();  // Sets up the interface for the card
  mmc_pins_on();  //Sets up MSP to talk to SD card
  mmcReturnValue=mmcInit_card();

  if (mmcReturnValue==MMC_SUCCESS){ // check good initialization 
    printf("\rCard initalized Sucessfully\r\n");
  }
  else{
    printf("Check SD card.\r\nInitalized failed.\r\n Error %i\r\n",mmcReturnValue);
  }

//populate buffer block
  for(i=1;i<=argc;i++) {// ignore 0 *argv input (argv[0]--> "SD_write" )
    strcat(buff,argv[i]); // appends chars from one string to another
    strcat(buff,"|");     // separates strings with | as strcat eats NULL
  }

//write to SD card
  result= mmcWriteBlock(100,buff); //(unsigned char*) casting my pointer(array) as a char 
 
  if (result>=0){ // check SD write 
  printf("SD card write success.\r\n");
  }
  else{
    printf("SD card write failed.\r\nError %i\r\n",result);
  }
  return 0;
}

int SD_read(char **argv,unsigned short argc){
  #define ASCIIOUT_STR  "%c "
 char buffer[512];
 int resp , i;
  
  //read from SD card
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
        return 0;
}

//******************************************************** Sending an I2C packet
int send_I2C(char **argv,unsigned short argc){
  unsigned short addr, cmd_id, resp, payload_count=argc-2, i;
  unsigned char packet[4+BUS_I2C_HDR_LEN+BUS_I2C_CRC_LEN], *payload;

  if (argc < 2) {
    printf("Not enough arguments.\n");
    return 0;
  }

  // Get integer value of address from passed string
  addr=strtoul(argv[1], NULL, 0);
  // Get integer value of I2C command from passed string
  cmd_id=strtoul(argv[2], NULL, 0);

  // Initialize packet with source I2C address, and I2C command
  payload=BUS_cmd_init(packet, cmd_id);

  for (i=payload_count; i > 0; --i) {
    payload[payload_count-i]=strtoul(argv[payload_count-i+2], NULL, 0); 
  }

  // Transmit packet to addr. Payload is zero length. 
  resp=BUS_cmd_tx(addr, packet, payload_count, 0);
  if (resp == 0) {
    printf("Valid response\n");
  }
  else {
    printf("Error: %d\n", resp);
  }
  /*initI2C(4,1,0);
  // I2C_tx expects a char* for transmitted data, not an integer
  resp=i2c_tx(addr, (unsigned char*)&cmd_id, 1);
  if (resp < 0){
    printf("I2C transaction error.\n");
  }*/

  return 0;
}
int addr_cmd(char **argv,unsigned short argc){
    if( !strcmp(argv[1],"LEDL")){
     sys_addr=BUS_ADDR_LEDL;
     }
    else if(!strcmp(argv[1],"ACDS")){
     sys_addr=BUS_ADDR_ACDS;
     }
    else  if(!strcmp(argv[1],"COMM")){
     sys_addr=BUS_ADDR_COMM;
     }
    else if(!strcmp(argv[1],"IMG")){
     sys_addr=BUS_ADDR_IMG;
     }
    else if(!strcmp(argv[1],"CDH")){
     sys_addr=BUS_ADDR_CDH;
     }
    else{
      printf("Not a valid address name, valid addresses are:\r\n\tLEDL, ACDS, COMM, IMG, CDH\r\n\t");
    }
  return 0;
}
  int addr_get(char **argv,unsigned short argc){
      printf("current address is %#02x\r\n\t",sys_addr);
    return 0;
}
//TODO This will be the command to turn the second board on
int turnOnCMD(char **argv,unsigned short argc){

     //int err=Bus_cmd_tx((int)argv[1],CMD_SUB_ON,len,0);
    
    return 0;
}
//enum{BUS_ADDR_LEDL=0x11,BUS_ADDR_ACDS=0x12,BUS_ADDR_COMM=0x13,BUS_ADDR_IMG=0x14,BUS_ADDR_CDH=0x15,BUS_ADDR_GC=0};

int what_addr_CMD(char **argv,unsigned short argc){
  char x; 
  BUS_set_OA(sys_addr);

  x = BUS_get_OA();
  printf("Own address is %u \r\n", (unsigned int) x);

  if (x!=sys_addr)
  {
    printf("Wrong address, setting address\r\n");
    BUS_set_OA(sys_addr);
  }
  return 0;
}



//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"ex","[arg1] [arg2] ...\r\n\t""Example command to show how arguments are passed",example_command},
                   {"timer_IR","[time]...\r\n\tExample command to show how the timer can be used as an interupt",example_timer_IR},
                   {"SD_write","Writes given args to the SD card",SD_write},
                   {"SD_read","",SD_read},
                   {"send_I2C","Sends I2C command to subsystem",send_I2C},
                   {"Change_Address","Does what you think it does",addr_cmd},
                   {"Get_Address","Does what you think it does",addr_get},
                   {"Turn_On","turns on selected board",turnOnCMD},
                   {"what_addr","turns on selected board",what_addr_CMD},


                   ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS,
                   //end of list
                   {NULL,NULL,NULL}};

