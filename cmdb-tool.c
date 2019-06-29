/*
 * Configuration Management Database Tool
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>

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

static int usage (void)
{
	fprintf (stderr,
		 "usage:\n\tcmdb <database> [<command> [, <command>] ...]\n"
		 "\n"
		 "commands:\n"
		 "\tlevel [<node> ...]\n"
		 "\tstore <attr> <value>\n"
		 "\tdelete <attr> [<value>]\n"
		 "\tshow\n");

	return 1;
}

static int do_level (struct cmdb *o, char **argv)
{
	int i;

	if (!cmdb_level (o, NULL))
		errx (1, "cannot set level");

	for (i = 1; argv[i] != NULL && strcmp (argv[i], ",") != 0; ++i)
		if (!cmdb_push (o, argv[i]))
			errx (1, "cannot set level");

	return i;
}

static int do_store (struct cmdb *o, char **argv)
{
	const char *attr, *value;

	++argv;

	if (*argv == NULL || strcmp (*argv, ",") == 0)
		errx (1, "cmdb store: attribute name required");

	attr = *argv++;

	if (*argv == NULL || strcmp (*argv, ",") == 0)
		errx (1, "cmdb store: value required");

	value = *argv;

	if (!cmdb_store (o, attr, value))
		errx (1, "cmdb store: %s", cmdb_error (o));

	return 3;
}

static int do_delete (struct cmdb *o, char **argv)
{
	const char *attr, *value = NULL;
	int n = 2;

	++argv;

	if (*argv == NULL || strcmp (*argv, ",") == 0)
		errx (1, "cmdb delete: attribute name required");

	attr = *argv++;

	if (*argv != NULL && strcmp (*argv, ",") != 0) {
		value = *argv;
		++n;
	}

	if (!cmdb_delete (o, attr, value))
		errx (1, "cmdb delete: %s", cmdb_error (o));

	return n;
}

static int do_show (struct cmdb *o, char **argv)
{
	show (o, 0);
	return 1;
}

int main (int argc, char *argv[])
{
	struct cmdb *o;
	int n;

	if (argc < 2)
		return usage ();

	if ((o = cmdb_open (argv[1], "rwx")) == NULL)
		errx (1, "cannot open database");

	for (argc -= 2, argv += 2; argc > 0; argc -= n, argv += n)
		if (strcmp (argv[0], ",") == 0)
			n = 1;
		else if (strcmp (argv[0], "level") == 0)
			n = do_level (o, argv);
		else if (strcmp (argv[0], "store") == 0)
			n = do_store (o, argv);
		else if (strcmp (argv[0], "delete") == 0)
			n = do_delete (o, argv);
		else if (strcmp (argv[0], "show") == 0)
			n = do_show (o, argv);
		else
			errx (1, "unknown command: %s", argv[0]);

	cmdb_close (o);
	return 0;
}
