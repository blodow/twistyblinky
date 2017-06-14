#include "ws.h"
#include "sha.h"

#include "stdlib.h"
#include "strings.h"

#include <application.h>

namespace {

int eatWhite(const char* buf, int pos) {
    while (buf[pos] == ' ' || buf[pos] == '\t') {
        ++pos;
    }
    return pos;
}

int eatBlack(const char* buf, int pos) {
    while (buf[pos] != ' ' && buf[pos] != '\t' && buf[pos] != '\r' && buf[pos] != '\n') {
        ++pos;
    }
    return pos;
}

int eatUntilIncluding(const char* buf, int pos, char delim) {
    while (buf[pos] != 0 && buf[pos] != delim) {
        ++pos;
    }
    if (buf[pos] == delim) {
        ++pos;
    }
    return pos;
}

bool isWebsocketAcceptHeader(const char *buf) {
  return strncasecmp(buf, "sec-websocket-accept:", 20) == 0;
}

} // end anonymous namespace

namespace ws {

int getWebSocketKey(const char* buf, const char** key, int* length) {
  int pos = 0;
  pos = eatUntilIncluding(buf, pos, ' ');
  long code = strtol(buf+pos, NULL, 10);
  if (code != 101 || key == NULL) {
    return code;
  }
  pos = eatUntilIncluding(buf, pos, '\n');
  while (buf[pos] != 0) {
    if (buf[pos] == '\r' && buf[pos+1] == '\n') {
      return code;
    }
    if (isWebsocketAcceptHeader(buf+pos)) {
      pos = eatUntilIncluding(buf, pos, ':');
      int keypos = eatWhite(buf, pos);
      *key = buf+keypos;
      pos = eatBlack(buf, keypos);
      *length = pos - keypos;
      Serial.print("WEB SOCKET KEY: '");
      for(const char* p = *key; p != buf+pos; ++p) {
          Serial.print((char)*p);
      }
      pos = eatUntilIncluding(buf, keypos, '\n');
      Serial.println("'");
    }
    pos = eatUntilIncluding(buf, pos, '\n');
  }
  return code;
}

#ifndef MG_VPRINTF_BUFFER_SIZE
#define MG_VPRINTF_BUFFER_SIZE 100
#endif

void verifyHandshake(const char* key_p, uint32_t key_length) {
  static const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  char buf[MG_VPRINTF_BUFFER_SIZE], sha[20], b64_sha[sizeof(sha) * 2];
  sha1::Context sha_ctx;

  snprintf(buf, sizeof(buf), "%.*s%s", (int) key_length, key_p, magic);

  sha1::init(&sha_ctx);
  sha1::update(&sha_ctx, (unsigned char *) buf, strlen(buf));
  sha1::final((unsigned char *) sha, &sha_ctx);

  cs_base64_encode((unsigned char *) sha, sizeof(sha), b64_sha);
  Serial.print("ENCODED KEY: '");
  for(char* p = b64_sha; p != b64_sha+key_length; ++p) {
      Serial.print((char)*p);
  }  Serial.println("'");
}

int getFrameHeaderLength(uint32_t length, bool mask) {
  if (length < 126) {
    return 2 + 4*mask;
  } else if (length < 65536) {
    return 4 + 4*mask;
  } else {
    return 10 + 4*mask;
  }
}

void encodeFrame(const char* buf, uint32_t length, bool maskEnabled, char* dest) {
  int32_t mask = 0;
  if (maskEnabled) {
    mask = 0x80;
  }

  uint32_t i;

  // text frame, ignore close frame, ping and pong frames
  dest[0] = 0x80 | 0x01;

  // compose header
  if (length < 126) {
    dest[1] = mask | length;
    dest += 2;
  } else if (length < 65536) {
    dest[1] = mask | 0x7E;
    dest[2] = (length >> 8) & 0xFF;
    dest[3] = length & 0xFF;
    dest += 4;
  } else {
    dest[1] = mask | 0x7F;
    uint32_t len2 = length;
    for (i = 8; i > 0; --i) {
      dest[i+1] = len2 & 0xFF;
      len2 = len2 >> 8;
    }
    dest += 10;
  }

  if (mask) {
    // create random mask
    uint32_t ki;
    char *key = dest;
    for (ki = 0; ki < 4; ++ki) {
      key[ki] = rand() & 0xFF;
    }
    dest += 4;

    // mask buffer content
    for (i = 0, ki = 0; i < length; ++i) {
      dest[i] = buf[i] ^ key[ki];
      if (++ki > 3) ki = 0;
    }
  } else {
    std::copy(buf, buf+length, dest);
  }
}




} // end uhttp namespace
