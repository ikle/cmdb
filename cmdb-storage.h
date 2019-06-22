/*
 * Configuration Management Database Storage
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CMDB_STORAGE_H
#define CMDB_STORAGE_H  1

#include <stddef.h>

struct cmdbs *cmdbs_open (const char *path, const char *mode);
int cmdbs_close (struct cmdbs *o);

const char *cmdbs_error (struct cmdbs *o);

int cmdbs_exists (struct cmdbs *o, const char *key);
const char *cmdbs_first (struct cmdbs *o, const char *key);
const char *cmdbs_next  (struct cmdbs *o, const char *key, const char *value);

int cmdbs_store  (struct cmdbs *o, const char *key, const char *value);
int cmdbs_delete (struct cmdbs *o, const char *key, const char *value);

int cmdbs_flush (struct cmdbs *o);

#endif  /* CMDB_STORAGE_H */
