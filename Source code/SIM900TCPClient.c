/*
 * SIM900TCPClient.c
 * http://www.electronicwings.com
 */

#include "SIM900TCPClient.h"		/* Include TCP Client header file */

int8_t Response_Status, CRLF_COUNT = 0;
volatile int16_t Counter = 0, pointer = 0;
uint32_t TimeOut = 0;

void Read_Response()
{
	char CRLF_BUF[2];
	char CRLF_FOUND;
	uint32_t TimeCount = 0, ResponseBufferLength;
	while(1)
	{
		if(TimeCount >= (DEFAULT_TIMEOUT+TimeOut))
		{
			CRLF_COUNT = 0; TimeOut = 0;
			Response_Status = SIM900_RESPONSE_TIMEOUT;
			return;
		}

		if(Response_Status == SIM900_RESPONSE_STARTING)
		{
			CRLF_FOUND = 0;
			memset(CRLF_BUF, 0, 2);
			Response_Status = SIM900_RESPONSE_WAITING;
		}
		ResponseBufferLength = strlen(RESPONSE_BUFFER);
		if (ResponseBufferLength)
		{
			_delay_ms(1);
			TimeCount++;
			if (ResponseBufferLength==strlen(RESPONSE_BUFFER))
			{
				for (uint16_t i=0;i<ResponseBufferLength;i++)
				{
					memmove(CRLF_BUF, CRLF_BUF + 1, 1);
					CRLF_BUF[1] = RESPONSE_BUFFER[i];
					if(!strncmp(CRLF_BUF, "\r\n", 2))
					{
						if(++CRLF_FOUND == (DEFAULT_CRLF_COUNT+CRLF_COUNT))
						{
							CRLF_COUNT = 0; TimeOut = 0;
							Response_Status = SIM900_RESPONSE_FINISHED;
							return;
						}
					}
				}
				CRLF_FOUND = 0;
			}
		}
		_delay_ms(1);
		TimeCount++;
	}
}

void TCPClient_Clear()
{
	memset(RESPONSE_BUFFER,0,DEFAULT_BUFFER_SIZE);
	Counter = 0;
}

void Start_Read_Response()
{
	Response_Status = SIM900_RESPONSE_STARTING;
	do {
		Read_Response();
	} while(Response_Status == SIM900_RESPONSE_WAITING);

}

void GetResponseBody(char* Response, uint16_t ResponseLength)
{

	uint16_t i = 12;
	char buffer[5];
	while(Response[i] != '\r')
	++i;

	strncpy(buffer, Response + 12, (i - 12));
	ResponseLength = atoi(buffer);

	i += 2;
	uint16_t tmp = strlen(Response) - i;
	memcpy(Response, Response + i, tmp);

	if(!strncmp(Response + tmp - 6, "\r\nOK\r\n", 6))
	memset(Response + tmp - 6, 0, i + 6);
}

bool WaitForExpectedResponse(char* ExpectedResponse)
{
	TCPClient_Clear();
	_delay_ms(200);
	Start_Read_Response();						/* First read response */
	if((Response_Status != SIM900_RESPONSE_TIMEOUT) && (strstr(RESPONSE_BUFFER, ExpectedResponse) != NULL))
	return true;							/* Return true for success */
	return false;								/* Else return false */
}

bool WaitCheckForReadPacket(char* response)
{
	uint32_t time = 0;
	uint8_t len_packet;
	TCPClient_Clear();
	_delay_ms(200);
	while(1)
	{
		len_packet = strlen(RESPONSE_BUFFER);
		_delay_ms(1);
		if(len_packet == strlen(RESPONSE_BUFFER))
		{
			if(strstr(RESPONSE_BUFFER,response)!= NULL)
			return true;
		}
		if ((time++) == (DEFAULT_TIMEOUT * 6L))
		return false;
	}
}
bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse)
{
	USART_SendString(ATCommand);				
	USART_SendString("\r\n");
	return WaitForExpectedResponse(ExpectedResponse);
}

bool SIM_Start()
{
	for (uint8_t i=0;i<5;i++)
	{
		if(SendATandExpectResponse("ATE0","OK")||SendATandExpectResponse("AT","OK"))
		return true;
	}
	return false;
}

bool MQTT_Start()
{
	USART_SendString("AT+CMQTTSTART\r\n");
	CRLF_COUNT = 2;
	return WaitForExpectedResponse("+CMQTTSTART: 0");
}

bool MQTT_Stop()
{
	USART_SendString("AT+CMQTTDISC=0,60\r\n");
	CRLF_COUNT = 2;
	if(!(WaitForExpectedResponse("+CMQTTDISC: 0,0")))
	return false;
	USART_SendString("AT+CMQTTREL=0\r\n");
	if(!(WaitForExpectedResponse("OK")))
	return false;
	USART_SendString("AT+CMQTTSTOP\r\n");
	CRLF_COUNT = 2;
	if(!(WaitForExpectedResponse("+CMQTTSTOP: 0")))
	return false;
	return true;
}

bool TCPClient_Connect()
{
	USART_SendString("AT+CREG?\r\n");
	if(!WaitForExpectedResponse("+CREG: 0,1"))
	return false;
	USART_SendString("AT+CGDCONT=1,\"IP\",\"v-internet\"\r\n");
	if(!WaitForExpectedResponse("OK"))
	return false;
	return true;
}

bool SIM_sleep_mode()
{
	USART_SendString("AT+CSCLK=1\r\n");
	return WaitForExpectedResponse("OK");	
}
bool SIM_sleep()
{
	PORTC &= ~(1 << PORTC5);
	_delay_ms(500);
}
void SIM_wake_up()
{
	PORTC |= (1 << PORTC5);
	_delay_ms(500);
}
void SIM_reset()
{
	uint8_t t = 0;
	USART_SendString("AT+CRESET\r\n");
	WaitForExpectedResponse("OK");
	do
	{
		_delay_ms(1000);
	}	while((t++) <= 15);
}

uint8_t TCPClient_Start_MQTT(char* USERNAME, char* PASSWORD, char* client_id, char* Domain, char* Port)
{
	USART_SendString("AT+CMQTTACCQ=0,\"");
	USART_SendString(client_id);
	USART_SendString("\"\r\n");
	WaitForExpectedResponse("OK");
	USART_SendString("AT+CMQTTCONNECT=0,\"tcp://");
	USART_SendString(Domain);
	USART_TxChar(':');
	USART_SendString(Port);
	USART_SendString("\",60,1,\"");
	USART_SendString(USERNAME);
	USART_SendString("\",\"");
	USART_SendString(PASSWORD);
	USART_SendString("\"\r\n");
	CRLF_COUNT = 2;
	if(!WaitForExpectedResponse("+CMQTTCONNECT: 0,0"))
	{
		if(Response_Status == SIM900_RESPONSE_TIMEOUT)
		return SIM900_RESPONSE_TIMEOUT;
		return SIM900_RESPONSE_ERROR;
	}
	return SIM900_RESPONSE_FINISHED;
}

uint8_t TCPClient_Send_Pub()
{
	CRLF_COUNT = 2;
	USART_SendString("AT+CMQTTPUB=0,1,60\r\n");
	if(!WaitForExpectedResponse("+CMQTTPUB: 0,0"))
	{
		if(Response_Status == SIM900_RESPONSE_TIMEOUT)
		return SIM900_RESPONSE_TIMEOUT;
		return SIM900_RESPONSE_ERROR;
	}
	return SIM900_RESPONSE_FINISHED;
}

uint8_t TCPClient_Send_Sub()
{
	CRLF_COUNT = 2;
	USART_SendString("AT+CMQTTSUB=0\r\n");
	if(!WaitForExpectedResponse("+CMQTTSUB: 0,0"))
	{
		if(Response_Status == SIM900_RESPONSE_TIMEOUT)
		return SIM900_RESPONSE_TIMEOUT;
		return SIM900_RESPONSE_ERROR;
	}
	return SIM900_RESPONSE_FINISHED;
}

uint8_t TCPClient_Send_Topic(char *topic)
{
	char buf[2];
	uint8_t len_topic = strlen(topic);
	itoa(len_topic, buf, 10);
	USART_SendString("AT+CMQTTTOPIC=0,");
    USART_SendString(buf);
	USART_SendString("\r\n");
	CRLF_COUNT = -1;
	WaitForExpectedResponse(">");
	for (uint16_t i = 0; i < len_topic; i++)
	USART_TxChar(topic[i]);
	CRLF_COUNT = -1;
	if(!WaitForExpectedResponse("OK"))
	{
		if(Response_Status == SIM900_RESPONSE_TIMEOUT)
		return SIM900_RESPONSE_TIMEOUT;
		return SIM900_RESPONSE_ERROR;
	}
	return SIM900_RESPONSE_FINISHED;
}

uint8_t TCPClient_Send_Mess(char* mess)
{
	char buf[2];
	uint8_t len_mess = strlen(mess);
	itoa(len_mess, buf, 10);
	USART_SendString("AT+CMQTTPAYLOAD=0,");
	USART_SendString(buf);
	USART_SendString("\r\n");
	CRLF_COUNT = -1;
	WaitForExpectedResponse(">");
	for (uint16_t i = 0; i < len_mess; i++)
	USART_TxChar(mess[i]);
	if(!WaitForExpectedResponse("OK"))
	{
		if(Response_Status == SIM900_RESPONSE_TIMEOUT)
		return SIM900_RESPONSE_TIMEOUT;
		return SIM900_RESPONSE_ERROR;
	}
	return SIM900_RESPONSE_FINISHED;
}

uint8_t TCPClient_Send_Subtopic(char* subtopic)
{
	char buf[2];
	uint8_t len_subtopic = strlen(subtopic);
	itoa(len_subtopic, buf, 10);
	USART_SendString("AT+CMQTTSUBTOPIC=0,");
	USART_SendString(buf);
	USART_SendString(",0\r\n");
	CRLF_COUNT = -1;
	WaitForExpectedResponse(">");
	for (uint16_t i = 0; i < len_subtopic; i++)
	USART_TxChar(subtopic[i]);
	if(!WaitForExpectedResponse("OK"))
	{
		if(Response_Status == SIM900_RESPONSE_TIMEOUT)
		return SIM900_RESPONSE_TIMEOUT;
		return SIM900_RESPONSE_ERROR;
	}
	return SIM900_RESPONSE_FINISHED;
}

ISR (USART_RX_vect)
{
	cli();
	RESPONSE_BUFFER[Counter] = UDR0;
	Counter++;
	if(Counter == DEFAULT_BUFFER_SIZE){
		Counter = 0; pointer = 0;
	}
	sei();
}
