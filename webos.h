#pragma once

namespace webos {

struct State {
  bool mute;
  bool power;
  float vol;
};

void getVolume ();
int getState(State* ret = NULL);
}

