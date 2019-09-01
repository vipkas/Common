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


#ifndef IOTMAKERS_SOCK_ARDUINO_H
#define IOTMAKERS_SOCK_ARDUINO_H

#if defined(ARDUINO)
#include <Arduino.h>
//#include <Client.h>
#endif

#define SHIELD_CC3000	0

#ifdef __cplusplus
extern "C"
{
#endif
//int iFindIPDPlus;
//void im_sock_set_Arduino_client(Client* client);

int im_connect_AP(const char* ssid, const char* pass);
boolean connectWiFi(const char* ssid, const char* pass);
void im_disableReceive();
void im_enableReceive();
//char cGet4BitHEXtoASCII(char num);
//void cDecodeASCIIorHEX(char* _cpStr, unsigned char _cData);

#ifdef __cplusplus
}
#endif


#endif
