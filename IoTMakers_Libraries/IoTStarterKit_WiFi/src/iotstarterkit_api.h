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



#ifndef IOT_STARTER_KIT_API_H
#define IOT_STARTER_KIT_API_H

#include <Arduino.h>

#define IM_KEEPALIVE_PERIODIC_SEC		30
#define IM_KEEPALIVE_SERVER_SEC			7
#define IM_READ_TIMEOUT_SEC				3
#define IM_PACKET_HEAD_LEN				35
#define IM_SERVER_IP					"220.90.216.90"
#define IM_SERVER_PORT					10020

#define READ_BUFF_LEN					40

typedef void (*IMCbTagidNumDataHndl)(char *tagid, double val);
typedef void (*IMCbTagidStrDataHndl)(char *tagid, char *val);
typedef void (*IMCbDataRespHndl)(long long trxid, char *respCode);
typedef void (*IMCbErrorHndl)(int errCode);

typedef struct IMBase_t {
	char*			ipstr;
	unsigned short	port;
	char*			deviceId;
	char*			athnRqtNo;
	char*			extrSysId;

	char			AuthNO[16];

	char			isChAthnSuccess;
	char			lastErrorCode;
	char			timeoutCount;

	IMCbTagidNumDataHndl	cb_numdata_hndl;
	IMCbTagidStrDataHndl	cb_strdata_hndl;
	IMCbDataRespHndl		cb_dataresp_hndl;
	IMCbErrorHndl			cb_error_hndl;

	unsigned int 		sndPktCount;
	unsigned int 		rcvPktCount;

} IMBase, *IMBasePtr;

////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

char im_isServerDisconnected();
int im_init_with_ip(char *ip, int port, char *deviceId, char *athnRqtNo, char *extrSysId);
int im_init(char *deviceId, char *athnRqtNo, char *extrSysId);
int im_connect();
int im_disconnect();
int im_auth_device();
int im_send_numdata(const char* tagid, double val);
int im_send_strdata(const char* tagid, char* val);
void im_loop();

void im_set_numdata_handler(IMCbTagidNumDataHndl cb_proc);
void im_set_strdata_handler(IMCbTagidStrDataHndl cb_proc);
void im_set_dataresp_handler(IMCbDataRespHndl cb_proc);
void im_set_error_handler(IMCbErrorHndl cb_proc);

#ifdef __cplusplus
}
#endif



#endif
