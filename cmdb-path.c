/*
 * Configuration Management Database Path
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdb-path.h"

void cmdb_path_init (struct cmdb_path *o)
{
	o->path = o->buf;
	o->size = sizeof (o->buf);
	cmdb_path_reset (o);
}

void cmdb_path_fini (struct cmdb_path *o)
{
	if (o == NULL || o->path == o->buf)
		return;

	free (o->path);
}

void cmdb_path_reset (struct cmdb_path *o)
{
	o->path[o->len = o->prefix = 0] = '\0';
}

static int resize (struct cmdb_path *o, size_t size)
{
	char *p = o->path == o->buf ? NULL : o->path;

	if ((p = realloc (p, size)) == NULL)
		return 0;

	if (o->path == o->buf)
		strncpy (p, o->buf, o->size);

	o->path = p;
	o->size = size;
	return 1;
}

int cmdb_path_copy (struct cmdb_path *o, struct cmdb_path *from)
{
	size_t size = from->len + 1;

	if (size > o->size && !resize (o, size))
		return 0;

	memcpy (o->path, from->path, size);

	o->prefix = from->prefix;
	o->len    = from->len;
	return 1;
}

static int append (struct cmdb_path *o, int type, const char *name)
{
	char   *p   = o->path + o->prefix;
	size_t size = o->size - o->prefix;

	return o->prefix + snprintf (p, size, "%c%s", type, name);
}

int cmdb_path_push (struct cmdb_path *o, const char *name)
{
	size_t len;

	if ((len = append (o, '\n', name)) >= o->size) {
		if (!resize (o, len + 1))
			return 0;

		append (o, '\n', name);
	}

	o->len = o->prefix = len;
	return 1;
}

const char *cmdb_path_pop (struct cmdb_path *o)
{
	size_t i;

	if (o->prefix == 0)
		return NULL;  /* cannot pop from root */

	for (i = o->prefix - 1; o->path[i] != '\n'; --i)
		if (i == 0)
			return NULL;  /* broken path */

	o->path[i] = '\0';
	o->path[o->prefix] = '\0';
	o->len = o->prefix = i;
	return o->path + i + 1;
}

int cmdb_path_set (struct cmdb_path *o, const char *name)
{
	int type = '\a';
	size_t len;

	if (iscntrl (name[0]))
		type = name[0], ++name;

	if ((len = append (o, type, name)) == o->size) {
		if (!resize (o, len + 1))
			return 0;

		append (o, type, name);
	}

	o->len = len;
	return 1;
}
