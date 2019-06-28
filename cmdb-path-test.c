/*
 * Configuration Management Database Path Test
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>

#include "cmdb-path.h"

static void show (const char *path)
{
	const char *p;

	printf ("[");

	for (p = path; *p != '\0'; ++p)
		switch (*p) {
		case '\a':	printf (": "); break;
		case '\n':	putchar (' '); break;
		default:	putchar (*p);  break;
		}

	printf ("]\n");
}

int main (int argc, char *argv[])
{
	struct cmdb_path o, copy;
	size_t i;

	cmdb_path_init (&o);
	cmdb_path_init (&copy);

	show (o.path);

	if (!cmdb_path_push (&o, "interfaces") ||
	    !cmdb_path_push (&o, "ethernet eth1") ||
	    !cmdb_path_set (&o, "address"))
		errx (1, "cannot make path");

	show (o.path);

	if (!cmdb_path_copy (&copy, &o))
		errx (1, "cannot copy path");

	printf ("top = [%s]\n", cmdb_path_pop (&o));
	printf ("top = [%s]\n", cmdb_path_pop (&o));

	if (cmdb_path_pop (&o) != NULL)
		errx (1, "oops, path should be empty");

	cmdb_path_push (&copy, "route");
	show (copy.path);

	cmdb_path_push (&o, "route");
	cmdb_path_reset (&o);

	if (cmdb_path_pop (&o) != NULL)
		errx (1, "oops, path should be empty");

	for (i = 0; i < 20; ++i)
		if (!cmdb_path_push (&o, "abcdefgh01234567"))
			errx (1, "cannot make path");

	show (o.path);

	if (o.path == o.buf)
		errx (1, "oops, buffer overflow");

	cmdb_path_fini (&copy);
	cmdb_path_fini (&o);
	return 0;
}
