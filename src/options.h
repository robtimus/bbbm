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

#ifndef __BBBM_OPTIONS_H_
#define __BBBM_OPTIONS_H_

#include <glib.h>

#define BBBM_OPTIONS_MAX_THUMB_WIDTH         (gdk_screen_width())
#define BBBM_OPTIONS_MAX_THUMB_HEIGHT        (gdk_screen_height())
#define BBBM_OPTIONS_MAX_THUMB_COLUMN_COUNT  100

typedef struct _BBBMOptions BBBMOptions;

struct _BBBMOptions {
    gchar *set_command;
    guint thumb_width;
    guint thumb_height;
    guint thumb_column_count;
    gboolean filename_as_label;
    gboolean filename_as_title;
    guint current_command_count;
    guint new_command_count;
    gchar **commands;
    gchar **command_labels;
};

/* Creates a new BBBMOptions object with default settings.
   The returned object must be freed with bbbm_options_destroy when no longer needed */
BBBMOptions *bbbm_options_new();

/* Creates a new BBBMOptions object with the settings read from the given file.
   If an error occurs reading the file NULL is returned instead.
   The returned object (if not NULL) must be freed with bbbm_options_destroy when no longer needed */
BBBMOptions *bbbm_options_read_from_file(const gchar *filename);

/* Writes the settings in the given BBBMOptions object to the given file */
void bbbm_options_write_to_file(BBBMOptions *options, const gchar *filename);

const gchar *bbbm_options_get_set_command(BBBMOptions *options);
gboolean bbbm_options_set_set_command(BBBMOptions *options, const gchar *set_command);

const guint bbbm_options_get_thumb_width(BBBMOptions *options);
const guint bbbm_options_get_thumb_height(BBBMOptions *options);
gboolean bbbm_options_set_thumb_size(BBBMOptions *options, const guint width, const guint height);

const guint bbbm_options_get_thumb_column_count(BBBMOptions *options);
gboolean bbbm_options_set_thumb_column_count(BBBMOptions *options, const guint column_count);

const gboolean bbbm_options_get_filename_as_label(BBBMOptions *options);
gboolean bbbm_options_set_filename_as_label(BBBMOptions *options, const gboolean filename_as_label);

const gboolean bbbm_options_get_filename_as_title(BBBMOptions *options);
gboolean bbbm_options_set_filename_as_title(BBBMOptions *options, const gboolean filename_as_title);

const guint bbbm_options_get_current_command_count(BBBMOptions *options);
const guint bbbm_options_get_new_command_count(BBBMOptions *options);
gboolean bbbm_options_set_command_count(BBBMOptions *options, const guint command_count);

const gchar *bbbm_options_get_command(BBBMOptions *options, guint index);
gboolean bbbm_options_set_command(BBBMOptions *options, guint index, const gchar *command);

const gchar *bbbm_options_get_command_label(BBBMOptions *options, guint index);
gboolean bbbm_options_set_command_label(BBBMOptions *options, guint index, const gchar *label);

void bbbm_options_destroy(BBBMOptions *options);

#endif /* __BBBM_OPTIONS_H_ */
