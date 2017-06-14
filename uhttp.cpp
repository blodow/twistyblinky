#include "uhttp.h"
#include "stdlib.h"
#include "strings.h"

namespace {

int eatUntilIncluding(const char* buf, int pos, char delim) {
    while (buf[pos] != 0 && buf[pos] != delim) {
        ++pos;
    }
    if (buf[pos] == delim) {
        ++pos;
    }
    return pos;
}

bool isContentLengthHeader(const char *buf) {
  return strncasecmp(buf, "content-length:", 15) == 0;
}

} // end anonymous namespace

namespace uhttp {

int parseHeader(const char* buf, const char** body, int* length) {
  int pos = 0;
  pos = eatUntilIncluding(buf, pos, ' ');
  long code = strtol(buf+pos, NULL, 10);
  if (code != 200 || body == NULL) {
    return code;
  }
  pos = eatUntilIncluding(buf, pos, '\n');
  while (buf[pos] != 0) {
    if (buf[pos] == '\r' && buf[pos+1] == '\n') {
      pos += 2;
      *body = buf+pos;
      return code;
    }
    if (isContentLengthHeader(buf+pos)) {
      pos = eatUntilIncluding(buf, pos, ':');
      *length = strtol(buf+pos, NULL, 10);
    }
    pos = eatUntilIncluding(buf, pos, '\n');
  }
  return code;
}

} // end uhhtp namespace
