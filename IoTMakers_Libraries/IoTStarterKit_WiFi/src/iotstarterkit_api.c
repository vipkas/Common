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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "iotstarterkit_api.h"
#include "util/util.h"
#include "util/log.h"
#include "packet/head.h"
#include "packet/body.h"
#include "netio/sock.h"

static IMBase	g_im;

static char* ATTR_athnRqtNo = "athnRqtNo";
static char* ATTR_athnNo = "athnNo";
static char* ATTR_respCd = "respCd";
static char* ATTR_respMsg = "respMsg";
static char* ATTR_snsnTagCd = "snsnTagCd";
static char* ATTR_dataTypeCd = "dataTypeCd";

static char	g_attr_buff[READ_BUFF_LEN];
static char	g_val_buff[READ_BUFF_LEN];

static unsigned int		g_bodyLength4Read;
static unsigned int		g_bodyReadCharIndex;

static unsigned int		g_packetLength4Send;
static unsigned int		g_bodyLength4Send;

static unsigned long	g_tickSec;
static unsigned long	g_keepAliveSentSec;

static unsigned long	g_ServerkeepAliveSentSec;


static void imcb_default_control_request_handler_4_numdata(char *tagid, double val)
{
	//INFO_LOG("tagid=[%s], numval=[%f]", tagid, val);
}

static void imcb_default_control_request_handler_4_strdata(char *tagid, char *val) // Temporarily
{
	//INFO_LOG("tagid=[%s], strval=[%s]", tagid, val);
	//im_imcb_default_control_request_handler_4_strdata(tagid, val);
}

static void imcb_default_data_response_handler_4_trxid(long long trxid, char *respCode)
{
	//INFO_LOG("trxid=[%lld], respCode=[%s]", trxid, respCode);
}

static void imcb_default_error_handler(int errCode)
{
	//INFO_LOG("respCode=[%d]", errCode);
}

char im_isServerDisconnected()
{
	if(g_im.timeoutCount>=10)
	{
		g_im.timeoutCount = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

// Default Setting
int im_init_with_ip(char *ip, int port, char *deviceId, char *athnRqtNo, char *extrSysId)
{
	g_im.ipstr = (char*)ip;
	g_im.port = (unsigned short)port;
	g_im.deviceId = (char*)deviceId;
	g_im.athnRqtNo = (char*)athnRqtNo;
	g_im.extrSysId = (char*)extrSysId;

	g_im.cb_numdata_hndl = imcb_default_control_request_handler_4_numdata;
	g_im.cb_strdata_hndl = imcb_default_control_request_handler_4_strdata; // Temporarily
	g_im.cb_dataresp_hndl = imcb_default_data_response_handler_4_trxid;
	g_im.cb_error_hndl = imcb_default_error_handler;

	g_im.sndPktCount =0;
	g_im.rcvPktCount = 0;
	g_im.timeoutCount = 0;

	g_tickSec = im_util_get_secs();
	g_keepAliveSentSec = g_tickSec;

	/**
	im_log_set_level(LOG_LEVEL_ERROR);
	im_log_set_level(LOG_LEVEL_INFO);
	im_log_set_level(LOG_LEVEL_DEBUG);
	*/
	im_log_set_level(LOG_LEVEL_ERROR);

	return 0;
}


int im_init(char *deviceId, char *athnRqtNo, char *extrSysId)
{
	return im_init_with_ip(IM_SERVER_IP, IM_SERVER_PORT, deviceId, athnRqtNo, extrSysId);
}

int im_connect()
{
	return im_sock_connect(g_im.ipstr, g_im.port);
}

int im_disconnect()
{
	return im_sock_disconnect();
}

void im_set_numdata_handler(IMCbTagidNumDataHndl cb_proc)
{
	g_im.cb_numdata_hndl = cb_proc;
}
void im_set_strdata_handler(IMCbTagidStrDataHndl cb_proc) // Temporarily
{
	g_im.cb_strdata_hndl = cb_proc;
}
void im_set_dataresp_handler(IMCbDataRespHndl cb_proc)
{
	g_im.cb_dataresp_hndl = cb_proc;
}
void im_set_error_handler(IMCbErrorHndl cb_proc)
{
	g_im.cb_error_hndl = cb_proc;
}

static void __write_length(unsigned long pktLen) // Temporarily
{
	pktLen = htonl(pktLen);
	im_sock_send((char*)&pktLen, 4);

}

static unsigned int __remove_redundancy() // Remove Needless Packet
{
	unsigned long pktLen;

	if ( im_sock_recv((char*)&pktLen, 134) < 0 )	{
		return -1;
	};
	return 0;
}

static unsigned long __read_length() // Temporarily
{
	unsigned long pktLen;
	if ( im_sock_recv((char*)&pktLen, 4) < 0 )	{
		return -1;
	};

	pktLen = ntohl(pktLen);
	return pktLen;
}

static char __body_get_ch()
{
	static char ch;

	if ( g_bodyReadCharIndex >= g_bodyLength4Read )	{
		return '\0';
	}

	if ( im_sock_available() <= 0 )	{
		return '\0';
	}
	im_sock_recv(&ch, 1);

	g_bodyReadCharIndex++;
	return ch;
}


static int __body_get_str_val(char* buff)
{
	char ch;
	int idx;

	while ( (ch=__body_get_ch()) != '\"' ) {
		if ( ch == '\0' )	{
			return -1;
		}
	};

	idx = 0;
	while ( (ch=__body_get_ch()) != '\"' ) {
		if(ch == '\0') {
			break;
		}
		buff[idx++] = ch;

		if(idx>=READ_BUFF_LEN)
		{
			im_sock_flush();
			return -1;
		}
	}
	buff[idx++] = '\0';

	return strlen(buff);
}

static int __body_get_attr(char* buff)
{
	char ch;

	if ( __body_get_str_val(buff) < 0 )	{
		return -1;
	}

	ch = __body_get_ch();
	if ( ch != ':' )	{
		return 0;
	}

	return strlen(buff);
}

static int __body_get_num_val(char* buff)
{
	char ch;
	int idx = 0;

	ch=__body_get_ch();
	do	{
		if ( isdigit(ch) || ch=='.' || ch=='-' )		{
			buff[idx++] = ch;
		} else {
			break;
		}
	}
	while ( (ch=__body_get_ch()) != '\0'  );
	buff[idx++] = '\0';
	return strlen(buff);
}

/*
DBG 0123: 0000: 7b 22 61 74 68 6e 52 71 | 74 4e 6f 22 3a 22 31 71   {"athnRqtNo":"1q
DBG 0123: 0000: 31 67 35 66 35 34 73 22 | 2c 22 61 74 68 6e 4e 6f   1g5f54s","athnNo
DBG 0123: 0000: 22 3a 22 30 30 30 30 30 | 30 30 30 33 42 39 41 43   ":"000000003B9AC
DBG 0123: 0000: 46 46 42 30 30 30 30 30 | 30 30 30 33 42 39 43 35   FFB000000003B9C5
DBG 0123: 0000: 45 44 41 22 2c 22 72 65 | 73 70 43 64 22 3a 22 31   EDA","respCd":"1
DBG 0123: 0000: 30 30 22 2c 22 72 65 73 | 70 4d 73 67 22 3a 22 53   00","respMsg":"S
DBG 0123: 0000: 55 43 43 45 53 53 22 7d |                           UCCESS"}


{"athnRqtNo":"1q1g5f54s","athnNo":"000000003B9ACFFB000000003B9C5EDA","respCd":"100","respMsg":"SUCCESS"}
*/
static void __read_DevAuth_res_body()
{
	g_im.isChAthnSuccess = (0);

	while ( __body_get_attr(g_attr_buff) >= 0 )
	{
		//ERROR_LOG("__body_get_attr");
		if ( strcmp(g_attr_buff, ATTR_athnRqtNo) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//ERROR_LOG("%s =[%s]", ATTR_athnRqtNo, g_attr_buff);
		} else if ( strcmp(g_attr_buff, ATTR_athnNo) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//ERROR_LOG("%s =[%s]", ATTR_athnNo, g_attr_buff);
			memcpy(g_im.AuthNO, g_attr_buff, sizeof(g_im.AuthNO));
			g_im.isChAthnSuccess = (1);
		} else if ( strcmp(g_attr_buff, ATTR_respCd) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//ERROR_LOG("%s =[%s]", ATTR_respCd, g_attr_buff);
		} else if ( strcmp(g_attr_buff, ATTR_respMsg) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//ERROR_LOG("%s =[%s]", ATTR_respMsg, g_attr_buff);
		}
	}
}

/*
DBG 0130 im_log_hex: 0000: 7b 22 6d 73 67 48 65 61 | 64 56 4f 22 3a 7b 22 6d   {"msgHeadVO":{"m
DBG 0130 im_log_hex: 0000: 61 70 48 65 61 64 65 72 | 45 78 74 65 6e 73 69 6f   apHeaderExtensio
DBG 0130 im_log_hex: 0000: 6e 22 3a 7b 7d 7d 2c 22 | 72 65 73 70 43 64 22 3a   n":{}},"respCd":
DBG 0130 im_log_hex: 0000: 22 31 30 30 22 2c 22 72 | 65 73 70 4d 73 67 22 3a   "100","respMsg":
DBG 0130 im_log_hex: 0000: 22 53 55 43 43 45 53 53 | 22 7d                     "SUCCESS"}
*/
static void __read_Report_res_body()
{
	while ( __body_get_attr(g_attr_buff) > 0 )
	{
		if ( strcmp(g_attr_buff, ATTR_respCd) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//DEBUG_LOG("%s =[%s]", ATTR_respCd, g_attr_buff);
			g_im.cb_dataresp_hndl(0, g_attr_buff);
		} else if ( strcmp(g_attr_buff, ATTR_respMsg) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//DEBUG_LOG("%s =[%s]", ATTR_respMsg, g_attr_buff);
		}
	}
}


/*
CONTROL EXAMPLES
STR = {"mapHeaderExtension":{},"devCnvyDataVOs":[{"devId":"river4D1459818172685","cnvyRowVOs":[{"snsnDataInfoVOs":[],"sttusDataInfoVOs":[],"contlDataInfoVOs":[],"cmdDataInfoVOs":[],"binDataInfoVOs":[],"strDataInfoVOs":[{"snsnTagCd":"led","strVal":"aaa"}],"dtDataInfoVOs":[],"genlSetupDataInfoVOs":[],"sclgSetupDataInfoVOs":[],"binSetupDataInfoVOs":[],"mapRowExtension":{}}]}],"msgHeadVO":{"mapHeaderExtension":{}}}
NUM = {"mapHeaderExtension":{},"devCnvyDataVOs":[{"devId":"river4D1459818172685","cnvyRowVOs":[{"snsnDataInfoVOs":[{"dataTypeCd":"volume","snsnVal":999.0}],"sttusDataInfoVOs":[],"contlDataInfoVOs":[],"cmdDataInfoVOs":[],"binDataInfoVOs":[],"strDataInfoVOs":[],"dtDataInfoVOs":[],"genlSetupDataInfoVOs":[],"sclgSetupDataInfoVOs":[],"binSetupDataInfoVOs":[],"mapRowExtension":{}}]}],"msgHeadVO":{"mapHeaderExtension":{}}}
*/
static void __read_Control_req_body()
{
	while ( __body_get_attr(g_attr_buff) >= 0 )
	{
		if ( strcmp(g_attr_buff, ATTR_snsnTagCd) == 0 )		{
			__body_get_str_val(g_attr_buff);
				//DEBUG_LOG("%s =[%s]", ATTR_snsnTagCd, g_attr_buff);
			if ( __body_get_attr(g_val_buff) < 0 )	{
				ERROR_LOG("fail body");
				im_sock_flush();
				break;
			}else{
				__body_get_str_val(g_val_buff);
				//DEBUG_LOG("strVal =[%s]", g_val_buff);
			}

			g_im.cb_strdata_hndl(g_attr_buff, g_val_buff);

		} else if ( strcmp(g_attr_buff, ATTR_dataTypeCd) == 0 )		{
			__body_get_str_val(g_attr_buff);
				//DEBUG_LOG("%s =[%s]", ATTR_dataTypeCd, g_attr_buff);
			if ( __body_get_attr(g_val_buff) < 0 )	{
				ERROR_LOG("fail body");
				im_sock_flush();
				break;
			}else{
				__body_get_num_val(g_val_buff);
				//DEBUG_LOG("%s =[%s]", g_val_buff, g_attr_buff);
			}

			g_im.cb_numdata_hndl(g_attr_buff, (double)atof(g_val_buff));
		}
	}
}

/*
DBG 0130 im_log_hex: 0000: 7b 22 72 65 73 70 43 64 | 22 3a 22 31 30 30 22 2c   {"respCd":"100",
DBG 0130 im_log_hex: 0000: 22 72 65 73 70 4d 73 67 | 22 3a 22 53 55 43 43 45   "respMsg":"SUCCE
DBG 0130 im_log_hex: 0000: 53 53 22 7d                                         SS"}
*/
static void __read_KeepAlive_res_body()
{
	while ( __body_get_attr(g_attr_buff) > 0 )
	{
		if ( strcmp(g_attr_buff, ATTR_respCd) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//DEBUG_LOG("%s =[%s]", ATTR_respCd, g_attr_buff);
		} else if ( strcmp(g_attr_buff, ATTR_respMsg) == 0 )		{
			__body_get_str_val(g_attr_buff);
			//DEBUG_LOG("%s =[%s]", ATTR_respMsg, g_attr_buff);
		}
	}
}

static void __read_Unknown_body()
{
}

static int __recv_packet(int timeout_sec) // Temporarily
{
	unsigned int packetLength;
	unsigned int previousSecs;
	unsigned int currentSecs;
	char* pktLen;
	int lenRead;

READ_NEXT:
	if ( timeout_sec <= 0 && im_sock_available() <= 0 )	{
		return 0;
	}
	// Check Read Available
	if ( im_sock_available() <= 0 )
	{
		previousSecs  = im_util_get_secs();
		while ( im_sock_available() <= 0 )	{
			currentSecs  = im_util_get_secs();
			if (  (int)(currentSecs-previousSecs) >=  timeout_sec ){
				g_im.timeoutCount = (g_im.timeoutCount > 10)? 10:g_im.timeoutCount+1;
				ERROR_LOG("timeout");
				return -1;
			}
		}
	}

	packetLength = __read_length(); // 1. read pkt_len
	if ( packetLength < IM_PACKET_HEAD_LEN || packetLength > 2048)	{ // 4
		ERROR_LOG("pkt_size:[%ld]", packetLength);
		im_sock_flush();
		//g_im.timeoutCount = 10;
		return -1;
	}

	lenRead = im_sock_recv(g_val_buff, IM_PACKET_HEAD_LEN); // 2. read head 35
	if ( lenRead < IM_PACKET_HEAD_LEN )	{
		ERROR_LOG("fail read");
		im_sock_flush();
		return -1;
	}
	g_im.timeoutCount = 0;

	g_bodyLength4Read = packetLength-IM_PACKET_HEAD_LEN; // Cal data Length
	g_bodyReadCharIndex = 0;

	if ( head_is_TypeDevAuth_res(g_val_buff) )		{ // Autho
		//DEBUG_LOG("DevAuth_res");
		__read_DevAuth_res_body();
	} else if ( head_is_TypeKeepAlive_res(g_val_buff) )		{ // Keep Alive
		//DEBUG_LOG("KeepAlive_res");
		__read_KeepAlive_res_body();
	} else if ( head_is_TypeReport_res(g_val_buff) )		{ // Report Send Message
		//DEBUG_LOG("Report_res");
		__read_Report_res_body();
	} else if ( head_is_TypeCtrl_req(g_val_buff) )		{ // Control Ack
		//DEBUG_LOG("TypeCtrl_req");
		__read_Control_req_body();
		im_send_control_ack();
	} else {

		////DEBUG_LOG("UNKNOWN HEAD");
		//im_log_hex((unsigned char*)g_val_buff, IM_PACKET_HEAD_LEN);
		__read_Unknown_body();
		im_sock_flush();

	}
	g_im.rcvPktCount++;

	//INFO_LOG("sPkt=[%ld], rPkt=[%ld]", g_im.sndPktCount, g_im.rcvPktCount);
	if ( im_sock_available() > 0 )	{
		goto READ_NEXT;
	}

	//ERROR_LOG("no packet to read");
//#endif


	return 0;
}

/*
 * im_auth_device
 */
int im_auth_device() // Temporarily
{
	unsigned int sent = 0;

	//ERROR_LOG("\nStart auth\n");
	delayMicroseconds(50);
	g_bodyLength4Send = body_sizeof_devAuth(g_im.extrSysId, g_im.deviceId, g_im.athnRqtNo);
	g_packetLength4Send = IM_PACKET_HEAD_LEN + g_bodyLength4Send; // 35 + 100

	// 1. write pkt_len
	//ERROR_LOG("\n1. write pkt_len\n");
	delayMicroseconds(50);
	__write_length(g_packetLength4Send); // Write 135

	// 2. write head
	//ERROR_LOG("\n2. write head\n");
	delayMicroseconds(50);
	sent += head_send_auth_device(); // 35

	// 3. write body // char Type
	//ERROR_LOG("\n3. write body\n");
	delayMicroseconds(50);
	sent += body_send_devAuth(g_im.extrSysId, g_im.deviceId, g_im.athnRqtNo);
	im_debug();

	if ( sent != g_packetLength4Send )	{
		ERROR_LOG("\nfail im_auth_device\n");
		im_sock_flush();
		return -1;
		//return sent;
		//return g_packetLength4Send;
	}
	//ERROR_LOG("\nsend success and will recv ack\n");

	g_im.sndPktCount++;

	// Temporarily
	if(__recv_packet(10)<0) // For Debug
	{
		ERROR_LOG("Recv EEROR");
		im_sock_flush();
		return -1;
	}

	if ( g_im.isChAthnSuccess != (1) )	{
		ERROR_LOG("chAthnSuccess != 1");
		im_sock_flush();
		return -1;
	}

	//return sent;
	return 0;
}

/*
 * im_send_numdata
 */
int im_send_numdata(const char* tagid, double val) // Temporarily
{
	unsigned int sent = 0;

#if defined(ARDUINO)
	im_disableReceive();
	dtostrf(val, 4, 3, g_val_buff);

#else
	snprintf(g_val_buff, sizeof(g_val_buff), "%.12f", val);
#endif
	// Count Length
	g_bodyLength4Send = body_sizeof_numdata_collection((char*)g_im.extrSysId, (char*)g_im.deviceId, (char*)tagid, g_val_buff);
	g_packetLength4Send = IM_PACKET_HEAD_LEN + g_bodyLength4Send;

	// 1. write pkt_len
	__write_length(g_packetLength4Send); //233

	// 2. write head
	sent += head_send_collection(g_im.AuthNO); // 35

	// 3. write body
	sent += body_send_numdata_collection((char*)g_im.extrSysId, (char*)g_im.deviceId, (char*)tagid, g_val_buff);
	if ( sent != g_packetLength4Send )	{
		ERROR_LOG("fail im_send_numdata");
		//return g_packetLength4Send; //233
		//return sent; // 158 205 235
		return -1;  // Rest 129
	}
	im_enableReceive();

	g_im.sndPktCount++;
	__recv_packet(IM_READ_TIMEOUT_SEC); // Temporarily

	return 0;
}
/*
 * im_send_strdata
 */
int im_send_strdata(const char* tagid, char* val)
{
	unsigned int sent = 0;
	im_disableReceive();
	g_bodyLength4Send = body_sizeof_strdata_collection((char*)g_im.extrSysId, (char*)g_im.deviceId, (char*)tagid, val);
	g_packetLength4Send = IM_PACKET_HEAD_LEN + g_bodyLength4Send;

	// 1. write pkt_len
	__write_length(g_packetLength4Send);
	// 2. write head
	sent += head_send_collection(g_im.AuthNO);
	// 3. write body
	sent += body_send_strdata_collection((char*)g_im.extrSysId, (char*)g_im.deviceId, (char*)tagid, val);
	if ( sent != g_packetLength4Send )	{
		ERROR_LOG("fail im_send_strdata");
		return -1;
	}
	im_enableReceive();
	g_im.sndPktCount++;
	__recv_packet(IM_READ_TIMEOUT_SEC); // Temp

	if ( g_im.isChAthnSuccess != (1) )	{
		return -1;
	}

	return 0;
}


/*
 * im_send_keepalive
 */
int im_send_keepalive() // Temporarily
{
	unsigned int sent = 0;
	im_disableReceive();
	g_packetLength4Send = IM_PACKET_HEAD_LEN;

	// 1. write pkt_len
	__write_length(g_packetLength4Send);
	// 2. write head
	sent += head_send_keepalive();
	if ( sent != g_packetLength4Send )	{
		ERROR_LOG("fail im_send_keepalive");
		return -1;
	}

	im_enableReceive();
	g_im.sndPktCount++;
	__recv_packet(IM_READ_TIMEOUT_SEC); // Temp

	return 0;
}


/*
 * im_send_control_ack
 */
int im_send_control_ack()
{
	unsigned int sent = 0;
	im_disableReceive();
	g_bodyLength4Send = body_sizeof_contrl_ack_resp();
	g_packetLength4Send = IM_PACKET_HEAD_LEN + g_bodyLength4Send;

	// 1. write pkt_len
	__write_length(g_packetLength4Send);
	// 2. write head
	sent += head_send_control_ack(g_im.AuthNO);
	// 3. write body
	sent += body_send_control_ack();
	if ( sent != g_packetLength4Send )	{
		ERROR_LOG("fail im_send_control_ack");
		return -1;
	}
	im_enableReceive();
	g_im.sndPktCount++;

	return 0;
}


/*
 * loop
 */
void im_loop() // Temporarily
{
	g_tickSec = im_util_get_secs();

		// Need For Debug
		//if ( g_im.isChAthnSuccess != (1) )	{
		//	im_auth_device();
		//	return;
		//}

		if ( g_tickSec - g_keepAliveSentSec > IM_KEEPALIVE_PERIODIC_SEC) { // 30 sec
			if ( !im_sock_connected() )	{ // Firset

				im_disconnect();
				if ( im_connect() < 0 )	{
					return;
				}
				im_auth_device();
			}
			g_keepAliveSentSec = g_tickSec;
			im_send_keepalive();
		}

		 __recv_packet(0);  // Temp
}
