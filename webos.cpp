#include "application.h"

#include "webos.h"
//#include "json.h
#include "ws.h"

#include <stdint.h>
#include <math.h>

namespace {

TCPClient client;
uint8_t server[] = {192, 168, 1, 116}; // TODO: write setter
const int RESPONSE_BUF_LENGTH = 2048;
char response[RESPONSE_BUF_LENGTH];

bool connect(const char** key = NULL, int* length = NULL) {
  if (client.connect(server, 3000))
  {
    client.println("GET / HTTP/1.1");
    client.println("Upgrade: websocket");
    client.println("Connection: Upgrade");
    client.println("Sec-WebSocket-Version: 13");
//    const char* mykey = "Smx2ZnYxbMJdN3p0SWdjNw==";
    const char* mykey = "YWJjZGVmZ2gxMjM0NTY3OA==";
    client.println("Sec-WebSocket-Key: YWJjZGVmZ2gxMjM0NTY3OA=="); // client.println(mykey);
    client.println("Host: 192.168.1.116:3000");
    client.println();

    memset(response, 0, RESPONSE_BUF_LENGTH);
    int pos = 0;
    while (client.connected()) {
        if (client.available()) {
            int c = client.read();
            if (c > -1) {
                response[pos++] = (char)c;
                Serial.print((char)c);
            }
            if (pos > 4 && response[pos-1] == '\n' && response[pos-2] == '\r' && response[pos-3] == '\n' && response[pos-4] == '\r') {
                Serial.println(". got end of http response");
                break;
            }
        }
    }
    Serial.println("\r\nAAAAAAAAAAAAAAaaaAAAAAAAAAA");
    if (!client.connected()) {
        client.stop();
    }
        
    if (ws::getWebSocketKey(response, key, length) == 101) {
        ws::verifyHandshake(mykey, *length);  
        return true;
    }
    return false;
  }
  return false;
}

bool sendTV(const char* message, int length, const char** resp = NULL, int* responseLength = NULL) {
    uint32_t headerLength = ws::getFrameHeaderLength(length, true);
    char buf[length + headerLength];
    ws::encodeFrame(message, length, true, buf);
    Serial.print("ENCODED MESSAGE: '");
    for(char* p = buf; p != buf+headerLength; ++p) {
      Serial.print(" 0x");
      Serial.print((int)*p, HEX);
    }  Serial.println("'");

    Serial.println("---------------------");
    for(const char* p = message; p != message+length; ++p) {
      Serial.print((char)*p);
    }
    Serial.println("---------------------");

    
    if (client.connected()) {
        Serial.println("Connected.");
    } else {
        Serial.println("NOT Connected.");
    }
    
    int n = client.write((uint8_t*)buf, length+headerLength);
    Serial.print(n);
    Serial.println(" bytes written");
    
    return true;
}

#define LG_KEY "a21eb28e3f719cb5d9ceb9d3ea8cf3f4"

const char* handshake = "{\"type\":\"register\",\"id\":\"register_0\",\"payload\":{\"forcePairing\":false,\"pairingType\":\"PROMPT\",\"client-key\":\""
    LG_KEY
    "\",\"manifest\":{\"manifestVersion\":1,\"appVersion\":\"1.1\",\"signed\":{\"created\":\"20140509\",\"appId\":\"com.lge.test\","
    "\"vendorId\":\"com.lge\",\"localizedAppNames\":{\"\":\"LG Remote App\",\"ko-KR\":\"LG APP\",\"zxx-XX\":\"LG APP\"},"
    "\"localizedVendorNames\":{\"\":\"LG Electronics\"},\"permissions\":[\"TEST_SECURE\",\"CONTROL_INPUT_TEXT\","
    "\"CONTROL_MOUSE_AND_KEYBOARD\",\"READ_INSTALLED_APPS\",\"READ_LGE_SDX\",\"READ_NOTIFICATIONS\",\"SEARCH\",\"WRITE_SETTINGS\","
    "\"WRITE_NOTIFICATION_ALERT\",\"CONTROL_POWER\",\"READ_CURRENT_CHANNEL\",\"READ_RUNNING_APPS\",\"READ_UPDATE_INFO\",\"UPDATE_FROM_REMOTE_APP\","
    "\"READ_LGE_TV_INPUT_EVENTS\",\"READ_TV_CURRENT_TIME\"],\"serial\":\"2f930e2d2cfe083771f68e4fe7bb07\"},\"permissions\":[\"LAUNCH\",\"LAUNCH_WEBAPP\","
    "\"APP_TO_APP\",\"CLOSE\",\"TEST_OPEN\",\"TEST_PROTECTED\",\"CONTROL_AUDIO\",\"CONTROL_DISPLAY\",\"CONTROL_INPUT_JOYSTICK\",\"CONTROL_INPUT_MEDIA_RECORDING\","
    "\"CONTROL_INPUT_MEDIA_PLAYBACK\",\"CONTROL_INPUT_TV\",\"CONTROL_POWER\",\"READ_APP_STATUS\",\"READ_CURRENT_CHANNEL\",\"READ_INPUT_DEVICE_LIST\","
    "\"READ_NETWORK_STATE\",\"READ_RUNNING_APPS\",\"READ_TV_CHANNEL_LIST\",\"WRITE_NOTIFICATION_TOAST\",\"READ_POWER_STATE\",\"READ_COUNTRY_INFO\"],"
    "\"signatures\":[{\"signatureVersion\":1,\"signature\":\"eyJhbGdvcml0aG0iOiJSU0EtU0hBMjU2Iiwia2V5SWQiOiJ0ZXN0LXNpZ25pbmctY2VydCIsInNpZ25hdHVyZVZl"
    "cnNpb24iOjF9.hrVRgjCwXVvE2OOSpDZ58hR+59aFNwYDyjQgKk3auukd7pcegmE2CzPCa0bJ0ZsRAcKkCTJrWo5iDzNhMBWRyaMOv5zWSrthlf7G128qvIlpMT0YNY+n/FaOHE7"
    "3uLrS/g7swl3/qH/BGFG2Hu4RlL48eb3lLKqTt2xKHdCs6Cd4RMfJPYnzgvI4BNrFUKsjkcu+WD4OO2A27Pq1n50cMchmcaXadJhGrOqH5YmHdOCj5NSHzJYrsW0HPlpuAx/ECMeIZYDh6RMqaFM2"
    "DXzdKX9NmmyqzJ3o/0lkk/N97gfVRLW5hA29yeAwaCViZNCP8iC9aO0q9fQojoa7NQnAtw==\"}]}}}";

} // end anonymous namespace

namespace webos {

void getVolume () {
    Serial.println("tvState.vol!!!!!");


    const char* cmdGetVolume = "{\"type\":\"request\",\"id\":1,\"uri\":\"ssap://audio/getVolume\",\"payload\":{}}";
    const char *key;
    int keyLength = 0;
    //  if (!client.connected()) {
        if (connect(&key, &keyLength)) {
            Serial.print(">");
            Serial.print(keyLength);
            Serial.println("<");
        } else {
            Serial.println("some error while connecting to websocket");
        }
    //}

    const char *resp;
    int length = 0;
    if (sendTV(handshake, strlen(handshake), &resp, &length)) {

        delay(1000);
        if (sendTV(cmdGetVolume, strlen(cmdGetVolume), &resp, &length)) {

        }
    
        int counter = 0;
        while (client.connected()) {
            if (client.available()) {
                int c = client.read();
                counter++;
                if (c > -1) {
                    if (counter > 4) {
                        Serial.print((char)c);
                    } else {
                        Serial.print("0x");
                        Serial.print((char)c, HEX);
                    }
                }
            }
        }
        Serial.println();
        Serial.println("End of transmission.");

        
    } else {
        Serial.println(">error connecting websocket<");
    }
}

int getState(State* ret) {
  ret->vol = 0;
  ret->power = true;
  ret->mute = true;
}

} // end namepsace webos

