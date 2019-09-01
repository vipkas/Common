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


#include "IoTStarterKit_WiFi.h"
#include "iotstarterkit_api.h"
#include "util/log.h"
#include "netio/sock.h"
#include "netio/sock_Arduino.h"

IoTMakers::IoTMakers(){
}

int IoTMakers::init(const char *deviceId, const char *devicePasswd, const char *gatewayId) // Temporarily
{
	im_init((char*)deviceId, (char*)devicePasswd, (char*)gatewayId);
	return 0;
}
int IoTMakers::begin(const char* ssid, const char* pass)
{
	return im_connect_AP((char*) ssid, (char*) pass);
}

int IoTMakers::getFreeRAM()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

char IoTMakers::isServerDisconnected()
{
	return im_isServerDisconnected();
}

int IoTMakers::connect()
{
	return im_connect();
}
int IoTMakers::disconnect()
{
	return im_sock_disconnect();
}

void IoTMakers::set_numdata_handler(IMCbTagidNumDataHndl cb_proc)
{
	im_set_numdata_handler(cb_proc);
}
void IoTMakers::set_strdata_handler(IMCbTagidStrDataHndl cb_proc) // Temporarily First Handler
{
	im_set_strdata_handler(cb_proc);
}
void IoTMakers::set_dataresp_handler(IMCbDataRespHndl cb_proc)
{
	im_set_dataresp_handler(cb_proc);
}
void IoTMakers::set_error_handler(IMCbErrorHndl cb_proc)
{
	im_set_error_handler(cb_proc);
}

int IoTMakers::auth_device()
{
	return im_auth_device();
}

int IoTMakers::send_numdata(const char* tagid, double val)
{
	return im_send_numdata((char*)tagid, val); //  trxid == 0
}

int IoTMakers::send_strdata(const char* tagid, char* val)
{
 	return im_send_strdata((char*)tagid, val);
}

void IoTMakers::loop()
{
	im_loop();
}
