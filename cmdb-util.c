/*
 * Configuration Management Database Helpers
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include "cmdb-util.h"

static void indent (FILE *to, int level)
{
	for (; level > 0; --level)
		fputc ('\t', to);
}

static void show_attr (struct cmdb *o, FILE *to, int level, const char *name)
{
	const char **list, **p;

	if ((list = cmdb_list (o, name)) == NULL)
		return;

	for (p = list; *p != NULL; ++p) {
		indent (to, level);
		fprintf (to, "%s = %s\n", name, *p);
	}

	free (list);
}

static void show (struct cmdb *o, FILE *to, int level)
{
	const char **list, **p;

	if ((list = cmdb_list (o, "\a")) != NULL) {
		for (p = list; *p != NULL; ++p)
			show_attr (o, to, level, *p);

		free (list);
	}

	if ((list = cmdb_list (o, "\n")) != NULL) {
		for (p = list; *p != NULL; ++p) {
			indent (to, level);
			fprintf (to, "%s:\n", *p);

			if (cmdb_push (o, *p)) {
				show (o, to, level + 1);
				cmdb_pop (o);
			}
		}

		free (list);
	}
}

int cmdb_save (struct cmdb *o, FILE *to)
{
	show (o, to, 0);
	return !ferror (to);
}
