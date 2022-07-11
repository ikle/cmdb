/*
 * Configuration Management Database Test
 *
 * Copyright (c) 2019-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <err.h>

#include "cmdb.h"
#include "cmdb-util.h"

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
	    !cmdb_store (o, "address", "10.0.26.7/24") ||
	    !cmdb_store (o, "lldp", "on"))
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
	printf ("-------- show --------\n");
	show (o, 0);

	if (!cmdb_level (o, "interfaces", "ethernet eth1", NULL) ||
	    !cmdb_delete (o, "address", "10.0.26.3/24"))
		errx (1, "cannot delete: %s", cmdb_error (o));

	if (!cmdb_flush (o))
		errx (1, "cannot flush: %s", cmdb_error (o));

	if (!cmdb_level (o, NULL))
		errx (1, "cannot set level");

	printf ("\n");
	printf ("-------- save --------\n");
	cmdb_save (o, stdout);
	cmdb_close (o);
	return 0;
}
