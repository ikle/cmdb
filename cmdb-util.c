/*
 * Configuration Management Database Helpers
 *
 * Copyright (c) 2019-2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "cmdb-util.h"

static void indent (FILE *to, int level)
{
	for (; level > 0; --level)
		fputc ('\t', to);
}

static void escape (FILE *to, const char *value)
{
	size_t stop = strcspn (value, " \"");

	if (value[stop] == '\0') {
		fprintf (to, "%s", value);
		return;
	}

	fputc ('"', to);

	for (; *value != '\0'; ++value)
		switch (*value) {
		case '"': case '\\':
			fputc ('\\', to);
			/* pass through */
		default:
			fputc (*value, to);
		}

	fputc ('"', to);
}

static void show_attr (struct cmdb *o, FILE *to, int level, const char *name)
{
	const char **list, **p;

	if ((list = cmdb_list (o, name)) == NULL)
		return;

	for (p = list; *p != NULL; ++p) {
		indent (to, level);
		fprintf (to, "%s = ", name);
		escape (to, *p);
		fputc ('\n', to);
	}

	free (list);
}

static void escape_node (FILE *to, const char *name)
{
	size_t stop = strcspn (name, " ");

	fprintf (to, "%.*s", (int) stop, name);

	if (name[stop] == '\0')
		return;

	fputc (' ', to);
	escape (to, name + stop + 1);
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
			escape_node (to, *p);
			fputs (":\n", to);

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
