/*
 * SIM900TCPClient.h
 * http://www.electronicwings.com
 */

#ifndef SIM900TCPCLIENT_			/* Define library H file if not defined */
#define SIM900TCPCLIENT_

#define F_CPU 16000000UL				/* Define CPU Frequency e.g. here its 8MHz */
#include <avr/io.h>					/* Include AVR std. library file */
#include <util/delay.h>				/* Include Delay header file */
#include <stdbool.h>				/* Include standard boolean library */
#include <string.h>					/* Include string library */
#include <stdio.h>					/* Include standard library */
#include <stdlib.h>					/* Include standard library */
#include <avr/interrupt.h>			/* Include avr interrupt header file */
#include "USART_RS232_H_file.h"		/* Include USART header file */


#define DEFAULT_BUFFER_SIZE		200
#define DEFAULT_TIMEOUT			10000
#define DEFAULT_CRLF_COUNT		2

enum SIM900_RESPONSE_STATUS {
	SIM900_RESPONSE_WAITING,
	SIM900_RESPONSE_FINISHED,
	SIM900_RESPONSE_TIMEOUT,
	SIM900_RESPONSE_BUFFER_FULL,
	SIM900_RESPONSE_STARTING,
	SIM900_RESPONSE_ERROR
};

char RESPONSE_BUFFER[DEFAULT_BUFFER_SIZE];

void Read_Response();
void TCPClient_Clear();
void Start_Read_Response();
void GetResponseBody(char* Response, uint16_t ResponseLength);
bool WaitForExpectedResponse(char* ExpectedResponse);
bool WaitCheckForReadPacket(char* response);
bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse);
bool SIM_Start();
bool MQTT_Start();
bool MQTT_Stop();
bool TCPClient_Connect();
uint8_t TCPClient_Start_MQTT(char* USERNAME, char* PASSWORD, char* client_id, char* Domain, char* Port);
uint8_t TCPClient_Send_Pub();
uint8_t TCPClient_Send_Sub();
uint8_t TCPClient_Send_Topic(char *topic);
uint8_t TCPClient_Send_Mess(char* mess);
uint8_t TCPClient_Send_Subtopic(char* subtopic);
bool SIM_sleep_mode();
bool SIM_sleep();
void SIM_wake_up();
void SIM_reset();

#endif