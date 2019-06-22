/*
 * Configuration Management Database Cache Test
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>

#include "cmdb-cache.h"

static const char *show_path (const char *path)
{
	const char *p;
	int len;

	for (p = path; *p != '\0'; p += len) {
		len = strcspn (p, "\a\n");

		printf ("%.*s", len, p);

		switch (p[len]) {
		case '\a':
			printf (":\n");
			return p + len + 1;
		case '\n':
			++len, putchar (' ');
		}
	}

	printf (":\n");
	return "(anon)";
}

static int show (struct cmdbc *o, const char *key, void *cookie)
{
	void *data;
	size_t size;
	const char *attr, *p;
	size_t avail, len;

	printf ("scan, ");
	attr = show_path (key);

	for (p = cmdbc_first (o, key); p != NULL; p = cmdbc_next (o, key, p))
		printf ("\t%s = %s\n", attr, p);;

	if ((size = cmdbc_export (o, key, &data)) == 0)
		return 0;

	printf ("export, ");
	attr = show_path (key);

	for (
		p = data, avail = size;
		(len = strnlen (p, avail)) < avail;
		++len, p += len, avail -= len
	)
		printf ("\t%s = %s\n", attr, p);

	free (data);
	return 1;
}

static const char *address  = "interfaces\nethernet eth1\aaddress";
static const char *hostname = "system\ahostname";

int main (int argc, char *argv[])
{
	struct cmdbc *o;

	if ((o = cmdbc_alloc ()) == NULL)
		err (1, "cannot create cache");

	cmdbc_store (o, hostname, "cmdb-cache-test");
	cmdbc_store (o, address, "10.0.26.3/24");
	cmdbc_store (o, address, "10.0.26.4/24");
	cmdbc_store (o, address, "10.0.26.5/24");
	cmdbc_store (o, address, "10.0.26.6/24");
	cmdbc_store (o, address, "10.0.26.4/24");
	cmdbc_store (o, address, "10.0.26.11/24");

	cmdbc_flush (o, show, NULL);

	cmdbc_store (o, "service\nssh\aaddress",  "192.168.0.1");
	cmdbc_store (o, "service\nssh\acipher",   "magma-cbc");
	cmdbc_store (o, "service\nsnmp\aaddress", "0.0.0.0");

	cmdbc_flush (o, show, NULL);

	cmdbc_free (o);
	return 0;
}
