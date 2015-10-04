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

typedef gboolean (* bbbm_save_function) (gpointer data, const gchar *file);

enum {
    OPTIONS_SET_COMMAND_CHANGED         = 1 << 0,
    OPTIONS_THUMB_SIZE_CHANGED          = 1 << 1,
    OPTIONS_THUMB_COLUMN_COUNT_CHANGED  = 1 << 2,
    OPTIONS_FILENAME_AS_LABEL_CHANGED   = 1 << 3,
    OPTIONS_FILENAME_AS_TITLE_CHANGED   = 1 << 4,
    OPTIONS_COMMANDS_CHANGED            = 1 << 5
};

/* Shows a question dialog with Yes/No options using the format and arguments.
   Returns TRUE if Yes was clicked, or FALSE otherwise */
gboolean bbbm_dialogs_question(GtkWindow *parent, const gchar *title, const gchar *format, ...);

/* Shows an error dialog using the given format and arguments */
void bbbm_dialogs_error(GtkWindow *parent, const gchar *format, ...);

/* Shows a dialog for opening multiple files.
   Returns a list of opened file names, or NULL if the user cancelled.
   The returned list and all elements (strings) must be freed when no longer needed */
GList *bbbm_dialogs_get_files(GtkWindow *parent, const gchar *title);

/* Shows a dialog for opening all files in a directory.
   Returns a list of opened file names, or NULL if the user cancelled.
   The returned list and all elements (strings) must be freed when no longer needed */
GList *bbbm_dialogs_get_files_dir(GtkWindow *parent, const gchar *title);

/* Shows a dialog for opening a single file.
   Returns the name of the opened file, or NULL if the user cancelled.
   The returned string must be freed when no longer needed */
gchar *bbbm_dialogs_get_file(GtkWindow *parent, const gchar *title);

/* Shows a save dialog for saving to a single file.
   Upon saving, the egiven save function is called with the given data */
void bbbm_dialogs_save(GtkWindow *parent, const gchar *title, bbbm_save_function save, gpointer data);

/* Shows a dialog to change the options, returning a value indicating what has changed.
   This is a bitwise mask containing OPTIONS_SET_COMMAND_CHANGED, etc */
guint bbbm_dialogs_options(GtkWindow *parent, BBBMOptions *options);

void bbbm_dialogs_about(GtkWindow *parent);

/* Shows a move dialog.
   Returns the selected value to move, or -1 if the user cancelled */
gint bbbm_dialogs_move(GtkWindow *parent, const gchar *title, guint limit);

/* Shows a dialog to edit a description.
   Returns the entered value, or NULL if the user cancelled.
   The returned string must be freed when no longer needed */
gchar *bbbm_dialogs_edit_description(GtkWindow *parent, const gchar *initial);

#endif /* __BBBM_DIALOGS_H_ */
