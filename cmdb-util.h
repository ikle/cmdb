/*
 * Configuration Management Database Helpers
 *
 * Copyright (c) 2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CMDB_UTIL_H
#define CMDB_UTIL_H  1

#include <stdio.h>

#include "cmdb.h"

int cmdb_save (struct cmdb *o, FILE *to);

#endif  /* CMDB_UTIL_H */
