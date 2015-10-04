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
#define THUMB_SIZE      "128x96"
#define THUMB_COLS      4

struct options
{
    gchar *set_command; // the command used for setting images
    gchar *view_command; // the command used for viewing images
    gchar *thumb_size; // the size of the thumb, in XxY format
    guint thumb_cols; // the number of columns
};

/* Fills opts with the default values for any NULL / 0 value. */
void create_default_options(struct options *opts);

/* Destroys the contents of the given options. */
void destroy_options(struct options *opts);

/* Writes the given options to the given file.
   Returns 0 when successful, or non-0 otherwise. */
gint write_options(struct options *opts, const gchar *filename);

/* Reads options from the given file into the given options, using default
    values for any missing options.
    Returns 0 when successful, or non-0 otherwise. */
gint read_options(struct options *opts, const gchar *filename);

#endif /* __BBBM_OPTIONS_H_ */
