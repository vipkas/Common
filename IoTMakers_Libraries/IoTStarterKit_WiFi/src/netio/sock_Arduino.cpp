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

#include <Arduino.h>
#include "util/log.h"
#include "netio/sock_Arduino.h"
#include "netio/sock.h"
#include "WiFi_Shield.h"

#define w_rxPin 8
#define w_txPin 9

WiFi_Shield wifiShield(w_rxPin, w_txPin);

int im_connect_AP(const char* ssid, const char* pass)// Connect AP
{
	for(int i=0; i<5; i++)
	{
		if(wifiShield.begin(ssid, 2, pass) == wifiShield.STATE_CONNECTED_AP)
		{
			return 0;
		}
	}
	return -1;
}

int im_sock_connect(char *ip, unsigned short port) // Connect Server
{
	wifiShield.disconnectServer();
	if(wifiShield.connect(ip, port, 0) == wifiShield.STATE_CONNECTED_SERVER)
	{
		delay(1000);
		return 0;
	}
	return -1;
}


int im_sock_disconnect()
{
	wifiShield.disconnectServer();
}

int im_sock_connected() // Real Function
{
	if(wifiShield.find("DISCONNECTED SOCKET")){
		return 0;
	}
	else{
		return 1; // For Debug
	}
}

int im_sock_available()
{
	return (wifiShield.available()>0)?1:0;
}

void im_sock_flush()
{
	while(wifiShield.available())wifiShield.read();
}

#define READ_CHUNK_LEN	16

int im_sock_send(char *buff, int len) // Temporarily
{
	int lenSent = 0;

	wifiShield.print(buff,len);
	lenSent = len;

	if ( len != lenSent ){
		Serial.println(F("sock fail write"));
		im_log_hex((unsigned char*)buff, len);
		return -1;
	}

	delayMicroseconds(100); // Time Temporarily

	return lenSent;
}

int im_sock_recv(char *buff, int len)
{
	int lenRead = 0;
	#ifdef CIPMODE_0
	char* IPDStr = "\r\n+IPD,";
	static char cFindIPDState = 0;
	static char cFindCount = 0;
	static char lengthData[4];
	static int receivedDataLen = 0, readCount = 0;
	#endif

	for (lenRead=0; lenRead<len; )
	{
		if ( !wifiShield.available() )
		{
			uint32_t previousMillis = millis();
			while(!wifiShield.available()) {
				uint32_t currentMillis = millis();
				//if(currentMillis - previousMillis >= ((int32_t) IM_SOCKET_TIMEOUT * 1000)){ // For Debug
				if(currentMillis - previousMillis >= ((int32_t) IM_SOCKET_TIMEOUT * 10000)){
					Serial.println(F("sock read timeout"));
					return -1;
				}
				//delay(100); // For Debug
			}
		}
		char ch =  wifiShield.read();
		//Serial.print((unsigned char)ch,HEX); Serial.print(":");
		delayMicroseconds(50);
		//Serial.print((char)ch); Serial.print(" ");
		#ifdef CIPMODE_0
		if(cFindIPDState == 1)
		{
			buff[lenRead] = ch; // Need It
			readCount++;
			//Serial.print("buff:");
			//Serial.println((unsigned char)buff[lenRead], HEX);
			if(readCount == receivedDataLen)
			{
				cFindIPDState = 0;
				readCount = 0;
				cFindCount = 0;
				receivedDataLen = 0;
			}
			lenRead++;
		}

		if(cFindCount >= 7)
		{
			//Serial.println("find +IPD,");
			if(ch == ':')
			{
				receivedDataLen = 0;
				int digit = 1;
				for(int i=cFindCount-7-1; i>=0; i--)
				{
					receivedDataLen += digit * lengthData[i];
					digit *= 10;
					//Serial.print("received Data len : ");
					//Serial.print(receivedDataLen);
					//Serial.print(" length data : ");
					//Serial.println((int)lengthData[i]);
				}
				cFindIPDState = 1;
				cFindCount = 0;
			}
			else
			{
				lengthData[cFindCount - 7] = ch - 0x30;
				cFindCount++;
			}
		}
		else if(ch == IPDStr[cFindCount])
		{
			cFindCount++;
		}
		else
		{
			cFindCount = 0;
		}
		#else
		buff[lenRead++] = ch; // Need It
		#endif//ifdef CIPMODE_0

	}

	return lenRead;
}

int im_imcb_default_control_request_handler_4_strdata(char *tagid, char *val){

}

void im_disableReceive()
{
}

void im_enableReceive()
{
}

void im_debug()
{
	wifiShield.RxFlush();
}

void SWReset()
{
}
#endif	// ARDUINO
