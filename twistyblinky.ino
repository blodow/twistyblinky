#include "buzz.h"
#include "neopixel/neopixel.h"
#include "ws.h"
#include "webos.h"
#include "xml.h"
#include "uhttp.h"
#include "amp.h"
#include "sha.h"

// create shorthand for the pins
static const int quadA = D2;
static const int quadB = D3;
static const int button = D5;
static const int lbo = A2;
static const int ledDataIn = D6;
static const int led2 = D7;

static int light = LOW;

int count = 0;
int last_count = count;
bool a_ = false;
bool b_ = false;

int sleepCounter = 0;

amp::State ampState;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, ledDataIn, WS2812B);
Adafruit_DRV2605 drv;

void buzz(int upDown) {
  static uint8_t hapticEffect = 47; // sharp click
  drv.setWaveform(0, hapticEffect);  // play effect
  drv.setWaveform(1, 0);       // end waveform
  drv.go();
  //hapticEffect+=upDown;
  if (hapticEffect > 117) hapticEffect = 1;
  if (hapticEffect <= 0) hapticEffect = 117;
}

class VolumeState {
public:
    VolumeState () { c = 0; last = 0; }

    float get() { return v; }

    bool shouldUpdate() { return c != last; }

    float desired() { last = c; return v_d; };

    void want(float val) {
        float diff = v_d - val;
        if (diff < 0)
            diff = -diff;

        if (diff < 0.001)
            return;

        v_d = val;
        c++;
    }

    void is(float val) { v = val; }

private:
    float v;
    float v_d;
    int c, last;
};

bool volStateInitialized = false;
VolumeState volState;

os_thread_return_t ampThreadLoop(void* nothing){
    while(true) {
        amp::getState(&ampState);
        //amp::printState(&ampState);
        volState.is(ampState.vol);
        if (!volStateInitialized) {
            volStateInitialized = true;
            count = ampState.vol * 255;
        }

        if (volState.shouldUpdate()) {
            float d = volState.desired();
            amp::setVolume(d);
            delay(100);
        } else {
            delay(400);
        }
    }
}

os_thread_return_t tvThreadLoop(void* nothing){
    while(true) {
        Serial.println("tvState.vol..");

        webos::getVolume();
        //amp::printState(&ampState);
        delay(1500);
    }
}

int offset = 0;
os_thread_return_t cycleOffsetLoop(void* nothing){
    while(true) {
        delay(100);
        offset += 3;
    }
}


void setup()
{
    pinMode(quadA, INPUT);
    pinMode(quadB, INPUT);
    pinMode(button, INPUT);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    drv.begin();
    drv.selectLibrary(1);
    drv.setMode(DRV2605_MODE_INTTRIG);
    drv.useERM();

    attachInterrupt(quadA, quadDecodeA, CHANGE);
    attachInterrupt(quadB, quadDecodeB, CHANGE);
    attachInterrupt(button, buttonPress, CHANGE);

    pinMode(led2, OUTPUT);
    digitalWrite(led2, light);

    Serial.begin(115200);
    Particle.function("ampPower", ampPower);
    Particle.function("getPower", ampGetPower);
    //Particle.function("ampVolume", ampVol);
    Particle.function("getState", getState);

    Particle.function("tvVolume", tvVol);
    a_ = pinReadFast(quadA);
    b_ = pinReadFast(quadB);


    Thread* ampThread = new Thread("ampThread", ampThreadLoop, NULL);
    //Thread* tvThread = new Thread("tvThread", tvThreadLoop, NULL);
    Thread* colorfulThread = new Thread("cycleOffsetThread", cycleOffsetLoop, NULL);

    //mediaPCThread = new Thread("mediaPCThread", pingThreadLoop, (void*)"tv.lan");
    //nasThread = new Thread("nasThread", pingThreadLoop, (void*)"nas.lan");
    rainbow(3);
}

int last_time = 0;

void loop() {
  handleRotation();
  //handleAmp();
  //handleTV();
  //handlePings();
  //handlePower();

  //rainbow(20);
  //off();
  //delay(1000);
  if (++sleepCounter > 10) {
    //gotoSleep();
  }
}

int center = 0;

void handleRotation() {
  if (count < 0) count = 0;
  if (count > 255) count = 255;
  if (last_count != count) {
    Serial.println(count);
    last_count = count;
  }

  int clicks = count / 4;
  float vol = count / 255.0f;
  volState.want(vol);
  visualizeQuadCounting(volState.get()*24.0f);
}

void gotoSleep() {
  System.sleep(button, CHANGE);
  System.reset();
}

void quadDecodeA() {
  bool a = pinReadFast(quadA);
  bool b = pinReadFast(quadB);
  count += parse(a,b);
  a_ = a;
  b_ = b;
}

void quadDecodeB() {
  bool a = pinReadFast(quadA);
  bool b = pinReadFast(quadB);
  count += parse(a,b);
  a_ = a;
  b_ = b;
}

int lastDebounceTime = 0;

int buttonPress() {
    if (!pinReadFast(button)) {
        if ((millis() - lastDebounceTime) > 200)
        {
            lastDebounceTime = millis();

            light = !light;
            digitalWrite(led2, light);
            buzz(0);
        }
    }
}

int parse(bool a, bool b) {
  if(a_ && b_){
    if(!a && b) return 1;
    if(a && !b) return -1;
  } else if(!a_ && b_){
    if(!a && !b) return 1;
    if(a && b) return -1;
  } else if(!a_ && !b_){
    if(a && !b) return 1;
    if(!a && b) return -1;
  } else if(a_ && !b_){
    if(a && b) return 1;
    if(!a && !b) return -1;
  }
  return 0;
}


int ampPower(String command) {
    if (command=="on") {
        return amp::turnOn();
    }
    else if (command=="off") {
        return amp::turnOff();
    }
    else {
        return -1;
    }
}

int ampGetPower(String command) {
    return amp::getPower();
}
int getState(String command) {
    unsigned long time1 = millis();

    return amp::getState();
    Serial.println(millis() - time1);
}

/*
int ampVol(String command) {
    if (command=="up") {
        //amp::setVol(-440);
        return amp::getVol();
    }
    else if (command=="down") {
        amp::setVol(-805);
        return amp::getVol();
    }
    else {
        return -1;
    }
}
*/

int tvVol(String command) {
    webos::getVolume();
    return 0;
}

void off() {
  uint16_t n = strip.numPixels();

  for(uint16_t i=0; i<n; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}


void visualizeQuadCounting(uint8_t pos) {
  uint16_t n = strip.numPixels();
  //strip.setPixelColor(0, 0x100000);
  for(uint16_t i=0; i<n; i++) {
      if (i <= (pos % n)) {
        strip.setPixelColor(i, Wheel((int)(offset + (i * 255.0 / n)) & 255, ((float)0xc) / 0xff));
      } else {
        strip.setPixelColor(i, 0x0);
      }
  }
  strip.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
  uint16_t n = strip.numPixels();

  for(j=0; j<256; j++) {
    for(i=0; i<n; i++) {
      strip.setPixelColor(i, Wheel((5*i+j) & 255, 0.05));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos, float brightness) {
  if(WheelPos < 85) {
   return strip.Color(brightness * WheelPos * 3, brightness * (255 - WheelPos * 3), 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(brightness * (255 - WheelPos * 3), 0, brightness * WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, brightness * WheelPos * 3, brightness * (255 - WheelPos * 3));
  }
}
