/*
 * bbbm - A background manager for Blackbox
 * Copyright (C) 2004-2015 Rob Spoor
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

#include <stdarg.h>
#include <glib.h>

/* if not defined (because glib is too old), define g_info here */
#ifndef g_info
#define g_info(...)  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

/* Returns whether or not the given string is NULL or empty */
gboolean bbbm_str_empty(const gchar *str);

/* Returns whether or not the given strings are equal (NULL safe) */
gboolean bbbm_str_equals(const gchar *str1, const gchar *str2);

/* Returns whether or not the given string is a valid image filename */
gboolean bbbm_util_is_image(const gchar *filename);

/* Returns a fully expanded command based on the given command and filename
   The returned string must be freed when no longer needed */
gchar *bbbm_util_get_command(const gchar *command, const gchar *filename);

/* Executes the given command for the given filename */
void bbbm_util_execute(const gchar *command, const gchar *filename);

/* Executes the given command */
void bbbm_util_execute_cmd(const gchar *command);

/* Returns the dirname for the given filename.
   The returned string must be freed when no longer needed */
gchar *bbbm_util_dirname(const gchar *filename);

/* Returns an absolute path based on the given path.
   The returned string must be freed when no longer needed */
gchar *bbbm_util_absolute_path(const gchar *path);

/* Lists all regular files in the given directory.
   The returned list and all of its elements (strings) must be freed when no longer needed */
GList *bbbm_util_listdir(const gchar *dir);

#endif /* __BBBM_UTIL_H_ */
