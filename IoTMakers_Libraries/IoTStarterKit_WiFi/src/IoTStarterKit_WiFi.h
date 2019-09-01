/*
 * GiGA IoT Platform version 2.0
 *
 *  Copyright(c) 2016 kt corp. All rights reserved.
 *
 *  This is a proprietary software of kt corp, and you may not use this file except in
 *  compliance with license agreement with kt corp. Any redistribution or use of this
 *  software, with or without modification shall be strictly prohibited without prior written
 *  approval of kt corp, and the copyright notice above does not evidence any actual or
 *  intended publication of such software.
 */

#ifndef IOT_STARTER_KIT_WIFI_H
#define IOT_STARTER_KIT_WIFI_H

#if defined(ARDUINO)
#include <Arduino.h>
#endif

typedef void (*IMCbTagidNumDataHndl)(char *tagid, double val);
typedef void (*IMCbTagidStrDataHndl)(char *tagid, char *val);
typedef void (*IMCbDataRespHndl)(long long trxid, char *respCode);
typedef void (*IMCbErrorHndl)(int errCode);

class IoTMakers
{
  public:
    IoTMakers();

#if defined(ARDUINO)
	//int init(const char *ip, int port, const char *deviceId, const char *devicePasswd, const char *gatewayId, Client& client);
	//int init(const char *deviceId, const char *devicePasswd, const char *gatewayId, Client& client);
	int init(const char *deviceId, const char *devicePasswd, const char *gatewayId); // Add Func
	int begin(const char* ssid, const char* pass); // Temporarily
	int getFreeRAM();
#else
	int init(const char *ip, int port, const char *deviceId, const char *devicePasswd, const char *gatewayId);
	int init(const char *deviceId, const char *devicePasswd, const char *gatewayId);
#endif

    char isServerDisconnected();

	int connect();
	int disconnect();
	int auth_device();
	int send_numdata(const char* tagid, double val);
	int send_strdata(const char* tagid, char* val);
	void loop();

	void set_numdata_handler(IMCbTagidNumDataHndl cb_proc);
	void set_strdata_handler(IMCbTagidStrDataHndl cb_proc);
	void set_dataresp_handler(IMCbDataRespHndl cb_proc);
	void set_error_handler(IMCbErrorHndl cb_proc);

#ifdef ARDUINO
    int readID();                // 160112, WiFi 셀렉 기능 추가를 위함, 개선 요망
    char* getDefaultAP();        // 160112, 접두사와 DIP 스위치 값 추가로 AP 이름 생성
#endif
};

#endif
