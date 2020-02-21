#include "orconfig.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_brotli.h"
#include "lib/string/printf.h"

#ifdef HAVE_BROTLI_ENCODE_H
#include <brotli/encode.h>
#endif

#ifdef HAVE_BROTLI_DECODE_H
#include <brotli/decode.h>
#endif

int
tor_brotli_method_supported(void)
{
#if defined(HAVE_LIBBROTLIDEC) && defined(HAVE_LIBBROTLIENC)
  return 1;
#else
  return 0;
#endif
}
#define VERSION_STR_MAX_LEN 16 /* more than enough space for 99.99.99 */

const char *
tor_brotli_get_version_str(void)
{
#ifdef HAVE_BROTLI_DECODE_H
  static char str[VERSION_STR_MAX_LEN] = "";

  uint32_t version = BrotliDecoderVersion();

  tor_snprintf(str, sizeof(str), "%u.%u.%u",
      version >> 24, (version >> 12) & 0xFFF, version & 0xFFF);

  return str;
#else
  return NULL;
#endif
}

const char *
tor_brotli_get_header_version_str(void)
{
  /* libbrotli version is not available in the public headers. */
  return NULL;
}

struct tor_brotli_compress_state_t {
  union {
#ifdef HAVE_LIBBROTLIDEC
    BrotliDecoderState *decoder_state;
#endif
#ifdef HAVE_LIBBROTLIENC
    BrotliEncoderState *encoder_state;
#endif
  } u;

  int compress;
};

static void *
tor_brotli_alloc(void *opaque, size_t len)
{
  (void)opaque;
  // XXX: use memarea?
  return tor_malloc(len);
}

static void
tor_brotli_free(void *opaque, void *address)
{
  (void)opaque;

  tor_free_(address);
}

static uint32_t
tor_brotli_quality_from_compression_level(
    compression_level_t compression_level)
{
  switch (compression_level) {
    default:
    case BEST_COMPRESSION:
      return BROTLI_MAX_QUALITY;
    case HIGH_COMPRESSION:
      return BROTLI_MAX_QUALITY - 1;
    case MEDIUM_COMPRESSION:
      return (BROTLI_MAX_QUALITY + BROTLI_MIN_QUALITY) / 2;
    case LOW_COMPRESSION:
      return BROTLI_MIN_QUALITY + 1;
  }
}

tor_brotli_compress_state_t *
tor_brotli_compress_new(int compress,
                      compress_method_t method,
                      compression_level_t compression_level)
{
  tor_assert(method == BROTLI_METHOD);

  if (compress) {
#ifdef HAVE_LIBBROTLIENC
    tor_brotli_compress_state_t *result =
      tor_malloc_zero(sizeof(tor_brotli_compress_state_t));

    BrotliEncoderState *encoder_state = BrotliEncoderCreateInstance(
        tor_brotli_alloc, tor_brotli_free, NULL);

    if (!encoder_state) {
      log_warn(LD_GENERAL, "Error creating Brotli encoder instance");
      goto err;
    }

    BrotliEncoderSetParameter(encoder_state, BROTLI_PARAM_MODE,
        BROTLI_MODE_TEXT);
    BrotliEncoderSetParameter(encoder_state, BROTLI_PARAM_QUALITY,
        tor_brotli_quality_from_compression_level(compression_level));

    result->u.encoder_state = encoder_state;

    return result;
#endif
  } else {
#ifdef HAVE_LIBBROTLIDEC
    tor_brotli_compress_state_t *result =
      tor_malloc_zero(sizeof(tor_brotli_compress_state_t));

    BrotliDecoderState *decoder_state = BrotliDecoderCreateInstance(
        tor_brotli_alloc, tor_brotli_free, NULL);

    if (!decoder_state) {
      log_warn(LD_GENERAL, "Error creating Brotli decoder instance");
      goto err;
    }

    result->u.decoder_state = decoder_state;

    return result;
#endif
  }

 err:
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

void
tor_brotli_compress_free_(tor_brotli_compress_state_t *state)
{
  (void)state;
}

size_t
tor_brotli_compress_state_size(const tor_brotli_compress_state_t *state)
{
  (void)state;

  return 0;
}

size_t
tor_brotli_get_total_allocation(void)
{
  return 0;
}

void
tor_brotli_init(void)
{
}

