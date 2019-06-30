/*
 * Configuration Management Database Storage Test
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <err.h>

#include "cmdb-storage.h"

static void show (struct cmdbs *o, const char *key)
{
	const char *p;

	for (p = cmdbs_first (o, key); p != NULL; p = cmdbs_next (o, key, p))
		printf ("%s = %s\n", key, p);
}

static void show_sorted (struct cmdbs *o, const char *key)
{
	const char **list, **p;

	if ((list = cmdbs_list (o, key)) == NULL)
		errx (1, "cannot fetch list for %s", key);

	for (p = list; *p != NULL; ++p)
		printf ("%s = %s\n", key, *p);

	free (list);
}

int main (int argc, char *argv[])
{
	struct cmdbs *o;

	if ((o = cmdbs_open ("cmdbs-test.db", "rwx")) == NULL)
		errx (1, "cannot open database");

	printf ("address %sfound\n",
		cmdbs_exists (o, "address", NULL) ? "" : "not ");

	if (!cmdbs_store (o, "address", "10.0.26.3/24") ||
	    !cmdbs_store (o, "address", "10.0.26.7/24") ||
	    !cmdbs_store (o, "address", "10.0.26.4/24"))
		errx (1, "cannot store: %s", cmdbs_error (o));

	if (!cmdbs_flush (o))
		errx (1, "cannot flush: %s", cmdbs_error (o));

	show_sorted (o, "address");
	show (o, "hostname");

	if (!cmdbs_store (o, "hostname", "cmdb-test"))
		errx (1, "cannot store: %s", cmdbs_error (o));

	cmdbs_close (o);
	return 0;
}
