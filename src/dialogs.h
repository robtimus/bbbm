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

#ifndef __BBBM_DIALOGS_H_
#define __BBBM_DIALOGS_H_

#include <gtk/gtk.h>
#include "options.h"

typedef gboolean (* save_function) (gpointer data, const gchar *file);

enum
{
    OPTIONS_SET_CMD_CHANGED = 1,
    OPTIONS_VIEW_CMD_CHANGED = 2,
    OPTIONS_THUMB_SIZE_CHANGED = 4,
    OPTIONS_THUMB_COLS_CHANGED = 8,
    OPTIONS_FILENAME_LABEL_CHANGED = 16,
    OPTIONS_FILENAME_TITLE_CHANGED = 32,
    OPTIONS_COMMAND_CHANGED = 64
};

gboolean bbbm_dialogs_question(GtkWindow *parent, const gchar *title,
                               const gchar *message);

void bbbm_dialogs_error(GtkWindow *parent, const gchar *message);

GList *bbbm_dialogs_get_files(GtkWindow *parent, const gchar *title);

GList *bbbm_dialogs_get_files_dir(GtkWindow *parent, const gchar *title);

gchar *bbbm_dialogs_get_file(GtkWindow *parent, const gchar *title);

void bbbm_dialogs_save(GtkWindow *parent, const gchar *title,
                       save_function save, gpointer data);

guint bbbm_dialogs_options(GtkWindow *parent, struct options *opts);

void bbbm_dialogs_about(GtkWindow *parent);

gint bbbm_dialogs_move(GtkWindow *parent, const gchar *title, guint limit);

gchar *bbbm_dialogs_edit(GtkWindow *parent, const gchar *initial);

#endif /* __BBBM_DIALOGS_H_ */
