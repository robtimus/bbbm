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
const gchar *bbbm_util_extension(const gchar *file);

/* Returns the dirname of the given file, with a trailing /.
   The dirname must be freed when it is no longer needed. */
gchar *bbbm_util_dirname(const gchar *file);

/* Creates dir, and if necessary any parent directory.
   Returns 0 if successful, or non-0 upon error. */
gint bbbm_util_makedirs(const gchar *dir);

/* Returns the absolute, normalized path pointed to by path.
   It must be freed when it is no longer needed. */
gchar *bbbm_util_absolute_path(const gchar *path);

/* Returns a list with the names of all files (not dirs) inside the given dir.
   The names must be freed with g_free when they are no longer needed,
   the list must be freed with g_slist_free when it is no longer needed. */
GList *bbbm_util_listdir(const gchar *dir);

#endif /* __BBBM_UTIL_H_ */
