/*
 * bbbm - A background manager for Blackbox
 * Copyright (C) 2004-2007 Rob Spoor
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

gboolean bbbm_util_is_image(const gchar *filename);

gchar *bbbm_util_get_command(const gchar *command, const gchar *filename);

void bbbm_util_execute(const gchar *command, const gchar *filename);

void bbbm_util_execute_cmd(const gchar *command);

gchar *bbbm_util_dirname(const gchar *filename);

gchar *bbbm_util_absolute_path(const gchar *path);

GList *bbbm_util_listdir(const gchar *dir);

#endif /* __BBBM_UTIL_H_ */
