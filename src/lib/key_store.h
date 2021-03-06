/*
 * Copyright (c) 2017, [Ribose Inc](https://www.ribose.com).
 * All rights reserved.
 *
 * This code is originally derived from software contributed to
 * The NetBSD Foundation by Alistair Crooks (agc@netbsd.org), and
 * carried further by Ribose Inc (https://www.ribose.com).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KEY_STORE_H_
#define KEY_STORE_H_

#include <rnp.h>
#include <json.h>

#include <stdint.h>

#include "packet.h"
#include "memory.h"

typedef struct rnp_key_store_t {
    DYNARRAY(pgp_key_t, key);
    pgp_hash_alg_t hashtype;
} rnp_key_store_t;

int rnp_key_store_load_keys(rnp_t *rnp, char *homedir);

int rnp_key_store_load_from_file(rnp_t *rnp, rnp_key_store_t *, const unsigned, const char *);
int rnp_key_store_load_from_mem(rnp_t *rnp, rnp_key_store_t *, const unsigned, pgp_memory_t *);

void rnp_key_store_free(rnp_key_store_t *);

int rnp_key_store_list(pgp_io_t *, const rnp_key_store_t *, const int);
int rnp_key_store_json(pgp_io_t *, const rnp_key_store_t *, json_object *, const int);

int rnp_key_store_append_keyring(rnp_key_store_t *, rnp_key_store_t *);

int rnp_key_store_add_key(pgp_io_t *, rnp_key_store_t *, pgp_key_t *, pgp_content_enum);
int rnp_key_store_add_keydata(pgp_io_t *,
                              rnp_key_store_t *,
                              pgp_keydata_key_t *,
                              pgp_content_enum);

int rnp_key_store_remove_key(pgp_io_t *, rnp_key_store_t *, const pgp_key_t *);
int rnp_key_store_remove_key_by_id(pgp_io_t *, rnp_key_store_t *, const uint8_t *);

const pgp_key_t *rnp_key_store_get_key_by_id(
  pgp_io_t *, const rnp_key_store_t *, const unsigned char *, unsigned *, pgp_pubkey_t **);
const pgp_key_t *rnp_key_store_get_key_by_name(pgp_io_t *,
                                               const rnp_key_store_t *,
                                               const char *);
const pgp_key_t *rnp_key_store_get_next_key_by_name(pgp_io_t *,
                                                    const rnp_key_store_t *,
                                                    const char *,
                                                    unsigned *);

#endif /* KEY_STORE_H_ */
