/*
 * Configuration Management Database
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CMDB_H
#define CMDB_H  1

#include <stddef.h>

struct cmdb *cmdb_open (const char *path, const char *mode);
int cmdb_close (struct cmdb *o);

const char *cmdb_error (struct cmdb *o);

int cmdb_push (struct cmdb *o, const char *name);
const char *cmdb_pop (struct cmdb *o);

/* pass NULL-terminated list of node names */
int cmdb_level (struct cmdb *o, ...);

int cmdb_exists (struct cmdb *o, const char *name, const char *value);
const char *cmdb_first (struct cmdb *o, const char *name);
const char *cmdb_next  (struct cmdb *o, const char *name, const char *value);

const char **cmdb_list (struct cmdb *o, const char *name);

int cmdb_store  (struct cmdb *o, const char *name, const char *value);
int cmdb_delete (struct cmdb *o, const char *name, const char *value);

int cmdb_flush (struct cmdb *o);

#endif  /* CMDB_H */
