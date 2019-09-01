/*
 * GiGA IoTMakers version 2
 *
 *  Copyright (c) 2016 kt corp. All rights reserved.
 *
 *  This is a proprietary software of kt corp, and you may not use this file except in
 *  compliance with license agreement with kt corp. Any redistribution or use of this
 *  software, with or without modification shall be strictly prohibited without prior written
 *  approval of kt corp, and the copyright notice above does not evidence any actual or
 *  intended publication of such software.
 */

#if defined(ARDUINO)

#include "util/log.h"
#include "netio/sock_Arduino.h"
#include "netio/sock.h"

Client* g_client = NULL;

void im_sock_set_Arduino_client(Client* client)
{
	g_client = client;
}

int im_sock_connect(char *ip, unsigned short port) // Connect Server
{
	unsigned int flds[4];
	unsigned long ipaddr;

	//WowSerial.print(F("connecting "));WowSerial.print(ip);WowSerial.print(F(":"));WowSerial.println(port);

	if(sscanf(ip,"%u.%u.%u.%u",flds,flds+1,flds+2,flds+3)==4)
	{
		ipaddr = (*(flds+3)*0x1000000)+(*(flds+2)*0x10000)+(*(flds+1)*0x100)+*(flds+0);
	}

	if ( g_client->connect(ipaddr, port) != (1) )	{
		#ifdef _EN_DEBUG
		Serial.println(F("sock fail connect"));
		#endif //#ifdef _EN_DEBUG
		return -1;
	}
	return 0;
}

int im_sock_disconnect()
{
	g_client->stop();
}

int im_sock_connected() // Real Function
{
	if ( g_client->connected() ) {return 1;}
	#ifdef _EN_DEBUG
	Serial.println(F("Sock not connected"));
	#endif //#ifdef _EN_DEBUG
	return 0;
}

int im_sock_available()
{
	return ((int)g_client->available()>0)?1:0;
}

void im_sock_flush()
{
	g_client->flush();	//Temporarily
}

#define READ_CHUNK_LEN	16

int im_sock_send(char *buff, int len) // Temporarily
{
	int lenSent = 0;
	char ch[READ_CHUNK_LEN];
	char cbuff;
	unsigned char ucbuff;

	int readCount = len / READ_CHUNK_LEN;
	int readRemain = len % READ_CHUNK_LEN;

	if ( !im_sock_connected() )	{
		return -1;
	}

	for (int i=0; i<readCount; i++)		{
		lenSent += g_client->write((const uint8_t*)buff+lenSent, READ_CHUNK_LEN);
	}

	if ( readRemain > 0 )	{
		lenSent += g_client->write((const uint8_t*)buff+lenSent, readRemain);
	}
	if ( len != lenSent ){
		#ifdef _EN_DEBUG
		Serial.println(F("sock fail write"));
		#endif //#ifdef _EN_DEBUG
		g_client->stop();
		return -1;
	}

	return lenSent;
}

int im_sock_recv(char *buff, int len)
{
	int lenRead = 0;

	for (lenRead=0; lenRead<len; lenRead++ )
	{
		if ( !g_client->available() )
		{
			uint32_t previousMillis = millis();
			while(!g_client->available()) {
				uint32_t currentMillis = millis();
				if(currentMillis - previousMillis >= ((int32_t) IM_SOCKET_TIMEOUT * 1000)){
					Serial.println(F("sock read timeout"));
					return -1;
				}
				delay(10);
			}
		}

		char ch =  g_client->read();
		buff[lenRead] = ch;
	}
	return lenRead;
}
int im_imcb_default_control_request_handler_4_strdata(char *tagid, char *val){

}

void im_debug(char *d)
{
	Serial.println(d);
}
#endif	// ARDUINO
