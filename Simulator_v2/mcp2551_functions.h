#pragma once


#ifndef mcp_2551_functions_h
#define mcp_2551_functions_h

#include "mcp_can_dfs.h"
#include "mcp_can.h"
#include "spi.h"


#define spi_readwrite SPI.transfer
#define spi_read() spi_readwrite(0x00)

    // chip select pin for the shield
#define SPI_CS_PIN 9



    // changes all bytes in a register
INT8U setRegister(const INT8U address, const INT8U value)
{
    digitalWrite(SPI_CS_PIN, LOW);
    SPI.transfer(MCP_WRITE);
    SPI.transfer(address);
    SPI.transfer(value);
    digitalWrite(SPI_CS_PIN, HIGH);
}

    // reads a selected register
INT8U readRegister(const INT8U address)
{
    INT8U ret;

    digitalWrite(SPI_CS_PIN, LOW);
    SPI.transfer(MCP_READ);
    SPI.transfer(address);
    ret = SPI.transfer(0x00);
    digitalWrite(SPI_CS_PIN, HIGH);

    return ret;
}

INT8U readStatus()
{
    INT8U ret;
    digitalWrite(SPI_CS_PIN, LOW);
    SPI.transfer(MCP_READ_STATUS);
    ret = SPI.transfer(0x00);
    digitalWrite(SPI_CS_PIN, HIGH);
    return ret;
}

    // modifies selected register
void modifyRegister(const INT8U address, const INT8U mask, const INT8U data)
{
    digitalWrite(SPI_CS_PIN, LOW);
    SPI.transfer(MCP_BITMOD);
    SPI.transfer(address);
    SPI.transfer(mask);
    SPI.transfer(data);
    digitalWrite(SPI_CS_PIN, HIGH);
}

void setRegisterS(const INT8U address, const INT8U values[], const INT8U n)
{
    INT8U i;
    digitalWrite(SPI_CS_PIN, LOW);
    spi_readwrite(MCP_WRITE);
    spi_readwrite(address);
    for (i = 0; i < n; i++)
    {
        spi_readwrite(values[i]);
    }
    digitalWrite(SPI_CS_PIN, HIGH);
}

    // configures interrupt for send 
void configure_interrupts()
{
    // configures the interupts for receive (?)
    modifyRegister(MCP_CANINTE, MCP_TX_INT, 0xFF);
    //modifyRegister(MCP_CANINTE,0x1F,0xFF);
    Serial.print("Interrupt configured: CANINTE REGISTER VALUE = ");
    Serial.println(readRegister(MCP_CANINTE), BIN);

}


void readRegisterS(const INT8U address, INT8U values[], const INT8U n)
{
    INT8U i;
    digitalWrite(SPI_CS_PIN, LOW);
    spi_readwrite(MCP_READ);
    spi_readwrite(address);
    // mcp2515 has auto-increment of address-pointer
    for (i = 0; i < n; i++)
    {
        values[i] = spi_read();
    }
    digitalWrite(SPI_CS_PIN, HIGH);
}

//void send_interrupt() {
//
//    Serial.println("Message sent, Resetting interrupt flag");
//    Serial.print("CANINTF after interrupt = ");
//    byte jic;
//    jic = readRegister(MCP_CANINTF);
//    Serial.println(jic, BIN);
//    modifyRegister(MCP_CANINTF, 0x1C, 0x0);
//    Serial.print("CANINTF after reset = ");
//    jic = readRegister(MCP_CANINTF);
//    Serial.println(jic, BIN);
//    Serial.println("");
//
//     successfully clears interrupts with this code and exits the loop
//    modifyRegister(MCP_CANINTF, 0xFF, 0x0);
//}

void configure_timer2_interrupt()
{
    cli();
    TIMSK1 = 0x02;
    TCCR1B = 0;
    //TCCR1B |= (1 << CS12);      // slower prescaler for test purposes
    //TCCR1B |= (1 << CS10);      // 
    TCCR1B |= (1 << CS12);      // prescaler
    TCCR1B |= 1;                // prescaler
    TCCR1B |= (1 << WGM12);     // compare match limit
    TCCR1A = 0;
    //
    // starts the interrupts
    //OCR1A = 30000;
    OCR1A = 50000;
    sei();
}

#endif
