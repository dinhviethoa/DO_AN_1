#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include <avr/interrupt.h>

#include "SIM900TCPClient.h"
#include "softwareuart.h"
#include "USART_RS232_H_file.h"
#include "Timer1.h"
#include "ADC_H.h"

#define MQTT_PROTOCOL_LEVEL		4

// define battery capacity
#define LOW_BAT  13


#define Buffer_Size 150
#define degrees_buffer_size 20

// define server key word 
#define AIO_SERVER		"demo.thingsboard.io"
#define AIO_SERVER_PORT		"1883"
#define AIO_USERNAME		"dinhviethoa"
#define AIO_KEY			"dinhviethoa2706"
#define RESPONSE_TRUE        "{\"UNLOCK\":true}"
#define TOPIC  "v1/devices/me/telemetry" 
#define SUBTOPIC  "v1/devices/me/attributes"
#define  clientID  "ABCDEF"


char Latitude_Buffer[15],Longitude_Buffer[15],Time_Buffer[15],Altitude_Buffer[8];

char degrees_buffer[degrees_buffer_size];
char GGA_Buffer[Buffer_Size];
uint8_t GGA_Pointers[20];
char GGA_CODE[3];
volatile uint16_t GGA_Index, CommaCounter;

char mess[100];


const char GPS_commend[16] = {0xB5,0x62,0x02,0x41,0x08,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x4D,0x3B};
volatile bool required = false;

uint16_t adc_value;
float bat_capacity;
float R2 = 1000.0;
float R1 = 10000.0;

void getstringGPS()
{
	uint8_t IsItGGAString = 0;
	while(IsItGGAString == 0)
	{
		while(!(SWseriale_available()));
		char received_char = SWseriale_read();
		if (received_char == 'G')
		{
			while(!(SWseriale_available()));
			received_char = SWseriale_read();
			if (received_char == 'G')
			{
				while(!(SWseriale_available()));
				received_char = SWseriale_read();
				if (received_char == 'A')
				{
					IsItGGAString = 1;
					do
					{
						while(!(SWseriale_available()));
						received_char = SWseriale_read();
						if(received_char == ',' ) GGA_Pointers[CommaCounter++] = GGA_Index;
						GGA_Buffer[GGA_Index++] = received_char;
					} while (GGA_Buffer[GGA_Index-1] != '$');
					GGA_Index = 0;
					CommaCounter = 0;
				}
				else IsItGGAString = 0;
			}
			else IsItGGAString = 0;
		}
		else IsItGGAString = 0;
	}
}
bool convert_to_degrees(char *raw){
	
	double value;
	float decimal_value,temp;
	
	int32_t degrees;
	
	float position;
	value = atof(raw);
	decimal_value = (value/100);
	degrees       = (int)(decimal_value);
	temp          = (decimal_value - (int)decimal_value)/0.6;
	position      = (float)degrees + temp;
	if (position == 0)
	return false;
	else
	{
		dtostrf(position, 6, 5, degrees_buffer);
		return true;
	}
	
}
bool get_latitude(uint16_t lat_pointer){
	uint8_t lat_index;
	uint8_t index = lat_pointer+1;
	lat_index=0;
	for(;GGA_Buffer[index]!=',';index++){
		Latitude_Buffer[lat_index]= GGA_Buffer[index];
		lat_index++;
	}
	
	Latitude_Buffer[lat_index++] = GGA_Buffer[index++];
	Latitude_Buffer[lat_index]   = GGA_Buffer[index];
	return convert_to_degrees(Latitude_Buffer);
}

bool get_longitude(uint16_t long_pointer){
	uint8_t long_index;
	uint8_t index = long_pointer+1;
	long_index=0;
	for( ; GGA_Buffer[index]!=','; index++){
		Longitude_Buffer[long_index]= GGA_Buffer[index];
		long_index++;
	}
	
	Longitude_Buffer[long_index++] = GGA_Buffer[index++];
	Longitude_Buffer[long_index]   = GGA_Buffer[index];
	return convert_to_degrees(Longitude_Buffer);
}
void GPS_sleep()
{
	SWseriale_write(GPS_commend, 16);
}
void GPS_wake_up()
{
	SWseriale_write("WAKE_UP",7);
}

void getmess(uint8_t t)
{
	if(t == 0)
	{
		strcat(mess, "{\"latitude\":");
		strcat(mess, degrees_buffer);
	}
	else
		{
		strcat(mess, ",\"longitude\":");
		strcat(mess, degrees_buffer);
		strcat(mess,"}");
		
	}
}
void measure_bat()
{
	adc_value = adc_read(0);
	bat_capacity = (float)adc_value*(float)5/(float)1024;
	bat_capacity = bat_capacity/(R2/(R1 + R2));
}

ISR(INT0_vect)
{
	cli();
	required = true;
	sei();
}
int main()
{
	DDRC |= 0x3C;
	PORTC &= ~(1 << PORTC4) & ~(1 << PORTC2);
	PORTC |= (1 << PORTC3);
	DDRD |= 0x04;
	PORTD |= (1 << PORTD2);
	EICRA |= (0b10 << ISC00);
	EIMSK |= (1 << INT0);
	
	memset(GGA_Buffer, 0, Buffer_Size);
	memset(degrees_buffer,0,degrees_buffer_size);
	
	GGA_Index = 0;
	CommaCounter = 0;
	bool receivered = false;
	uint32_t first_time;
	uint8_t time = 0;
	
	timer1_init();
	USART_Init();
	SWseriale_begin();
	
	sei();
	
	SIM_wake_up();
	SIM_reset();
	SIM_sleep_mode();
	
	first_time = second();
	
	while(1)
	{
		if ((delay(first_time, 120)) || (receivered == false))
		{
			GPS_wake_up();
			_delay_ms(500);
			memset(mess,0,100);
			getstringGPS();
			if(!(get_latitude(GGA_Pointers[1])))
			receivered = false;
			else receivered = true;
			getmess(0);
			memset(degrees_buffer,0,degrees_buffer_size);
			get_longitude(GGA_Pointers[3]);
			getmess(1);
			memset(degrees_buffer,0,degrees_buffer_size);
			GPS_sleep();
			_delay_ms(1000);
			if (receivered == true)
			{
				SIM_wake_up();
				SIM_reset();
				while(!SIM_Start());
				TCPClient_Connect();
				MQTT_Start();
				do 
				{
					_delay_ms(1000);
				} while ((time++) < 3);
				time = 0;
				TCPClient_Start_MQTT(AIO_USERNAME, AIO_KEY, clientID, AIO_SERVER, AIO_SERVER_PORT);
				TCPClient_Send_Topic(TOPIC);
				TCPClient_Send_Mess(mess);
				TCPClient_Send_Pub();
				MQTT_Stop();
				SIM_sleep();
			}
		}
		if (required == true)
		{
			PORTC |= (1 << PORTC2);
			required = false;
			SIM_wake_up();
			SIM_reset();
			while(!SIM_Start());
			time = 0;
			TCPClient_Connect();
			MQTT_Start();
			do
			{
				_delay_ms(1000);
			} while ((time++) < 3);
			TCPClient_Start_MQTT(AIO_USERNAME, AIO_KEY, clientID, AIO_SERVER, AIO_SERVER_PORT);
			TCPClient_Send_Topic(TOPIC);
			TCPClient_Send_Mess("{\"required_UNLOCK\":yes}");
			TCPClient_Send_Pub();
			TCPClient_Send_Subtopic(SUBTOPIC);
			TCPClient_Send_Sub();
			if(WaitCheckForReadPacket(RESPONSE_TRUE))
			{
				PORTC &= ~(1 << PORTC3);
				PORTC |= (1 << PORTC4);
			}
			TCPClient_Send_Topic(TOPIC);
			TCPClient_Send_Mess("{\"required_UNLOCK\":no}");
			TCPClient_Send_Pub();
			MQTT_Stop();
			SIM_sleep();
			PORTC &= ~(1 << PORTC2);
		}
	}
}
