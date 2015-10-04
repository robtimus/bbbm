/*
 * bbbm - A background manager for Blackbox
 * Copyright (C) 2004 Rob Spoor
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __BBBM_UTIL_H_
#define __BBBM_UTIL_H_

#include <glib.h>

/* Returns a pointer to the extension of the given filename, or NULL if there
   is no extension. */
const gchar *extension(const gchar *file);

/* Puts the dirname of the given filename into dir.
   dir must have enough storage to store the dirname. */
void dirname(const gchar *, gchar *);

/* Creates dir, and if necessary any parent directory.
   Returns 0 if successful, or non-0 upon error. */
gint makedirs(const gchar *dir);

/* Returns a list with the names of all files (not dirs) inside the given dir.
   The names must be freed with g_free when they are no longer necessary. */
GSList *listdir(const gchar *dir);

#endif /* __BBBM_UTIL_H_ */
