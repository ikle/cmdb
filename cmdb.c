/*
 * Configuration Management Database
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <ctype.h>
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

const char **cmdb_list (struct cmdb *o, const char *name)
{
	if (!cmdb_path_set (&o->path, name))
		return 0;

	return cmdbs_list (o->db, o->path.path);
}

static int make_node (struct cmdb *o)
{
	struct cmdb_path backup, work;
	const char *name;

	cmdb_path_init (&backup);

	if (!cmdb_path_copy (&backup, &o->path))
		return 0;

	cmdb_path_init (&work);

	if (!cmdb_path_copy (&work, &o->path))
		goto no_work;

	while ((name = cmdb_path_pop (&work)) != NULL) {
		cmdb_path_pop (&o->path);

		if (cmdb_exists (o, "\n", name))
			break;

		if (!cmdb_store (o, "\n", name))
			goto no_store;
	}

	cmdb_path_fini (&work);
	cmdb_path_copy (&o->path, &backup);
	cmdb_path_fini (&backup);
	return 1;
no_store:
	cmdb_path_fini (&work);
	cmdb_path_copy (&o->path, &backup);
no_work:
	cmdb_path_fini (&backup);
	return 0;
}

int cmdb_store (struct cmdb *o, const char *name, const char *value)
{
	if (!make_node (o) ||
	    !cmdb_path_set (&o->path, name) ||
	    !cmdbs_store (o->db, o->path.path, value))
		return 0;

	if (iscntrl (name[0]))
		return 1;

	/* catalogue attributes */
	return cmdb_path_set (&o->path, "\a") &&
	       cmdbs_store (o->db, o->path.path, name);
}

static int drop_node (struct cmdb *o)
{
	const char *p;

	for (p = cmdb_first (o, "\n"); p != NULL; p = cmdb_next (o, "\n", p))
		if (cmdb_path_push (&o->path, p)) {
			drop_node (o);
			cmdb_path_pop (&o->path);
		}

	cmdb_delete (o, "\n", NULL);

	for (p = cmdb_first (o, "\a"); p != NULL; p = cmdb_next (o, "\a", p))
		cmdb_delete (o, p, NULL);

	cmdb_delete (o, "\a", NULL);
	return 1;
}

int cmdb_delete (struct cmdb *o, const char *name, const char *value)
{
	if (name == NULL)
		return drop_node (o);

	if (!cmdb_path_set (&o->path, name) ||
	    !cmdbs_delete (o->db, o->path.path, value))
		return 0;

	if (iscntrl (name[0]) || cmdbs_first (o->db, o->path.path) != NULL)
		return 1;

	/* remove empty attributes */
	return cmdb_path_set (&o->path, "\a") &&
	       cmdbs_delete (o->db, o->path.path, name);
}

int cmdb_flush (struct cmdb *o)
{
	return cmdbs_flush (o->db);
}
