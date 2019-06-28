/*
 * Configuration Management Database Path
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CMDB_PATH_H
#define CMDB_PATH_H  1

#include <stddef.h>

struct cmdb_path {
	char buf[256], *path;
	size_t size, prefix, len;
};

void cmdb_path_init (struct cmdb_path *o);
void cmdb_path_fini (struct cmdb_path *o);

void cmdb_path_reset (struct cmdb_path *o);
int cmdb_path_copy (struct cmdb_path *o, struct cmdb_path *from);

int cmdb_path_push (struct cmdb_path *o, const char *name);
const char *cmdb_path_pop (struct cmdb_path *o);

int cmdb_path_set (struct cmdb_path *o, const char *name);

#endif  /* CMDB_PATH_H */
