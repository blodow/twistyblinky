#pragma once

namespace amp {
    
struct State {
  bool mute;
  bool inputTV;
  bool power;
  float vol;
};

int getVol();
bool getPower();
bool getMute();
int getInput();

int getState(State* ret = NULL);

// updateState();

bool setVolUp();
bool setVolDown();
bool setVolume(float vol);
bool toggleMute();
bool turnOff();
bool turnOn();

}

