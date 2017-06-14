#include <stdlib.h>
#include <math.h>
#include <string.h>

namespace {

bool match(const char* buf, int pos, const char* tag, const char* tagEnd) {
    return (buf[pos] == '<' && strncmp(buf+pos+1, tag, tagEnd-tag) == 0);
}

int getTagNameEnd(const char* buf, int start) {
    while (buf[start] != 0 && buf[start] != ' ' && buf[start] != '>') {
        ++start;
    }
    return start;
}

int eatOpenTag(const char* buf, int pos) {
    while (buf[pos] != 0 && buf[pos] != '>') {
        ++pos;
    }
    if (buf[pos] != 0) {
        ++pos;
    }
    return pos;
}

int eatCloseTag(const char* buf, int pos) {
    const char* tag = buf+pos+1;
    pos = getTagNameEnd(buf, pos);
    const char* tagEnd = buf+pos;
    // For now, simply look for any matching closing tag.
    while (buf[pos] != 0) {
        if (buf[pos] == '<' && buf[pos+1] == '/' && buf[pos+2+(tagEnd-tag)] == '>' && strncmp(buf+pos+2, tag, tagEnd-tag) == 0) {
          return pos+3+tagEnd-tag;
        }
        ++pos;
    }
    return pos;
}

int eatText(const char *buf, int pos, const char* tag, const char* tagEnd) {
    while (buf[pos] != 0) {
        if (buf[pos] == '<' && buf[pos+1] == '/' && buf[pos+2+(tagEnd-tag)] == '>' && strncmp(buf+pos+2, tag, tagEnd-tag) == 0) {
          return pos;
        }
        ++pos;
    }
    return pos;
}

} // end anonymous namespace

bool getText(const char* buf, const char *path[], int path_items, const char** text, const char** textEnd) {
  int bufLen = strlen(buf);
  int path_pos = 0;
  int pos = 0;

  while(pos < bufLen) {
      const char *tag = path[path_pos];
      const char* tagEnd = tag + strlen(path[path_pos]);
      if (match(buf, pos, tag, tagEnd)) {
          pos = eatOpenTag(buf, pos);
          if (buf[pos] != 0) {
            if (path_pos + 1 >= path_items) {
              int end = eatText(buf, pos, tag, tagEnd);
              *text = buf+pos;
              *textEnd = buf+end;
              return true;
            } else {
              path_pos += 1;
            }
          } else {
            return false;
          }
      } else {
          pos = eatCloseTag(buf, pos);
      }
  }
  return false;
}

long strtola (const char* start, const char* end) {
    char lenStr[1+end-start];
    memcpy(lenStr, start, end-start);
    lenStr[end-start] = 0;
    return strtol(lenStr, NULL, 10);
}
