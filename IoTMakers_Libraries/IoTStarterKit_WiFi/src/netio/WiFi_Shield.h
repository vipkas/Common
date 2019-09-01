#ifndef WiFi_Shield_h
#define WiFi_Shield_h
#define _SS_MAX_RX_BUFF 128
#include "Arduino.h"
#include "Print.h"
#include "SoftwareSerial.h"
#include "IPAddress.h"

//Encoding인코딩 - UTF-8 ABC가나다abcㄱㄴㄷ

// Maximum size of a IP
#define WL_IP_MAX_LENGTH        16
// Maximum size of a SSID
#define WL_SSID_MAX_LENGTH      32
// Length of passphrase. Valid lengths are 8-63.
#define WL_WPA_KEY_MAX_LENGTH   63
// Length of key in bytes. Valid values are 5 and 13.
#define WL_WEP_KEY_MAX_LENGTH   13
// Size of a MAC-address or BSSID
#define WL_MAC_ADDR_LENGTH      18
// Size of a MAC-address or BSSID
#define WL_IPV4_LENGTH          4
//Maximum number of request
#define WL_MAX_RETRY_COUNT      3
// 5 secs of delay to have the connection established
#define WL_DELAY_START_CONNECTION   8000

#define SEND_BUFFER_LENGTH          40
#define RECEIVE_BUFFER_LENGTH       64

class WiFi_Shield : public SoftwareSerial
{
private:
    typedef void (*ReceiveDataHandl)(const char* tag, const char* type, const char* value);

    char* _ssidPrefix;  // 공유기 SSID의 접두사

    char _localIP[WL_IP_MAX_LENGTH];

    char _ssid[WL_SSID_MAX_LENGTH];
    char _bssid[WL_MAC_ADDR_LENGTH];
    char inputSsid[WL_SSID_MAX_LENGTH];
    uint8_t _state;
    uint8_t _at_state;

    char _szTxPacket[SEND_BUFFER_LENGTH];
    char _szRxPacket[RECEIVE_BUFFER_LENGTH];
    uint32_t _rxPacketCount;

    void connectAP(const char* ssid, const char* secure_key);
    int connectServer(const char* ip, const unsigned long port, const uint8_t type);

    uint8_t requestCommandNoRetry(const uint8_t requestState);
    uint8_t requestCommand(const uint8_t requestState);
    uint8_t requestCommand(const uint8_t requestState, int size);
    uint8_t receive();

    char cmd_buf[26];
    char* readCMDOnPGM(const char* PROGMEM data);

    ReceiveDataHandl receiveDataHandl;

public:

    enum State {
        STATE_CONNECTED_AP,
        STATE_DISCONNECTED_AP,
        STATE_CONNECTED_SERVER,
        STATE_DISCONNECTED_SERVER,
        STATE_IDLE,
        AT_STATE_OK,
        AT_STATE_ERROR,
        AT_STATE_SEND_OK,
        AT_STATE_FAIL
    };

    enum SecureType {
      SECURE_TYPE_OPEN,
      SECURE_TYPE_WEP,
      SECURE_TYPE_WPA
    };

    enum SocketType{
      SOCKET_TYPE_TCP,
      SOCKET_TYPE_UDP
    };

    WiFi_Shield(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);

    int begin(const char* ssid);
    int begin(const char* ssid, const uint8_t secure_type, const char* secure_key);

    int connect(const char* ip, const long port, const uint8_t type);
    void disconnectServer();
    int processPacket();
    void registerControlHandler(ReceiveDataHandl handler);

    char* localIP();
    char* SSID();
    char* BSSID();

    uint8_t status();

    void print(char* data, unsigned int length);

    unsigned long availablePacket();
    uint8_t readPacket(char* buffer);

    void RxFlush();
    void WaitRxFlush();
};

#endif
