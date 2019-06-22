/*
 * Configuration Management Database Cache
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CMDB_CACHE_H
#define CMDB_CECHE_H  1

#include <stddef.h>

struct cmdbc *cmdbc_alloc (void);
void cmdbc_free (struct cmdbc *o);

size_t cmdbc_count (struct cmdbc *o, const char *key);
const char *cmdbc_first (struct cmdbc *o, const char *key);
const char *cmdbc_next  (struct cmdbc *o, const char *key, const char *value);

int  cmdbc_store  (struct cmdbc *o, const char *key, const char *value);
void cmdbc_delete (struct cmdbc *o, const char *key, const char *value);

int cmdbc_import (struct cmdbc *o, const char *key, const void *data,
		  size_t size);
size_t cmdbc_export (const struct cmdbc *o, const char *key, void *data,
		     size_t size);

typedef int cmdbc_visitor (struct cmdbc *o, const char *key, void *cookie);

int cmdbc_flush (struct cmdbc *o, cmdbc_visitor *fn, void *cookie);

#endif  /* CMDB_CACHE_H */
