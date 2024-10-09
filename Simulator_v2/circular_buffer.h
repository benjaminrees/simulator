#pragma once

#ifndef _circular_buffer_H_
#define _circular_buffer_H_

#define MAX_QUEUE 8
#define SIZE 13
#define MAX_ARRAY 100

#define PRINT Serial.print
#define PRINTL Serial.println

#include "mcp2551_functions.h"
#include "mcp_can_dfs.h"

class circular_buffer {
protected:

	// pointer to 2d matrix which contains the data
	INT8U** queue;

	// head, tail and length
	// length can probably be removed (eventually)
	uint8_t head;
	int length = 0;
	uint8_t tail = 0;
	uint8_t buffer1_previous_tail;
	uint8_t buffer2_previous_tail;
	uint8_t buffer3_previous_tail;

	unsigned long buffer1_time_loaded;
	unsigned long buffer2_time_loaded;
	unsigned long buffer3_time_loaded;

	unsigned int message_counter = 0;
	bool buffer1_message_sent = false;
	bool buffer2_message_sent = false;
	bool buffer3_message_sent = false;
	bool buffer_available;
	bool print_data = false;

	// 0: buffer 1. 1: buffer 2. 2: buffer 3
	bool buffer1_available = true;
	bool buffer2_available = true;
	bool buffer3_available = true;

	// bytes 0-3: identifier
	// Byte 4: 3 LSB's: data length code. 
	// Bytes 5-12:  message data

	//bool showBuffer = false;

	// arrays for collecting data

	//unsigned long times[105];
	//unsigned int service_times[105];
	//uint8_t buffer_length[200];

	// stores times message is loaded
	unsigned long loadtime_buffer[MAX_QUEUE];

	unsigned long total_times;
	unsigned long start_time, end_time;
	unsigned int drops = 0;
	unsigned int receive_counter = 0;
	unsigned long test_time;
	unsigned int arrivals = 0;

	unsigned long wait_times[100];


	
public:
	//circular_buffer();

	// hopefully this works from inside the function. Should only need to be done once
	byte* assignMemory()
	{
		queue = (INT8U**)malloc(MAX_QUEUE * sizeof(INT8U*));
		for (int i = 0; i < MAX_QUEUE; i++)
		{
			queue[i] = (INT8U*)malloc(SIZE * sizeof(INT8U));
		}

		for (int i = 0; i < MAX_QUEUE; i++)
		{
			for (int j = 0; j < SIZE; j++)
			{
				queue[i][j] = 0;
			}
		}
	}


	bool isEmpty()
	{
		if (length == 0)
			return true;
		else
			return false;
	}

	void printContents()
	{
		Serial.println("Buffer contents: ");
		int counter = 0;
		for (int i = 0; i < MAX_QUEUE; i++)
		{
			for (int j = 0; j < SIZE; j++)
			{
				Serial.print(queue[i][j],HEX);
				Serial.print(" ");

				//cout << queue[i][j] << " ";
				//if (queue[i][j] < 10) { cout << " "; }

				counter++;
				//cout << "i = " << i << " j = " << j << endl;
			}
			
			if (head == i)
			{
				Serial.print(" <-- HEAD ");
			}
			if (tail == i)
			{
				Serial.print(" <-- TAIL");
			}
			Serial.println("");
		}
		Serial.print((queue[head][11] << 8) + queue[head][12]);
		Serial.println("");
		Serial.println("");
		
	}



	void enqueue(byte* messagebuf) {
		arrivals++;
		unsigned long queue_instant = micros();
		//Serial.print(queue_instant);
		//Serial.print(",");

		//times[message_counter] = queue_instant;

		//buffer_length[message_counter] = length;

		if (length == MAX_QUEUE)
		{
			//Serial.println("Buffer full. Message being dropped.");
			drops++;
			return;
		}

		for (int i = 0; i < SIZE; i++)
		{
			queue[head][i] = messagebuf[i];

			//Serial.print(messagebuf[i], HEX);
			//Serial.print(" ");

		}
		loadtime_buffer[head] = queue_instant;
		//Serial.println("");


		queue[head][12] = arrivals;
		head = (head + 1) % (MAX_QUEUE);
		length++;
		//Serial.println(length);
	}

	void enqueue_dropFront(byte* messagebuf) {
		arrivals++;
		unsigned long queue_instant = micros();
		//Serial.print(queue_instant);
		//Serial.print(",");

		//times[message_counter] = queue_instant;

		//buffer_length[message_counter] = length;


		if (length == MAX_QUEUE)
		{
			tail = (tail + 1) % (MAX_QUEUE);
			drops++;
		}

		for (int i = 0; i < SIZE; i++)
		{
			queue[head][i] = messagebuf[i];

			//Serial.print(messagebuf[i], HEX);
			//Serial.print(" ");

		}
		loadtime_buffer[head] = queue_instant;
		//Serial.println("");


		queue[head][12] = message_counter;
		
		head = (head + 1) % (MAX_QUEUE);
		if (length == MAX_QUEUE)
			return;
		else
			length++;
		//printContents();
		//Serial.println(length);
	}

	// called internally, doesn't need any data
	bool isBufferAvailable()
	{
		return buffer_available;
	}

	void bufferIsEmpty()
	{
		buffer_available = true;
	}

	void interruptFunc_doubleBuffer()
	{
		//bufferIsEmpty();
		// successfully clears interrupts with this code and exits the loop
		byte CANINTF_result = readRegister(MCP_CANINTF);

		if ((CANINTF_result & 0x04) == 0x04)
		{
			buffer1_available = true;
			//Serial.print("Message received on buffer 1");
		}
		if ((CANINTF_result & 0x08) == 0x08)
		{
			buffer2_available = true;	
		}

		modifyRegister(MCP_CANINTF, 0xFF, 0x0);

	}

	void sendNextMessage_tripleBuffer()
	{
		// go into loop if either buffer becomes available
		// buffer only becomes available on send

		//unsigned long duration = micros() - start;

		byte test = readStatus();
		//Serial.println(test, BIN);
		if ((test & 4) == 0)
			buffer1_available = true;
		if ((test & 16) == 0)
			buffer2_available = true;
		if ((test & 64) == 0)
			buffer3_available = true;


	
		
		//unsigned long start = micros();

		//byte test = readRegister(MCP_TXB0CTRL);
		//if ((test & 8) == 0)
		//{
		//	buffer1_available = true;
		//	//Serial.println(timee);
		//}
		//test = readRegister(MCP_TXB1CTRL);
		//if ((test & 8) == 0)
		//{
		//	buffer2_available = true;
		//}
		//test = readRegister(MCP_TXB2CTRL);
		//if ((test & 8) == 0)
		//	buffer3_available = true;



		//Serial.println(duration);
		//

		//Serial.print(test, BIN);

		//if (buffer3_available == false)
		//	Serial.print("Finally");

		//if (buffer1_available == false)
		//	Serial.println("Buffer 1 not available");
		//else
		//	Serial.println("Buffer 1 Available");
		//if (buffer2_available == false)
		//	Serial.println("Buffer 2 not available");
		//else
		//	Serial.println("Buffer 2 Available");
		//if (buffer3_available == false)
		//	Serial.println("Buffer 3 not available");
		//else
		//	Serial.println("Buffer 3 Available");

		//Serial.println(length);


		if (buffer1_available)
		{
			if (buffer1_message_sent)
			{
				//queue[head][12] = receive_counter;
				//Serial.println("2");
				// loadtime_buffer comes from the queue
				unsigned long time_sent = micros();
				unsigned long service_time = time_sent - buffer1_time_loaded;
				//Serial.println(service_time);
				wait_times[receive_counter] = service_time;
				buffer1_message_sent = false;
				receive_counter++;
			}
			if (!isEmpty())
			{
				//Serial.println("buffer 1 sending");
				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer1_time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_time_loaded = loadtime_buffer[tail];
				buffer1_message_sent = true;
				buffer1_available = false;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}

		if (buffer2_available)
		{
			if (buffer2_message_sent)
			{
				// loadtime_buffer comes from the queue
				unsigned long time_sent = micros();
				unsigned long service_time = time_sent - buffer2_time_loaded;
				//Serial.println(service_time);
				wait_times[receive_counter] = service_time;
				buffer2_message_sent = false;
				receive_counter++;

				//Serial.print("Buffer 1: ");
				//Serial.println(service_time);
				//Serial.println(total_times);
			}
			if (!isEmpty())
			{
				//Serial.println("buffer 2 sending");
				setRegisterS(MCP_TXB1CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer2_time_sent = micros();
				modifyRegister(MCP_TXB1CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer2_time_loaded = loadtime_buffer[tail];
				buffer2_message_sent = true;
				buffer2_available = false;;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}

		if (buffer3_available)
		{
			if (buffer3_message_sent)
			{
				queue[head][12] = receive_counter;
				// loadtime_buffer comes from the queue
				unsigned long time_sent = micros();
				unsigned long service_time = time_sent - buffer3_time_loaded;
				//Serial.println(service_time);
				wait_times[receive_counter] = service_time;
				buffer3_message_sent = false;
				receive_counter++;

				//Serial.print("Buffer 2: ");
				//Serial.println(service_time);

				//Serial.println(total_times);
			}
			if (!isEmpty())
			{
				//Serial.println("buffer 3 sending");
				setRegisterS(MCP_TXB2CTRL + 1, &queue[tail][0], SIZE);

				//unsigned long buffer2_time_sent = micros();
				modifyRegister(MCP_TXB2CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer3_time_loaded = loadtime_buffer[tail];
				//Serial.println(buffer3_time_loaded);
				buffer3_message_sent = true;
				buffer3_available = false;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}


		if (receive_counter >= (MAX_ARRAY))
		{
			//Serial.print("go");
			end_time = micros();
			unsigned long time_taken = end_time - start_time;
			if (print_data)
			{
				Serial.print(wait_times[0]);
				Serial.print(",");
				Serial.print(time_taken);
				Serial.print(",");
				Serial.print(drops);
				Serial.print(",");
				Serial.println(arrivals);
				for (int i = 1; i < MAX_ARRAY; i++)
				{
					Serial.println(wait_times[i]);
				}
				clearBuffer();
			}
			print_data = false;
			start_time = micros();
			drops = 0;
			receive_counter = 0;
			arrivals = 0;
		}
		//}
		////unsigned long end_loop = micros() - start_loop;
		////Serial.println(end_loop);
	}

	void sendNextMessage_doubleBuffer()
	{
		// go into loop if either buffer becomes available
		// buffer only becomes available on send
		unsigned long start_loop = micros();

		byte test = readRegister(MCP_TXB0CTRL);
		if ((test & 8) == 0)
			buffer1_available = true;
		//Serial.println(test,BIN);

		test = readRegister(MCP_TXB1CTRL);
		if ((test & 8) == 0)
			buffer2_available = true;

		//Serial.print("Buffer 1 open: ");
		//Serial.println(buffer1_available);
		//Serial.print("Buffer 2 open: ");
		//Serial.println(buffer2_available);


			if (buffer1_available)
			{
				//queue[head][12] = receive_counter;
				if (buffer1_message_sent)
				{
					// loadtime_buffer comes from the queue
					unsigned long time_sent = micros();
					unsigned long service_time = time_sent - buffer1_time_loaded;
					wait_times[receive_counter] = service_time;
					buffer1_message_sent = false;
					receive_counter++;
				}
				if (!isEmpty())
				{
					//Serial.println("buffer 1 sending");
					setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
					//unsigned long buffer1_time_sent = micros();
					modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
					buffer1_message_sent = true;
					buffer1_available = false;
					buffer1_time_loaded = loadtime_buffer[tail];
					tail = (tail + 1) % (MAX_QUEUE);
					length--;
				}
			}

			if (buffer2_available)
			{
				if (buffer2_message_sent)
				{
					// loadtime_buffer comes from the queue
					unsigned long time_sent = micros();
					unsigned long service_time = time_sent - buffer2_time_loaded;
					wait_times[receive_counter] = service_time;
					buffer2_message_sent = false;
					receive_counter++;
				}
				if (!isEmpty())
				{
					//Serial.println("buffer 2 sending");
					setRegisterS(MCP_TXB1CTRL + 1, &queue[tail][0], SIZE);
					//unsigned long buffer2_time_sent = micros();
					modifyRegister(MCP_TXB1CTRL, MCP_TXB_TXREQ_M, 0xFF);
					buffer2_message_sent = true;
					buffer2_available = false;
					buffer2_time_loaded = loadtime_buffer[tail];
					tail = (tail + 1) % (MAX_QUEUE);
					length--;
				}
			}

			
			if (receive_counter >= (MAX_ARRAY))
			{
				//Serial.print("go");
				end_time = micros();
				unsigned long time_taken = end_time - start_time;
				if (print_data)
				{
					Serial.print(wait_times[0]);
					Serial.print(",");
					Serial.print(time_taken);
					Serial.print(",");
					Serial.print(drops);
					Serial.print(",");
					Serial.println(arrivals);
					for (int i = 1; i < MAX_ARRAY; i++)
					{
						Serial.println(wait_times[i]);
					}
					clearBuffer();
				}
				print_data = false;
				start_time = micros();
				drops = 0;
				receive_counter = 0;
				arrivals = 0;
			}
		//}
		unsigned long end_loop = micros() - start_loop;
		Serial.println(end_loop);
	}

	void sendNextMessage_datarecord()
	{
		//delay(10);
		//unsigned long start_loop = micros();
		if (buffer_available)
		{
			//Serial.print("Message counter: ");
			//Serial.println(message_counter);
			if (buffer1_message_sent)
			{
				unsigned long time_sent = micros();
				//unsigned long service_time = time_sent - time_loaded;
			//	total_times += service_time;
				buffer1_message_sent = false;
				receive_counter++;

			}
			// check if queue has data in it
			if (!isEmpty())
			{

				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				unsigned long time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_message_sent = true;
				buffer_available = false;
				// NEW PART
				// TAIL UPDATED WHEN LOADED, PREVIOUS TAIL RETRIEVED 
				// OR JUST USE AN INTEGER TO STORE IT
				// YES, THAT WOULD BE BETTER
				buffer1_previous_tail = tail;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
			if (receive_counter >= MAX_ARRAY)
			{
				
				end_time = micros();
				unsigned long time_taken = end_time - start_time;
				if (print_data)
				{
					total_times /= 100;
					Serial.print(total_times);
					Serial.print(",");
					Serial.print(time_taken);
					Serial.print(",");
					Serial.println(drops);
					print_data = false;
					buffer_available = true;
				}
				start_time = micros();
				total_times = 0;
				drops = 0;
				receive_counter = 0;
			}
		}
		//unsigned long end_loop = micros() - start_loop;
		//Serial.println(end_loop);
	}

	void sendNextMessage()
	{
		//delay(10);
		if (buffer_available)
		{
			if (buffer1_message_sent)
			{

				unsigned long time_loaded = loadtime_buffer[tail];
				unsigned long time_sent = micros();
				unsigned long service_time = time_sent - time_loaded;
				//service_times[queue[tail][12]] = service_time;
				buffer1_message_sent = false;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;

				//Serial.print("Time received:");
				//Serial.println(time_sent);

				//Serial.print(index);
				//Serial.print(",");
				//Serial.print("");
				
			}
			// check if queue has data in it
			if (!isEmpty())
			{

				//Serial.print("Time sent: ");
				//Serial.println(time_sent);

				//Serial.print(service_time);
				//Serial.println("");

				//Serial.print("Message counter: ");
				//Serial.println(queue[tail][12]);

				// sets bit to send message

				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				unsigned long time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_message_sent = true;
				buffer_available = false;
			}
			if (message_counter == MAX_ARRAY)
			{
				if (print_data)
				{
					for (int i = 0; i < MAX_ARRAY - 1; i++)
					{
						//Serial.print(service_times[i]);
						//Serial.print(",");
						//Serial.print(times[i]);
						//Serial.print(",");
						//Serial.print(buffer_length[i]);
						//Serial.print(",");
						//Serial.print(drops);
						//Serial.println(" ");
					}
					print_data = false;
				}
				message_counter = 0;
				Serial.print("Message counter: ");
				Serial.println(message_counter);
				
			}
		}
	}

	void sendNextMessage_singleBuffer()
	{
		byte test = readRegister(MCP_TXB0CTRL);
		if ((test & 8) == 0)
			buffer1_available = true;
		//Serial.println(test,BIN);

		if (buffer1_available)
		{
			if (buffer1_message_sent)
			{
				// loadtime_buffer comes from the queuel];
				unsigned long time_sent = micros();
				unsigned long service_time = time_sent - buffer1_time_loaded;
				wait_times[receive_counter] = service_time;
				buffer1_message_sent = false;
				receive_counter++;
			}
			if (!isEmpty())
			{
				//Serial.println("buffer 1 sending");
				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer1_time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_time_loaded = loadtime_buffer[tail];
				buffer1_message_sent = true;
				buffer1_available = false;
				buffer1_previous_tail = tail;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}
		if (receive_counter >= (MAX_ARRAY - 1))
		{
			//Serial.print("go");
			end_time = micros();
			unsigned long time_taken = end_time - start_time;
			if (print_data)
			{
				total_times /= 100;
				Serial.print(total_times);
				Serial.print(",");
				Serial.print(time_taken);
				Serial.print(",");
				Serial.println(drops);
				print_data = false;
				buffer_available = true;
			}
			start_time = micros();
			total_times = 0;
			drops = 0;
			receive_counter = 0;
		}
	}

	void sendNextMessage_singleBuffer_stdev()
	{
		byte test = readRegister(MCP_TXB0CTRL);
		if ((test & 8) == 0)
			buffer1_available = true;
		//Serial.println(test,BIN);

		if (buffer1_available)
		{
			if (buffer1_message_sent)
			{
				// loadtime_buffer comes from the queuel];
				unsigned long time_sent = micros();
				unsigned long service_time = time_sent - buffer1_time_loaded;
				total_times += service_time;
				wait_times[receive_counter] = service_time;
				buffer1_message_sent = false;
				receive_counter++;
			}
			if (!isEmpty())
			{
				//Serial.println("buffer 1 sending");
				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer1_time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_time_loaded = loadtime_buffer[tail];
				buffer1_message_sent = true;
				buffer1_available = false;
				buffer1_previous_tail = tail;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}
		if (receive_counter >= (MAX_ARRAY))
		{
			//Serial.print("go");
			end_time = micros();
			unsigned long time_taken = end_time - start_time;
			if (print_data)
			{
				Serial.print(wait_times[0]);
				Serial.print(",");
				Serial.print(time_taken);
				Serial.print(",");
				Serial.print(drops);
				Serial.print(",");
				Serial.println(arrivals);
				for (int i = 1; i < MAX_ARRAY; i++) 
				{
					Serial.println(wait_times[i]);
				}
				clearBuffer();
			}
			print_data = false;
			start_time = micros();
			drops = 0;
			receive_counter = 0;
			arrivals = 0;
		}
	}

	void interruptFunc_new()
	{

		//bufferIsEmpty();
		// successfully clears interrupts with this code and exits the loop
		unsigned long time_sent = micros();
		byte test = readStatus();
		//Serial.println(test, BIN);
		if ((test & 4) == 0)
		{
			buffer1_available = true;
			unsigned long service_time = time_sent - buffer1_time_loaded;
			wait_times[receive_counter] = service_time;
			receive_counter++;
		}
		if ((test & 16) == 0)
		{
		buffer2_available = true;
		unsigned long service_time = time_sent - buffer1_time_loaded;
		wait_times[receive_counter] = service_time;
		receive_counter++;
		}
		//if ((test & 64) == 0)
		//{
		//	buffer3_available = true;
		//	unsigned long time_sent = micros();
		//	unsigned long service_time = time_sent - buffer2_time_loaded;
		//	wait_times[receive_counter] = service_time;
		//	receive_counter++;
		//}
		//modifyRegister(MCP_CANINTF, 0xFF, 0x0);
		setRegister(MCP_CANINTF, 0);
	}

	void sendNextMessage_doubleBuffer_interrupt()
	{
		///Serial.println(receive_counter);
		byte test = readStatus();
		//Serial.println(test, BIN);
		if ((test & 4) == 0)
			buffer1_available = true;
		if ((test & 16) == 0)
			buffer2_available = true;

		//buffer1_available = true;
		//Serial.print("h");


		if (buffer1_available)
		{
			if (!isEmpty())
			{
				//Serial.println("buffer 1 sending");
				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer1_time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_time_loaded = loadtime_buffer[tail];
				buffer1_message_sent = true;
				buffer1_available = false;
				buffer1_previous_tail = tail;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}
		if (buffer2_available)
		{
			if (!isEmpty())
			{
				//Serial.println("buffer 2 sending");
				setRegisterS(MCP_TXB1CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer2_time_sent = micros();
				modifyRegister(MCP_TXB1CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer2_message_sent = true;
				buffer2_available = false;
				buffer2_time_loaded = loadtime_buffer[tail];
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}
		//Serial.println(receive_counter);
		if (receive_counter >= (MAX_ARRAY))
		{
			//Serial.print("go");
			end_time = micros();
			unsigned long time_taken = end_time - start_time;
			if (print_data)
			{
				Serial.print(wait_times[0]);
				Serial.print(",");
				Serial.print(time_taken);
				Serial.print(",");
				Serial.print(drops);
				Serial.print(",");
				Serial.println(arrivals);
				for (int i = 1; i < MAX_ARRAY; i++)
				{
					Serial.println(wait_times[i]);
				}
				clearBuffer();
			}
			print_data = false;
			start_time = micros();
			drops = 0;
			receive_counter = 0;
			arrivals = 0;
		}
	}

	void sendNextMessage_singleBuffer_interrupt()
	{
		//byte test = readRegister(MCP_TXB0CTRL);
		//if ((test & 8) == 0)
		//	buffer1_available = true;
		//Serial.println(test,BIN);

		if (buffer1_available)
		{
			if (!isEmpty())
			{
				//Serial.println("buffer 1 sending");
				setRegisterS(MCP_TXB0CTRL + 1, &queue[tail][0], SIZE);
				//unsigned long buffer1_time_sent = micros();
				modifyRegister(MCP_TXB0CTRL, MCP_TXB_TXREQ_M, 0xFF);
				buffer1_time_loaded = loadtime_buffer[tail];
				buffer1_message_sent = true;
				buffer1_available = false;
				buffer1_previous_tail = tail;
				tail = (tail + 1) % (MAX_QUEUE);
				length--;
			}
		}
		if (receive_counter >= (MAX_ARRAY))
		{
			//Serial.print("go");
			end_time = micros();
			unsigned long time_taken = end_time - start_time;
			if (print_data)
			{
				Serial.print(wait_times[0]);
				Serial.print(",");
				Serial.print(time_taken);
				Serial.print(",");
				Serial.print(drops);
				Serial.print(",");
				Serial.println(arrivals);
				for (int i = 1; i < MAX_ARRAY; i++)
				{
					Serial.println(wait_times[i]);
				}
				clearBuffer();
			}
			print_data = false;
			start_time = micros();
			drops = 0;
			receive_counter = 0;
			arrivals = 0;
		}
	}

	bool isBufferfull()
	{
		if (length == MAX_QUEUE)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void clearBuffer()
	{
		// not actually neccessary to return all the values in tbe buffer to zero
		// but for the sake of printing this should be fine
		for (int i = 0; i < MAX_QUEUE; i++)
		{
			for (int j = 0; j < SIZE; j++)
			{
				queue[i][j] = 0;
			}
		}
		resetBuffer();
	}

	void resetBuffer()
	{
		head = 0;
		tail = 0;
		length = 0;
	}

	void printRegisterContents()
	{
		Serial.print("Register contents: ");
		INT8U out[SIZE];
		readRegisterS(MCP_TXB0CTRL, out, 14);

		for (int i = 0; i < 14; i++)
		{
			Serial.print(out[i],HEX);
			Serial.print(" ");
		}
		Serial.println("");
		Serial.println("");
	}

	void interruptFunc()
	{
		bufferIsEmpty();
		// successfully clears interrupts with this code and exits the loop
		//byte CANTINT = readRegister(MCP_CANINTF);

		modifyRegister(MCP_CANINTF, 0xFF, 0x0);
		//Serial.println("intterrupted");




	}

	void timer1_interruptFunc()
	{
		print_data = true;


		//Serial.print("Buffer 1 Available: ");
		//Serial.println(buffer1_available);
		//Serial.print("buffer 1 sent: ");
		//Serial.println(buffer1_message_sent);
		//Serial.print("Buffer 2 Available: ");
		//Serial.println(buffer2_available);
		//Serial.print("buffer 2 sent: ");
		//Serial.println(buffer2_message_sent);
		//Serial.println("");

	}
	
	//void set_showBuffer(bool set_show)
	//{
	//	showBuffer = set_show;
	//}


};


#endif
