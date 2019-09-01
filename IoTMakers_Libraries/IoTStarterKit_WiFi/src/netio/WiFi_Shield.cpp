#include "WiFi_Shield.h"
#include "Arduino.h"

//Encoding인코딩 - UTF-8 ABC가나다abcㄱㄴㄷ 똠방각하 전홥니다

const char FMT_SET_CIPMODE[] PROGMEM = "AT+CIPMODE=%d\r\n\x00";
const char FMT_SET_CWMODE[] PROGMEM = "AT+CWMODE_CUR=%d\r\n\x00";
const char FMT_SET_CWJAP[] PROGMEM = "AT+CWJAP=\"%s\",\"%s\"\r\n\x00";
const char FMT_REQUESE_CIFSR[] PROGMEM = "AT+CIFSR\r\n\x00";
const char FMT_REQUESE_CWJAP[] PROGMEM = "AT+CWJAP?\r\n\x00";
const char FMT_SET_CIPSTART[] PROGMEM = "AT+CIPSTART=\"%s\",\"%s\",%ld\r\n\x00";

WiFi_Shield::WiFi_Shield(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic)
:SoftwareSerial(receivePin, transmitPin, inverse_logic)
{
	_ssidPrefix =  "";

	_state = STATE_IDLE;
	_rxPacketCount = 0;
}

int WiFi_Shield::begin(const char* ssid)
{
	SoftwareSerial::begin(9600);

	connectAP(ssid, "0");

	return _state;
}

int WiFi_Shield::begin(const char* ssid, const uint8_t secure_type, const char* secure_key)
{
	SoftwareSerial::begin(9600);

	connectAP(ssid, secure_key);

	return _state;
}

int WiFi_Shield::connect(const char* ip, const long port, const uint8_t type)
{
	connectServer(ip, port, type);

	return _state;
}

char* WiFi_Shield::localIP()
{
	return _localIP;
}

char* WiFi_Shield::SSID()
{
	return _ssid;
}

char* WiFi_Shield::BSSID()
{
    return _bssid;
}

uint8_t WiFi_Shield::status()
{
	receive();
    return _state;
}

unsigned long WiFi_Shield::availablePacket()
{
	receive();
	return _rxPacketCount;
}

uint8_t WiFi_Shield::readPacket(char* buffer)
{
	//memcpy(buffer, _szRxPacket, _rxPacketCount);
	sprintf(buffer,"%s", _szRxPacket);
	//buffer[_rxPacketCount] = 0;
	_rxPacketCount = 0;

	return _state;
}


void WiFi_Shield::connectAP(const char* ssid, const char* secure_key)
{
	this->SoftwareSerial::print(F("+++"));
	delay(1000);
	this->SoftwareSerial::print(F("+++"));
	delay(1200);
	this->SoftwareSerial::print(F("AT\r\n"));
	delay(100);
	this->SoftwareSerial::print(F("ATE0\r\n")); //Echo mode disable
	delay(100);
	RxFlush();

	readCMDOnPGM(FMT_SET_CIPMODE);
	sprintf(_szTxPacket, cmd_buf, 0);
	requestCommand(AT_STATE_OK);

	readCMDOnPGM(FMT_SET_CWMODE);
	sprintf(_szTxPacket, cmd_buf, 1);
	requestCommand(AT_STATE_OK);
	if(_at_state != AT_STATE_OK)
		return;

	strcpy(inputSsid, ssid);
	strncpy(inputSsid, ssid, sizeof(inputSsid) - 1); /* OK ... but `dst` needs to be NUL terminated */
		inputSsid[sizeof(inputSsid)-1] = '\0';
	// inputSsid = ssid;

	readCMDOnPGM(FMT_SET_CWJAP);
	sprintf(_szTxPacket, cmd_buf, ssid, secure_key);
	requestCommand(AT_STATE_OK);

	if(_at_state != AT_STATE_FAIL)
	{
		if(_at_state == AT_STATE_OK)
		{
			_state = STATE_CONNECTED_AP;
		}
		RxFlush();//버퍼의 쓰레기 값 처리
		readCMDOnPGM(FMT_REQUESE_CIFSR);
		sprintf(_szTxPacket, cmd_buf); //로컬 IP
		requestCommand(AT_STATE_OK);
		delay(100); //WiFi 에서 요청한 데이터를 수신받을 수 있게 대기
		readCMDOnPGM(FMT_REQUESE_CWJAP);
		sprintf(_szTxPacket, cmd_buf); //AP 접속 정보
		requestCommand(AT_STATE_OK);
		delay(100); //WiFi 에서 요청한 데이터를 수신받을 수 있게 대기
		receive(); //WiFi 에서 받은 데이터(AP 접속 정보, 로컬 IP 등) 파싱하여 저장
	}
}

int WiFi_Shield::connectServer(const char* ip, const unsigned long port, const uint8_t type)
{
	readCMDOnPGM(FMT_SET_CIPSTART);
    sprintf(_szTxPacket, cmd_buf, ((type == SOCKET_TYPE_TCP) ? "TCP" : "UDP"), ip, port);

	requestCommand(STATE_CONNECTED_SERVER);
	if(_state == STATE_CONNECTED_SERVER)
	{
		this->SoftwareSerial::print(F("AT+CIPMODE=1\r\n"));
		delay(100);
		this->SoftwareSerial::print(F("AT+CIPSEND\r\n"));
		delay(500);
		RxFlush();
	}
}

void WiFi_Shield::disconnectServer()
{
	this->SoftwareSerial::print(F("AT+CIPCLOSE\r\n"));
	delay(200);
	RxFlush();
}

void WiFi_Shield::print(char* data, unsigned int length)
{
	int data_length = length;

	for(int i=0; i<length; i++)
	{
		this->SoftwareSerial::write(data[i]);
	}
}

//AT+CIPSEND 를 다시 보내지 않기 위하여 재 전송을 없엠
uint8_t WiFi_Shield::requestCommandNoRetry(const uint8_t requestState)
{
	uint8_t attempts = WL_MAX_RETRY_COUNT;
	uint32_t currMillis = WL_DELAY_START_CONNECTION + 1;
	uint32_t prevMillis = 0;

	this->SoftwareSerial::print(_szTxPacket);

	do {
		if((currMillis - prevMillis) > WL_DELAY_START_CONNECTION)
		{
			prevMillis = currMillis;

		    attempts--;
		}
		currMillis = millis();
		receive();
	}while((_state != requestState) && (attempts > 0) && (_at_state != requestState) );
}

uint8_t WiFi_Shield::requestCommand(const uint8_t requestState)
{
	uint8_t attempts = WL_MAX_RETRY_COUNT;
	uint32_t currMillis = 0;
	uint32_t prevMillis = 0;

	do {
		if((currMillis - prevMillis) > WL_DELAY_START_CONNECTION || attempts == WL_MAX_RETRY_COUNT)
		{
			prevMillis = currMillis;
			#ifdef _WOW_DEBUG
			Serial.print(F("Send AT : "));
			Serial.println(_szTxPacket);
			#endif
		    this->SoftwareSerial::print(_szTxPacket);

		    attempts--;
		}
		receive();
	}while((_state != requestState) && (attempts > 0) && (_at_state != requestState) );
}

uint8_t WiFi_Shield::requestCommand(const uint8_t requestState, int size)
{
	int8_t attempts = WL_MAX_RETRY_COUNT;
	uint32_t currMillis = 0;
	uint32_t prevMillis = 0;

	do {
		if((currMillis - prevMillis) > WL_DELAY_START_CONNECTION || attempts == WL_MAX_RETRY_COUNT)
		{
			prevMillis = currMillis;
			for(int index=0; index < size; index++ )
			{
				this->SoftwareSerial::write((char)_szTxPacket[index]);
				#ifdef _WOW_DEBUG
				Serial.print(F("Byte:")); Serial.println((unsigned char)_szTxPacket[index], HEX);
				#endif
			}
		    attempts--;
		}
		currMillis = millis();
		receive();
	}while((_state != requestState) && (attempts > 0) && (_at_state != requestState) );
}

int WiFi_Shield::processPacket()
{
    static int cnt;
    static char* pszFindIdx;
    static char* pszToken[3];

    if(this->availablePacket() > 0) {
        pszFindIdx = _szRxPacket;
        pszToken[0] = pszFindIdx;
        for(cnt = 1; cnt < 3; cnt++) {
            pszFindIdx = strchr(pszFindIdx, ',');
            if(pszFindIdx == NULL) {
                break;
            } else {
                *pszFindIdx = 0;
                pszFindIdx++;
                pszToken[cnt] = pszFindIdx;
            }
        }

        if(cnt == 3) {
            pszToken[2][strlen(pszToken[2]) - 1] = 0;

            receiveDataHandl(pszToken[0], pszToken[1], pszToken[2]);
        }
    }
}

void WiFi_Shield::registerControlHandler(ReceiveDataHandl handler)
{
	receiveDataHandl = handler;
}

uint8_t WiFi_Shield::receive() {
	static char szRxDataBuffer[RECEIVE_BUFFER_LENGTH];
	static unsigned char Buffcnt = 0;
	char* pszStart;
	char* pszEnd;
	uint8_t digit = 1;
	uint8_t ret = 0;
	_at_state=STATE_IDLE;
	// Serial.print("-----------------");
	// Serial.println(inputSsid);
	while(this->available() > 0) {
		szRxDataBuffer[Buffcnt] = this->read();

		#if 0
		Serial.print("[");
		Serial.print(Buffcnt);
		Serial.print("]");
		if(szRxDataBuffer[Buffcnt]<48)
		{
			Serial.print("0x");
			Serial.print(szRxDataBuffer[Buffcnt], HEX);
		}
		else
			Serial.print(szRxDataBuffer[Buffcnt]);

		Serial.print(" ");
		#endif

		if((Buffcnt == 1) && (szRxDataBuffer[Buffcnt] == '\n'))
		{
			if(Buffcnt < (RECEIVE_BUFFER_LENGTH - 1)) {
				Buffcnt++;
			}
		    continue;
		}

		if(szRxDataBuffer[Buffcnt] == '\n'){
			if(szRxDataBuffer[0] == '\r')
			{
				#ifdef _WOW_DEBUG
				Serial.print(F("AT Resopose : "));
				Serial.println(szRxDataBuffer);
				#endif //#ifdef _WOW_DEBUG
				if(strncmp(szRxDataBuffer + 2, "SEND OK", 7) == 0)
				{
					_at_state = AT_STATE_SEND_OK;
				}
				else if(strncmp(szRxDataBuffer + 2, "OK",2) == 0)
				{
					_at_state = AT_STATE_OK;
				}
				else if(strncmp(szRxDataBuffer + 2, "ERROR",5) == 0)
				{
					_at_state = AT_STATE_ERROR;
				}
				else
				{
					if(strncmp(szRxDataBuffer + 2, "+IPD",4) == 0)
					{
						//Serial.println("Received the message from Gateway");
						szRxDataBuffer[Buffcnt + 1]=0;
						pszStart = strchr(szRxDataBuffer+2, ',') + 1;
						_rxPacketCount = 0;
						while( ('0' <= *pszStart) && (*pszStart <= '9') )
						{
							_rxPacketCount *= digit;
							_rxPacketCount += ((uint32_t)(*pszStart-48));
							pszStart++;
							digit*=10;
						}
						pszStart = strchr(pszStart, ':') + 1;

						memcpy(_szRxPacket, pszStart, _rxPacketCount);
						memset(szRxDataBuffer,0,RECEIVE_BUFFER_LENGTH);
						_szRxPacket[_rxPacketCount+1] = 0;

						/*
						Serial.print("_rxPacketCount : ");
						Serial.print(_rxPacketCount);
						Serial.print("\tRecieved Data : ");
						Serial.println(_szRxPacket);
						*/
						ret = _rxPacketCount;
					}
        		}
		    	Buffcnt = 0;
				return ret;
			}
			else if( (szRxDataBuffer[Buffcnt-1] == '\r') && (Buffcnt > 0) )//Message from esp
			{
				#ifdef _WOW_DEBUG
				Serial.print(F("message:"));
				Serial.println(szRxDataBuffer);
				#endif //#ifdef _WOW_DEBUG
				if(strncmp(szRxDataBuffer, "WIFI DISCONNECT",15)== 0)
				{
					//Serial.println("Message : WIFI Disconnect"); //debug
					_state = STATE_DISCONNECTED_AP;
				}
				else if(strncmp(szRxDataBuffer, "WIFI CONNECTED",14) == 0)
				{
					//Serial.println("Message : WIFI Connected"); //debug
					_state = STATE_CONNECTED_AP;
				}
				else if(strncmp(szRxDataBuffer, "CONNECT", 7) == 0)
				{
					//Serial.println("Message : Gateway Connected"); //debug
					_state = STATE_CONNECTED_SERVER;
				}
				else if(strncmp(szRxDataBuffer, "CLOSED", 6) == 0)
				{
					//Serial.println("Message : Gateway CLOSED"); //debug
					_state = STATE_DISCONNECTED_SERVER;
				}
				else if(strncmp(szRxDataBuffer, "+CWJAP:",7) == 0)
				{
					/*
					>AT+CWJAP?
					<+CWJAP:"WOWEDU222","f4:<f2:6d:bf:69:e1",1,-65\r\n
					*/
					pszStart = szRxDataBuffer + 8;	//" 제외
					pszEnd = strchr(pszStart, ','); //" 제외
					memcpy(_ssid, pszStart, (int)(pszEnd - 1 - pszStart));

					pszStart = pszEnd + 2; //" 제외
					pszEnd = strchr(pszStart, ','); //" 제외
					memcpy(_bssid, pszStart, (int)(pszEnd - 1 - pszStart));
					_at_state = AT_STATE_OK;
				}
				else if(strncmp(szRxDataBuffer, "+CIFSR:STAIP,",13) == 0)
				{
					delay(100);
					/*
					>AT+CIFSR
					<+CIFSR:STAIP,"192.168.20.4"\r\n
					<+CIFSR:STAMAC,"5c:cf:7f:d1:bf:7e"\r\n
					*/
					pszStart = szRxDataBuffer + 14; //" 제외
					pszEnd = strchr(pszStart, '\"'); //" 제외
					memcpy(_localIP, pszStart, (int)(pszEnd - pszStart));
					_at_state = AT_STATE_OK;
				}
				memset(szRxDataBuffer,0,RECEIVE_BUFFER_LENGTH);
				Buffcnt=0;
				return 0;
			}
		}
		else if(Buffcnt < (RECEIVE_BUFFER_LENGTH - 1))
		{
			Buffcnt++;
		}
	}

	return ret;
}

char* WiFi_Shield::readCMDOnPGM(const char* PROGMEM data)
{
	char* p = (char*)data;
	unsigned char idx = 0;

	while ( (cmd_buf[idx++] = (char)pgm_read_byte_near(p++)) != 0 );

	return cmd_buf;
}

void WiFi_Shield::RxFlush()
{
	while(this->available())
		this->read();
}

void WiFi_Shield::WaitRxFlush()
{
	while(this->available()==0);
	RxFlush();
}
