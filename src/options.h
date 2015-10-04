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

#ifndef __BBBM_OPTIONS_H_
#define __BBBM_OPTIONS_H_

#include <glib.h>

#define SET_COMMAND     "bsetbg"
#define VIEW_COMMAND    "gqview"
#define THUMB_WIDTH     128
#define THUMB_HEIGHT    96
#define THUMB_COLS      4

#define MAX_WIDTH       (gdk_screen_width())
#define MAX_HEIGHT      (gdk_screen_height())
#define MAX_COLS        10

struct options
{
    gchar *set_cmd;
    gchar *view_cmd;
    gint thumb_width;
    gint thumb_height;
    gint thumb_cols;
    gboolean filename_label;
    gboolean filename_title;
};

struct options *bbbm_options_new();

void bbbm_options_destroy(struct options *opts);

gint bbbm_options_write(struct options *opts, const gchar *filename);

struct options *bbbm_options_read(const gchar *filename);

#endif /* __BBBM_OPTIONS_H_ */
