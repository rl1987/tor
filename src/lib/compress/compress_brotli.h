/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compress_brotli.h
 * \brief Header for compress_brotli.c
 **/

#ifndef TOR_COMPRESS_LZMA_H
#define TOR_COMPRESS_LZMA_H

int tor_brotli_method_supported(void);

const char *tor_brotli_get_version_str(void);

const char *tor_brotli_get_header_version_str(void);

/** Internal state for an incremental LZMA compression/decompression. */
typedef struct tor_brotli_compress_state_t tor_brotli_compress_state_t;

tor_brotli_compress_state_t *
tor_brotli_compress_new(int compress,
                      compress_method_t method,
                      compression_level_t compression_level);

tor_compress_output_t
tor_brotli_compress_process(tor_brotli_compress_state_t *state,
                          char **out, size_t *out_len,
                          const char **in, size_t *in_len,
                          int finish);

void tor_brotli_compress_free_(tor_brotli_compress_state_t *state);
#define tor_brotli_compress_free(st)                      \
  FREE_AND_NULL(tor_brotli_compress_state_t,   \
                           tor_brotli_compress_free_, (st))

size_t tor_brotli_compress_state_size(const tor_brotli_compress_state_t *state);

size_t tor_brotli_get_total_allocation(void);

void tor_brotli_init(void);

#endif /* !defined(TOR_COMPRESS_LZMA_H) */

