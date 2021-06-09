/*
 * Configuration Management Database Storage
 *
 * Copyright (c) 2019-2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <tdb.h>

#include "cmdb-cache.h"
#include "cmdb-storage.h"

static int make_path (const char *path)
{
	size_t size = strlen (path) + 1;
	char line[size], *p;

	if (path == NULL || path[0] == '\0') {
		errno = EINVAL;
		return 0;
	}

	strncpy (line, path, size);

	for (p = line + 1; *p != '\0'; ++p)
		if (*p == '/') {
			*p = '\0';

			if (mkdir (line, 0777) != 0 && errno != EEXIST)
				return 0;

			*p = '/';
		}

	return 1;
}

struct cmdbs {
	struct cmdbc *cache;
	TDB_CONTEXT *db;
};

struct cmdbs *cmdbs_open (const char *path, const char *mode)
{
	struct cmdbs *o;
	int flags = O_RDONLY;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if ((o->cache = cmdbc_alloc ()) == NULL)
		goto no_cache;

	for (; *mode != '\0'; ++mode)
		if (*mode == 'w') {
			flags = O_RDWR | O_CREAT;

			if (!make_path (path))
				goto no_db;
		}

	if ((o->db = tdb_open (path, 0, 0, flags, 0666)) == NULL)
		goto no_db;

	return o;
no_db:
	cmdbc_free (o->cache);
no_cache:
	free (o);
	return NULL;
}

int cmdbs_close (struct cmdbs *o)
{
	int ret = 1;

	if (o == NULL)
		return ret;

	if (!cmdbs_flush (o))
		ret = 0;

	if (tdb_close (o->db) != 0)
		ret = 0;

	cmdbc_free (o->cache);
	free (o);
	return ret;
}

const char *cmdbs_error (struct cmdbs *o)
{
	return tdb_errorstr (o->db);
}

int cmdbs_exists (struct cmdbs *o, const char *key, const char *value)
{
	if (cmdbs_first (o, key) == NULL)
		return 0;

	return cmdbc_exists (o->cache, key, value);
}

const char *cmdbs_first (struct cmdbs *o, const char *key)
{
	TDB_DATA k, v;
	int ret;

	if (cmdbc_exists (o->cache, key, NULL))
		return cmdbc_first (o->cache, key);

	k.dptr = (void *) key;
	k.dsize = strlen (key) + 1;
	v = tdb_fetch (o->db, k);

	if (v.dptr == NULL)
		return NULL;

	ret = cmdbc_import (o->cache, key, v.dptr, v.dsize);
	free (v.dptr);

	if (!ret)
		return NULL;

	return cmdbc_first (o->cache, key);
}

const char *cmdbs_next (struct cmdbs *o, const char *key, const char *value)
{
	return cmdbc_next (o->cache, key, value);
}

const char **cmdbs_list (struct cmdbs *o, const char *key)
{
	if (cmdbs_first (o, key) == NULL)
		return NULL;

	return cmdbc_list (o->cache, key);
}

int cmdbs_store (struct cmdbs *o, const char *key, const char *value)
{
	return cmdbc_store (o->cache, key, value);
}

int cmdbs_delete (struct cmdbs *o, const char *key, const char *value)
{
	cmdbc_delete (o->cache, key, value);
	return 1;
}

static int writer (struct cmdbc *cache, const char *key, void *cookie)
{
	struct cmdbs *o = cookie;
	TDB_DATA k, v;
	int ret;

	k.dptr  = (void *) key;
	k.dsize = strlen (key) + 1;

	if ((v.dsize = cmdbc_export (cache, key, NULL, 0)) == 0)
		/* drop empty nodes */
		return tdb_delete (o->db, k) == 0;

	if ((v.dptr = malloc (v.dsize)) == NULL)
		return 0;

	cmdbc_export (cache, key, v.dptr, v.dsize);

	ret = tdb_store (o->db, k, v, TDB_REPLACE) == 0;
	free (v.dptr);
	return ret;
}

int cmdbs_flush (struct cmdbs *o)
{
	return cmdbc_flush (o->cache, writer, o);
}
