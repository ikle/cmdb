/*
 * Configuration Management Database Cache
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <data/hash.h>
#include <data/ht.h>
#include <data/string.h>

#include "cmdb-cache.h"

static const struct data_type string_type = {
	.free	= free,
	.eq	= string_eq,
	.hash	= string_hash,
};

struct record {
	char *key;
	struct ht set;
	int changed;
};

static struct record *record_alloc (const char *key)
{
	struct record *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if ((o->key = strdup (key)) == NULL)
		goto no_key;

	if (!ht_init (&o->set, &string_type))
		goto no_set;

	o->changed = 0;
	return o;
no_set:
	free (o->key);
no_key:
	free (o);
	return NULL;
}

static void record_free (struct record *o)
{
	if (o == NULL)
		return;

	ht_fini (&o->set);
	free (o->key);
	free (o);
}

static void record_drop (void *o)
{
	record_free (o);
}

static int record_eq (const void *a, const void *b)
{
	const struct record *p = a;
	const struct record *q = b;

	return strcmp (p->key, q->key) == 0;
}

static size_t record_hash (const void *o)
{
	const struct record *p = o;

	return hash (0, p->key, strlen (p->key));
}

static const struct data_type record_type = {
	.free	= record_drop,
	.eq	= record_eq,
	.hash	= record_hash,
};

struct cmdbc {
	struct ht root;
};

struct cmdbc *cmdbc_alloc (void)
{
	struct cmdbc *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if (!ht_init (&o->root, &record_type))
		goto no_root;

	return o;
no_root:
	free (o);
	return NULL;
}

void cmdbc_free (struct cmdbc *o)
{
	if (o == NULL)
		return;

	ht_fini (&o->root);
	free (o);
}

int cmdbc_store (struct cmdbc *o, const char *key, const char *value)
{
	const struct record sample = { (char *) key };
	struct record *r;
	char *v;

	if ((r = ht_lookup (&o->root, &sample)) == NULL) {
		if ((r = record_alloc (key)) == NULL ||
		    !ht_insert (&o->root, r, 0))
			return 0;
	}

	if ((v = strdup (value)) == NULL)
		return 0;

	if (!ht_insert (&r->set, v, 1)) {
		free (v);
		return 0;
	}

	r->changed = 1;
	return 1;
}

void cmdbc_delete (struct cmdbc *o, const char *key, const char *value)
{
	const struct record sample = { (char *) key };
	struct record *r;

	if (value == NULL) {
		ht_remove (&o->root, &sample);
		return;
	}

	if ((r = ht_lookup (&o->root, &sample)) == NULL)
		return;

	ht_remove (&r->set, value);
	r->changed = 1;
}

size_t cmdbc_count (struct cmdbc *o, const char *key)
{
	const struct record sample = { (char *) key }, *r;
	size_t count, i;

	if ((r = ht_lookup (&o->root, &sample)) == NULL)
		return 0;

	for (count = 0, i = 0; i < r->set.size; ++i)
		if (r->set.table[i] != NULL)
			++count;

	return count;
}

const char *cmdbc_first (struct cmdbc *o, const char *key)
{
	const struct record sample = { (char *) key }, *r;
	size_t i;

	if ((r = ht_lookup (&o->root, &sample)) == NULL)
		return NULL;

	/* find first entry */
	for (i = 0; i < r->set.size; ++i)
		if (r->set.table[i] != NULL)
			return r->set.table[i];

	return NULL;
}

const char *cmdbc_next  (struct cmdbc *o, const char *key, const char *value)
{
	const struct record sample = { (char *) key }, *r;
	size_t i;

	if ((r = ht_lookup (&o->root, &sample)) == NULL)
		return NULL;

	i = ht_index (&r->set, value);
	if (r->set.table[i] == NULL)
		return NULL;

	/* find next entry */
	for (++i; i < r->set.size; ++i)
		if (r->set.table[i] != NULL)
			return r->set.table[i];

	return NULL;
}

int cmdbc_import (struct cmdbc *o, const char *key, const void *data,
		  size_t size)
{
	const struct record sample = { (char *) key };
	struct record *r;
	const char *p;
	size_t avail, len;
	char *v;

	if ((r = ht_lookup (&o->root, &sample)) == NULL) {
		if ((r = record_alloc (key)) == NULL ||
		    !ht_insert (&o->root, r, 0))
			return 0;
	}

	for (
		p = data, avail = size;
		(len = strnlen (p, avail)) < avail;
		++len, p += len, avail -= len
	) {
		if ((v = strdup (p)) == NULL)
			return 0;

		if (ht_insert (&r->set, v, 1))
			continue;

		free (v);
		return 0;
	}

	return 1;
}

size_t cmdbc_export (const struct cmdbc *o, const char *key, void *data,
		     size_t avail)
{
	const struct record sample = { (char *) key }, *r;
	size_t size, i, len;
	const char *entry;
	char *p;

	if ((r = ht_lookup (&o->root, &sample)) == NULL)
		return 0;

	for (size = 0, i = 0; i < r->set.size; ++i)
		if ((entry = r->set.table[i]) != NULL)
			size += snprintf (NULL, 0, "%s", entry) + 1;

	if (size > avail)
		return size;

	for (p = data, i = 0; i < r->set.size; ++i)
		if ((entry = r->set.table[i]) != NULL) {
			len = snprintf (p, avail, "%s", entry) + 1;
			p += len, avail -= len;
		}

	return size;
}

int cmdbc_flush (struct cmdbc *o, cmdbc_visitor *fn, void *cookie)
{
	size_t i;
	struct record *r;

	for (i = 0; i < o->root.size; ++i)
		if ((r = o->root.table[i]) != NULL && r->changed) {
			if (!fn (o, r->key, cookie))
				return 0;
			else
				r->changed = 0;
		}

	return 1;
}
