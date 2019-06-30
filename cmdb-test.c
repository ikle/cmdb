/*
 * Configuration Management Database Test
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <err.h>

#include "cmdb.h"

static void show_attr (struct cmdb *o, int level, const char *name)
{
	const char *p;

	for (p = cmdb_first (o, name); p != NULL; p = cmdb_next (o, name, p))
		printf ("%*s%s = %s\n", level * 4, "", name, p);
}

static void show (struct cmdb *o, int level)
{
	const char *p;

	for (p = cmdb_first (o, "\a"); p != NULL; p = cmdb_next (o, "\a", p))
		show_attr (o, level, p);

	for (p = cmdb_first (o, "\n"); p != NULL; p = cmdb_next (o, "\n", p)) {
		printf ("%*s%s:\n", level * 4, "", p);

		if (cmdb_push (o, p)) {
			show (o, level + 1);
			cmdb_pop (o);
		}
	}
}

static void show_attr_sorted (struct cmdb *o, int level, const char *name)
{
	const char **list, **p;

	if ((list = cmdb_list (o, name)) == NULL)
		return;

	for (p = list; *p != NULL; ++p)
		printf ("%*s%s = %s\n", level * 4, "", name, *p);

	free (list);
}

static void show_sorted (struct cmdb *o, int level)
{
	const char **list, **p;

	if ((list = cmdb_list (o, "\a")) != NULL) {
		for (p = list; *p != NULL; ++p)
			show_attr_sorted (o, level, *p);

		free (list);
	}

	if ((list = cmdb_list (o, "\n")) != NULL) {
		for (p = list; *p != NULL; ++p) {
			printf ("%*s%s:\n", level * 4, "", *p);

			if (cmdb_push (o, *p)) {
				show_sorted (o, level + 1);
				cmdb_pop (o);
			}
		}

		free (list);
	}
}

int main (int argc, char *argv[])
{
	struct cmdb *o;

	if ((o = cmdb_open ("cmdb-test.db", "rwx")) == NULL)
		errx (1, "cannot open database");

	if (!cmdb_level (o, "interfaces", "ethernet eth1", NULL))
		errx (1, "cannot set level");

	printf ("address %sfound\n",
		cmdb_exists (o, "address", NULL) ? "" : "not ");

	if (!cmdb_store (o, "address", "10.0.26.3/24") ||
	    !cmdb_store (o, "address", "10.0.26.4/24") ||
	    !cmdb_store (o, "address", "10.0.26.7/24"))
		errx (1, "cannot store: %s", cmdb_error (o));

	if (!cmdb_flush (o))
		errx (1, "cannot flush: %s", cmdb_error (o));

	if (!cmdb_level (o, "system", NULL))
		errx (1, "cannot set level");

	if (!cmdb_store (o, "hostname", "cmdb-test"))
		errx (1, "cannot store: %s", cmdb_error (o));

	if (!cmdb_level (o, NULL))
		errx (1, "cannot set level");

	printf ("\n");
	show (o, 0);

	if (!cmdb_level (o, "interfaces", "ethernet eth1", NULL) ||
	    !cmdb_delete (o, "address", "10.0.26.3/24"))
		errx (1, "cannot delete: %s", cmdb_error (o));

	if (!cmdb_flush (o))
		errx (1, "cannot flush: %s", cmdb_error (o));

	if (!cmdb_level (o, NULL))
		errx (1, "cannot set level");

	printf ("\n");
	show_sorted (o, 0);
	cmdb_close (o);
	return 0;
}
