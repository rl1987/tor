#include "orconfig.h"

#include "lib/log/log.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/compress/compress.h"
#include "lib/compress/compress_brotli.h"
#include "lib/string/printf.h"
#include "lib/thread/threads.h"

#ifdef HAVE_BROTLI_ENCODE_H
#include <brotli/encode.h>
#endif

#ifdef HAVE_BROTLI_DECODE_H
#include <brotli/decode.h>
#endif

static atomic_counter_t total_brotli_allocation;

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

  size_t input_so_far;
  size_t output_so_far;
};

static void *
tor_brotli_alloc(void *opaque, size_t len)
{
  (void)opaque;
  // XXX: this is not included into total_brotli_allocation
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

    atomic_counter_add(&total_brotli_allocation,
        tor_brotli_compress_state_size(result));

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

    atomic_counter_add(&total_brotli_allocation,
        tor_brotli_compress_state_size(result));

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
  tor_assert(state);

  size_t old_in_len = *in_len;

  if (state->compress) {
#if HAVE_LIBBROTLIENC
    BrotliEncoderOperation op =
      finish ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_FLUSH;

    size_t total_out = 0;

    BROTLI_BOOL success = BrotliEncoderCompressStream(state->u.encoder_state,
        op, in_len, (const uint8_t **)in, out_len, (uint8_t **)out,
        &total_out);

    if (success) {
      return TOR_COMPRESS_OK;
    } else {
      if (BrotliEncoderHasMoreOutput(state->u.encoder_state)) {
        return TOR_COMPRESS_BUFFER_FULL;
      } else {
        return TOR_COMPRESS_ERROR;
      }
    }
#endif
  } else {
#if HAVE_LIBBROTLIDEC
    size_t total_out = 0;

    BROTLI_BOOL success = BrotliDecoderDecompressStream(state->u.decoder_state,
        in_len, (const uint8_t **)in, out_len, (uint8_t **)out, &total_out);

    state->input_so_far += old_in_len - *in_len;
    state->output_so_far = total_out;

    if (tor_compress_is_compression_bomb(state->input_so_far,
                                         state->output_so_far)) {
      log_warn(LD_DIR, "Possible compression bomb; abandoning Brotli stream.");
      return TOR_COMPRESS_ERROR;
    }

    if (success) {
      return TOR_COMPRESS_OK;
    } else {
      if (BrotliDecoderHasMoreOutput(state->u.decoder_state)) {
        return TOR_COMPRESS_BUFFER_FULL;
      } else {
        return TOR_COMPRESS_ERROR;
      }
    }
#endif
  }

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
  tor_assert(state);

  atomic_counter_sub(&total_brotli_allocation,
      tor_brotli_compress_state_size(state));

#ifdef HAVE_LIBBROTLIENC
  if (state->u.encoder_state)
    BrotliEncoderDestroyInstance(state->u.encoder_state);
#endif
#ifdef HAVE_LIBBROTLIDEC
  if (state->u.decoder_state)
    BrotliDecoderDestroyInstance(state->u.decoder_state);
#endif

  tor_free(state);
}

size_t
tor_brotli_compress_state_size(const tor_brotli_compress_state_t *state)
{
  tor_assert(state);

  size_t allocated = sizeof(state);

  // XXX: this does not include size of encoder/decoder instance and
  // it's internal buffers.

  return allocated;
}

size_t
tor_brotli_get_total_allocation(void)
{
  return atomic_counter_get(&total_brotli_allocation);
}

void
tor_brotli_init(void)
{
  atomic_counter_init(&total_brotli_allocation);
}

