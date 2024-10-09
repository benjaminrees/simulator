/*
 Name:		Simulator_v1.ino
 Created:	1/15/2020 12:28:11 PM
 Author:	Benjamin
*/




#ifdef _SOME_OS_WITH_OTHER_TYPES_
typedef uint8 INT8U; // here uint8 - some unsigned integer type = 8 bits syze
#else
typedef unsigned char INT8U;
#endif

#include "mcp_can.h"
#include "mcp_can_dfs.h"
#include "messages.h"
#include "SPI.h"
#include "circular_buffer.h"



#define MCP_CHIP_SELECT 9
#define MCP_INTERRUPT 3

//#define MAX_QUEUE 8
#define SIZE 13


circular_buffer FIFO_buffer;
circular_buffer* buffer_ptr = &FIFO_buffer;
MCP_CAN CAN0(MCP_CHIP_SELECT);
poisson_message message1(700);
//poisson_message message143224);
//periodic_message message1(50);
//
//poisson_message message3(1000);
//uint8_t queue[SIZE] = { 170, 85, 0, 0, 15, 240, 15, 0, 170, 85, 204, 51, 8 };

unsigned int last_time = 0;

unsigned char stmp[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char stmp2[8] = { 0x55, 0x55, 55, 55, 55, 55, 55, 55 };
unsigned char stmp3[8] = { 170, 85, 204, 51, 15, 240, 15, 0 };
unsigned char stmp4[8] = { 85, 85, 85, 85, 85, 85, 85, 85 };

int interruptcount;

// indexes for circular buffer

// the setup function runs once when you press reset or power the board
void setup()
{
    FIFO_buffer.assignMemory();

    Serial.begin(9600);
    //attachInterrupt(digitalPinToInterrupt(2), send_interrupt, FALLING);
    SPI.begin();

    //message1.attach_CAN(&CAN0);
    message1.setMessage(4,stmp3);
    message1.setIdentifier(0x44,false);
    message1.seed();
    message1.attachBufferPointer(buffer_ptr);

    configure_timer2_interrupt();

    while (CAN_OK != CAN0.begin(CAN_100KBPS))              // init can bus : baudrate = 500k
// while (CAN_OK != CAN.begin(CAN_1000KBPS))  
    {

        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");

    configure_interrupts();

    INT8U result;
    modifyRegister(MCP_TXB0CTRL, 0xFF, 00);
    //modifyRegister(MCP_CANINTE, 0xFF, 00);
    //FIFO_buffer.set_showBuffer(false);
    FIFO_buffer.bufferIsEmpty();
    
}

// the loop function runs over and over again until power down or reset
void loop(){

    message1.runMessageGenerator();
   // message1.runMessage_dropFront();

    

    //FIFO_buffer.sendNextMessage_datarecord();
   //FIFO_buffer.sendNextMessage_singleBuffer();
   //FIFO_buffer.sendNextMessage_doubleBuffer();
   // FIFO_buffer.sendNextMessage_tripleBuffer();
    FIFO_buffer.sendNextMessage_singleBuffer_stdev();
   //FIFO_buffer.sendNextMessage_doubleBuffer_interrupt();

    }

void send_interrupt() {

    //FIFO_buffer.interruptFunc_new();
    //FIFO_buffer.interruptFunc_doubleBuffer();
    //Serial.println("interrupt");
}



ISR(TIMER1_COMPA_vect)
{
    //Serial.println("hello");
    FIFO_buffer.timer1_interruptFunc();
    //Serial.println("hello");
    
}