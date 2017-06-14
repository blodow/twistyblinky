#include "application.h"

#include "amp.h"
#include "xml.h"
#include "uhttp.h"

#include <stdint.h>
#include <math.h>

namespace {

TCPClient client;
uint8_t server[] = {192, 168, 1, 192}; // TODO: write setter
const int RESPONSE_BUF_LENGTH = 2048;
char response[RESPONSE_BUF_LENGTH];

bool sendAmp(const char* cmd, const char** body = NULL, int* length = NULL) {
  int size = strlen(cmd);
  if (client.connect(server, 80))
  {
    client.println("POST /YamahaRemoteControl/ctrl HTTP/1.1");
    client.println("Host: 192.168.1.192");
    client.print("Content-Length: "); client.println(size);
    client.println();
    client.println(cmd);

    memset(response, 0, RESPONSE_BUF_LENGTH);
    int pos = 0;
    while (client.connected()) {
        if (client.available()) {
            int c = client.read();
            if (c > -1) {
                response[pos++] = (char)c;
                Serial.print((char)c);
            }
        }
    }
    if (!client.connected()) {
        client.stop();
    }
    return uhttp::parseHeader(response, body, length) == 200;
  }
  return false;
}

} // end anonymous namespace

namespace amp {

int getVol() {
  const char *volGetCommand = "<YAMAHA_AV cmd=\"GET\"><Main_Zone><Volume><Lvl>GetParam</Lvl></Volume></Main_Zone></YAMAHA_AV>";
  //const char *resp = "<YAMAHA_AV rsp=\"GET\" RC=\"0\"><Main_Zone><Volume><Lvl><Val>-445</Val><Exp>1</Exp><Unit>dB</Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";

  int length = 0;
  const char *resp;
  sendAmp(volGetCommand, &resp, &length);
  if (length <= 0) {
    return -1000;
  }
  const char *pathVol[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Val"};
  const char *pathUnit[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Unit"};
  const char *pathExp[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Exp"};
  const char* text = NULL;
  const char* textEnd = NULL;

  //char* unit = NULL;
  float vol = -2000;
  if (getText(resp, pathVol, 5, &text, &textEnd)) {
    vol = strtola(text, textEnd);
  }
  if (getText(resp, pathUnit, 5, &text, &textEnd)) {
    //unit = (char*)calloc(sizeof(char), 1+textEnd-text);
  }
  if (getText(resp, pathExp, 5, &text, &textEnd)) {
    vol /= pow(10, strtola(text, textEnd));
  }
  return vol;
}


bool getPower() { 
  const char *volGetBasicStatus = "<YAMAHA_AV cmd=\"GET\"><Main_Zone><Basic_Status>GetParam</Basic_Status></Main_Zone></YAMAHA_AV>";
//  const char *volGetBasicStatus = "<YAMAHA_AV cmd=\"GET\"><Main_Zone><Basic_Status><Volume>GetParam</Volume></Basic_Status></Main_Zone></YAMAHA_AV>";
  int length = 0;
  const char *resp;
  sendAmp(volGetBasicStatus, &resp, &length);
  Serial.print("amp::getPower ");
  Serial.print(resp);
  
  return false; 
}
bool getMute() { return false; }
int getInput() { return false; }

int getState(State* ret) {
   const char *volGetBasicStatus = "<YAMAHA_AV cmd=\"GET\"><Main_Zone><Basic_Status>GetParam</Basic_Status></Main_Zone></YAMAHA_AV>";
  int length = 0;
  const char *resp;
  sendAmp(volGetBasicStatus, &resp, &length);

  const char *pathPower[] = {"YAMAHA_AV", "Main_Zone", "Basic_Status", "Power_Control", "Power"};
  const char *pathVol[] = {"YAMAHA_AV", "Main_Zone", "Basic_Status", "Volume", "Lvl", "Val"};
  const char *pathUnit[] = {"YAMAHA_AV", "Main_Zone", "Basic_Status", "Volume", "Lvl", "Unit"};
  const char *pathExp[] = {"YAMAHA_AV", "Main_Zone", "Basic_Status", "Volume", "Lvl", "Exp"};
  const char *pathMute[] = {"YAMAHA_AV", "Main_Zone", "Basic_Status", "Volume", "Mute"};
  const char *pathInput[] = {"YAMAHA_AV", "Main_Zone", "Basic_Status", "Input", "Input_Sel"};
  
  const char* text = NULL;
  const char* textEnd = NULL;
  
  bool mute = false;
  bool inputTV = false;
  bool power = false;
  float vol = -2000;

  if (getText(resp, pathVol, 6, &text, &textEnd)) {
    vol = strtola(text, textEnd);
  }
  if (getText(resp, pathExp, 6, &text, &textEnd)) {
    vol /= pow(10, strtola(text, textEnd));
  } else {
    return -1;
  }

  //char* unit = NULL;
  if (getText(resp, pathUnit, 6, &text, &textEnd)) {
    int length = textEnd - text;
    bool decibel = length == 2 && strncmp(text, "dB", length) == 0;
    //bool units = length == 2 && strncmp(text, "dB", length) == 0;
    
    if (decibel) {
        static const float max = 16.5f;
        static const float min = -80.5f;
        vol = (vol - min) / (max - min);
    }
    //if (units) {
    //    vol = vol / 2;
    //}
    //unit = (char*)calloc(sizeof(char), 1+textEnd-text);
  }

  if (getText(resp, pathMute, 5, &text, &textEnd)) {
    int length = textEnd - text;
    mute = length == 2 && strncmp(text, "On", length) == 0;
  } else {
    return -1;
  }
  if (getText(resp, pathInput, 5, &text, &textEnd)) {
    int length = textEnd - text;
    //inputHDMI = length == 5 && strncmp(text, "HDMI1", length) == 0;
    inputTV = length == 3 && strncmp(text, "AV1", length) == 0;
  } else {
    return -1;
  }
  if (getText(resp, pathPower, 5, &text, &textEnd)) {
    int length = textEnd - text;
    power = length == 2 && strncmp(text, "On", length) == 0;
  } else {
    return -1;
  }
  
  Serial.println("---");
  Serial.print("power: "); if (power)      {Serial.println("yes");} else {Serial.println("no");}
  Serial.print("vol:   "); Serial.println(vol); //Serial.println(unit);
  Serial.print("mute:  "); if (mute)      {Serial.println("yes");} else {Serial.println("no");}
  Serial.print("tv:    "); if (inputTV)   {Serial.println("yes");} else {Serial.println("no");}
  Serial.println("---");

  if (inputTV && power) {
    Serial.println("mode: green");
  }
  if (!power) {
    Serial.println("mode: red");
  }
  Serial.println("---");
  
  if (ret) {
    ret->mute = mute;
    ret->inputTV = inputTV;
    ret->power = power;
    ret->vol = vol;
  }
  
  return 0;
}

/*
<YAMAHA_AV rsp="GET" RC="0">
  <Main_Zone>
    <Basic_Status>
      <Power_Control>
        <Power>On</Power>
        <Sleep>Off</Sleep>
      </Power_Control>
      <Volume>
        <Lvl>
          <Val>-440</Val>
          <Exp>1</Exp>
          <Unit>dB</Unit>
        </Lvl>
        <Mute>On</Mute>
        <Subwoofer_Trim>
          <Val>0</Val>
          <Exp>1</Exp>
          <Unit>dB</Unit>
        </Subwoofer_Trim>
        <Scale>0-97</Scale>
      </Volume>
      <Input>
        <Input_Sel>HDMI1</Input_Sel>
        <Input_Sel_Item_Info>
          <Param>HDMI1</Param>
          <RW>RW</RW>
          <Title>Media PC</Title>
          <Icon>
            <On>/YamahaRemoteControl/Icons/icon004.png</On>
            <Off />
          </Icon>
          <Src_Name />
          <Src_Number>1</Src_Number>
        </Input_Sel_Item_Info>
      </Input>
      <Surround>
        <Program_Sel>
          <Current>
            <Straight>On</Straight>
            <Enhancer>Off</Enhancer>
            <Sound_Program>5ch Stereo</Sound_Program>
          </Current>
        </Program_Sel>
        <_3D_Cinema_DSP>Auto</_3D_Cinema_DSP>
      </Surround>
      <Party_Info>Off</Party_Info>
      <Sound_Video>
        <Tone>
          <Bass>
            <Val>-10</Val>
            <Exp>1</Exp>
            <Unit>dB</Unit>
          </Bass>
          <Treble>
            <Val>0</Val>
            <Exp>1</Exp>
            <Unit>dB</Unit>
          </Treble>
        </Tone>
        <Direct>
          <Mode>Off</Mode>
        </Direct>
        <HDMI>
          <Standby_Through_Info>On</Standby_Through_Info>
          <Output>
            <OUT_1>On</OUT_1>
          </Output>
        </HDMI>
        <Extra_Bass>Off</Extra_Bass>
        <Adaptive_DRC>Off</Adaptive_DRC>
      </Sound_Video>
    </Basic_Status>
  </Main_Zone>
</YAMAHA_AV>
*/






// updateState();

//const char *volUpCmd = "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>Up 1 dB</Val><Exp></Exp><Unit></Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
//const char *volDownCmd = "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>Down 1 dB</Val><Exp></Exp><Unit></Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";

bool setVolUp() {
    const char *volUpCmd = "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>Up 1 dB</Val><Exp></Exp><Unit></Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
    return sendAmp(volUpCmd);
}

bool setVolDown() {
    const char *volDownCmd = "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>Down 1 dB</Val><Exp></Exp><Unit></Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
    return sendAmp(volDownCmd);
}

bool setVolume(float vol) {
    float min = -80.5f;
    float max = 16.5f;
    float volume = (vol*(max-min)) + min;
    int r = 10.0f * roundf(volume * 2.0f) / 2.0f;

    char cmd[150]; // 126 + x
    sprintf(cmd, "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>%d</Val><Exp>1</Exp><Unit>dB</Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>", r);
    return sendAmp(cmd);
}

bool toggleMute() { return false; }

bool turnOff() {
    return sendAmp("<YAMAHA_AV cmd=\"PUT\"><System><Power_Control><Power>Standby</Power></Power_Control></System></YAMAHA_AV>");
}

bool turnOn() {
    return sendAmp("<YAMAHA_AV cmd=\"PUT\"><System><Power_Control><Power>On</Power></Power_Control></System></YAMAHA_AV>");
}

} // end anonymous namespace
