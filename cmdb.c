/*
 * Configuration Management Database
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdarg.h>
#include <stdlib.h>

#include "cmdb.h"
#include "cmdb-path.h"
#include "cmdb-storage.h"

struct cmdb {
	struct cmdbs *db;
	struct cmdb_path path;
};

struct cmdb *cmdb_open (const char *path, const char *mode)
{
	struct cmdb *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if ((o->db = cmdbs_open (path, mode)) == NULL)
		goto no_db;

	cmdb_path_init (&o->path);
	return o;
no_db:
	return NULL;
}

int cmdb_close (struct cmdb *o)
{
	int ret = 1;

	if (o == NULL)
		return ret;

	cmdb_path_fini (&o->path);

	if (!cmdbs_close (o->db))
		ret = 0;

	free (o);
	return ret;
}

const char *cmdb_error (struct cmdb *o)
{
	return cmdbs_error (o->db);
}

int cmdb_push (struct cmdb *o, const char *name)
{
	return cmdb_path_push (&o->path, name);
}

const char *cmdb_pop (struct cmdb *o)
{
	return cmdb_path_pop (&o->path);
}

/* pass NULL-terminated list of node names */
int cmdb_level (struct cmdb *o, ...)
{
	va_list ap;
	const char *p;

	cmdb_path_reset (&o->path);

	va_start (ap, o);

	while ((p = va_arg (ap, const char *)) != NULL)
		if (p[0] == '\0' || !cmdb_path_push (&o->path, p))
			goto error;

	va_end (ap);
	return 1;
error:
	va_end (ap);
	return 0;
}

int cmdb_exists (struct cmdb *o, const char *name, const char *value)
{
	if (!cmdb_path_set (&o->path, name))
		return 0;

	return cmdbs_exists (o->db, o->path.path, value);
}

const char *cmdb_first (struct cmdb *o, const char *name)
{
	if (!cmdb_path_set (&o->path, name))
		return 0;

	return cmdbs_first (o->db, o->path.path);
}

const char *cmdb_next (struct cmdb *o, const char *name, const char *value)
{
	if (!cmdb_path_set (&o->path, name))
		return 0;

	return cmdbs_next (o->db, o->path.path, value);
}

int cmdb_store (struct cmdb *o, const char *name, const char *value)
{
	if (!cmdb_path_set (&o->path, name))
		return 0;

	return cmdbs_store (o->db, o->path.path, value);
}

int cmdb_delete (struct cmdb *o, const char *name, const char *value)
{
	if (!cmdb_path_set (&o->path, name))
		return 0;

	return cmdbs_delete (o->db, o->path.path, value);
}

int cmdb_flush (struct cmdb *o)
{
	return cmdbs_flush (o->db);
}
