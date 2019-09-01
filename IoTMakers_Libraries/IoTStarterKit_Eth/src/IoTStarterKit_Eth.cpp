/*
 * GiGA IoTStarterKit_Eth version 2
 *
 *  Copyright (c) 2016 kt corp. All rights reserved.
 *
 *  This is a proprietary software of kt corp, and you may not use this file except in
 *  compliance with license agreement with kt corp. Any redistribution or use of this
 *  software, with or without modification shall be strictly prohibited without prior written
 *  approval of kt corp, and the copyright notice above does not evidence any actual or
 *  intended publication of such software.
 */


#include "IoTStarterKit_Eth.h"
#include "iotstarterkit_api.h"
#include "util/log.h"
#include "netio/sock.h"
#include "netio/sock_Arduino.h"


IoTStarterKit_Eth::IoTStarterKit_Eth(){
}


#if defined(ARDUINO)
int IoTStarterKit_Eth::init(const char *ip, int port, const char *deviceId, const char *devicePasswd, const char *gatewayId, Client& client)
{
	im_sock_set_Arduino_client(&client);
	im_init_with_ip((char*)ip, port, (char*)deviceId, (char*)devicePasswd, (char*)gatewayId);
	return 0;
}
int IoTStarterKit_Eth::init(const char *deviceId, const char *devicePasswd, const char *gatewayId, Client& client)
{
	im_sock_set_Arduino_client(&client);
	im_init((char*)deviceId, (char*)devicePasswd, (char*)gatewayId);
	return 0;
}
// User Define
int IoTStarterKit_Eth::init(const char *deviceId, const char *devicePasswd, const char *gatewayId) // Temporarily
{
	im_init((char*)deviceId, (char*)devicePasswd, (char*)gatewayId);
	return 0;
}
int IoTStarterKit_Eth::getFreeRAM()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#else
int IoTStarterKit_Eth::init(const char *ip, int port, const char *deviceId, const char *devicePasswd, const char *gatewayId)
{
	im_init_with_ip((char*)ip, port, (char*)deviceId, (char*)devicePasswd, (char*)gatewayId);
	return 0;
}
int IoTStarterKit_Eth::init(const char *deviceId, const char *devicePasswd, const char *gatewayId)
{
	im_init((char*)deviceId, (char*)devicePasswd, (char*)gatewayId);
	return 0;
}
#endif

int IoTStarterKit_Eth::connect()
{
	return im_connect();
}
int IoTStarterKit_Eth::disconnect()
{
	return im_sock_disconnect();
}

void IoTStarterKit_Eth::set_numdata_handler(IMCbTagidNumDataHndl cb_proc)
{
	im_set_numdata_handler(cb_proc);
}
void IoTStarterKit_Eth::set_strdata_handler(IMCbTagidStrDataHndl cb_proc) // Temporarily First Handler
{
	im_set_strdata_handler(cb_proc);
}
void IoTStarterKit_Eth::set_dataresp_handler(IMCbDataRespHndl cb_proc)
{
	im_set_dataresp_handler(cb_proc);
}
void IoTStarterKit_Eth::set_error_handler(IMCbErrorHndl cb_proc)
{
	im_set_error_handler(cb_proc);
}

int IoTStarterKit_Eth::auth_device()
{
	return im_auth_device();
}

int IoTStarterKit_Eth::send_numdata(const char* tagid, double val, long long trxid) // Temporarily
{
	return im_send_numdata((char*)tagid, val, trxid); //  trxid == 0
}

int IoTStarterKit_Eth::send_strdata(const char* tagid, char* val, long long trxid)
{
 	return im_send_strdata((char*)tagid, val, trxid);
}

void IoTStarterKit_Eth::loop()
{
	im_loop();
}
