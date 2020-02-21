#include "orconfig.h"

#include "lib/compress/compress.h"
#include "lib/compress/compress_brotli.h"

int tor_brotli_method_supported(void)
{
  return 0;
}

const char *tor_brotli_get_version_str(void)
{
  return NULL;
}

const char *tor_brotli_get_header_version_str(void)
{
  return NULL;
}

struct tor_brotli_compress_state_t {

  int is_encoding;
};

tor_brotli_compress_state_t *
tor_brotli_compress_new(int compress,
                      compress_method_t method,
                      compression_level_t compression_level)
{
  (void)compress;
  (void)method;
  (void)compression_level;

  return NULL;
}

tor_compress_output_t
tor_brotli_compress_process(tor_brotli_compress_state_t *state,
                          char **out, size_t *out_len,
                          const char **in, size_t *in_len,
                          int finish)
{
  (void)state;
  (void)out;
  (void)out_len;
  (void)in;
  (void)in_len;
  (void)finish;

  return TOR_COMPRESS_ERROR;
}

void tor_brotli_compress_free_(tor_brotli_compress_state_t *state)
{
  (void)state;
}

size_t tor_brotli_compress_state_size(const tor_brotli_compress_state_t *state)
{
  (void)state;

  return 0;
}

size_t tor_brotli_get_total_allocation(void)
{
  return 0;
}

void tor_brotli_init(void)
{

}
