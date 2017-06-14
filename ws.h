#pragma once

#include <stdint.h>


namespace ws {

int getWebSocketKey(const char* buf, const char** key, int* length);

void verifyHandshake(const char* key_p, uint32_t key_length);

int getFrameHeaderLength(uint32_t length, bool mask);

void encodeFrame(const char* buf, uint32_t length, bool mask, char* dest);

} // end uhttp namespace
