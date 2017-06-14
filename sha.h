#pragma once

#include <stdint.h>

namespace sha1 {

typedef struct {
  uint32_t state[5];
  uint32_t count[2];
  unsigned char buffer[64];
} Context;

void init(Context *context);
void update(Context *context, const unsigned char *data, uint32_t len);
void final(unsigned char digest[20], Context *context);

}


/*struct cs_base64_ctx {
  // cannot call it putc because it's a macro on some environments
  cs_base64_putc_t b64_putc;
  unsigned char chunk[3];
  int chunk_size;
  void *user_data;
};

void cs_base64_init(struct cs_base64_ctx *ctx, cs_base64_putc_t putc,
                    void *user_data);
void cs_base64_update(struct cs_base64_ctx *ctx, const char *str, size_t len);
void cs_base64_finish(struct cs_base64_ctx *ctx);
*/

void cs_base64_encode(const unsigned char *src, int src_len, char *dst);
/*void cs_fprint_base64(FILE *f, const unsigned char *src, int src_len);
int cs_base64_decode(const unsigned char *s, int len, char *dst);
*/
