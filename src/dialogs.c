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

#include <string.h>
#include <gtk/gtk.h>
#include "dialogs.h"
#include "options.h"
#include "bbbm.h"
#include "util.h"

#define PADDING         5

gboolean bbbm_dialogs_question(GtkWindow *parent, const gchar *title,
                               const gchar *message)
{
    gboolean result;
    GtkWidget *dialog = gtk_message_dialog_new(parent, 0, GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_YES_NO, message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    result = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES;
    gtk_widget_destroy(dialog);
    return result;
}

void bbbm_dialogs_error(GtkWindow *parent, const gchar *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(parent, 0, GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK, message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

GList *bbbm_dialogs_get_files(GtkWindow *parent, const gchar *title)
{
    GList *result = NULL;
    GtkWidget *chooser = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(chooser), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(chooser), TRUE);
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(chooser));
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar *dir;
        gchar **files =
                gtk_file_selection_get_selections(GTK_FILE_SELECTION(chooser));
        if (files && files[0] && g_file_test(files[0], G_FILE_TEST_IS_REGULAR))
        {
            guint i;
            for (i = 0; files[i]; ++i)
                if (g_file_test(files[i], G_FILE_TEST_IS_REGULAR))
                    result = g_list_append(result,
                                           bbbm_util_absolute_path(files[i]));
            g_strfreev(files);
            break;
        }
        dir = (g_file_test(files[0], G_FILE_TEST_IS_DIR) ?
               g_strconcat(files[0], "/", NULL) :
               bbbm_util_dirname(files[0]));
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
        g_free(dir);
        g_strfreev(files);
    }
    gtk_widget_destroy(chooser);
    return result;
}

GList *bbbm_dialogs_get_files_dir(GtkWindow *parent, const gchar *title)
{
    GList *result = NULL;
    GtkWidget *chooser = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(chooser), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(chooser), FALSE);
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(chooser));
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar *dir;
        gchar *file = bbbm_util_absolute_path(
                 gtk_file_selection_get_filename(GTK_FILE_SELECTION(chooser)));
        if (g_file_test(file, G_FILE_TEST_IS_DIR))
        {
            result = bbbm_util_listdir(file);
            g_free(file);
            break;
        }
        dir = bbbm_util_dirname(file);
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
        g_free(dir);
        g_free(file);
    }
    gtk_widget_destroy(chooser);
    return result;
}

gchar *bbbm_dialogs_get_file(GtkWindow *parent, const gchar *title)
{
    gchar *result = NULL;
    GtkWidget *chooser = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(chooser), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(chooser), FALSE);
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(chooser));
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar *dir;
        gchar *file = bbbm_util_absolute_path(
                 gtk_file_selection_get_filename(GTK_FILE_SELECTION(chooser)));
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
        {
            result = file;
            break;
        }
        dir = (g_file_test(file, G_FILE_TEST_IS_DIR) ?
               g_strconcat(file, "/", NULL) :  bbbm_util_dirname(file));
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
        g_free(dir);
        g_free(file);
    }
    gtk_widget_destroy(chooser);
    return result;
}

void bbbm_dialogs_save(GtkWindow *parent, const gchar *title,
                       save_function save, gpointer data)
{
    GtkWidget *chooser = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(chooser), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(chooser), FALSE);
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar *dir;
        gchar *file = bbbm_util_absolute_path(
                 gtk_file_selection_get_filename(GTK_FILE_SELECTION(chooser)));
        gchar *message = g_strconcat("File '", file, "' exists. Overwrite?",
                                     NULL);
        if (!g_file_test(file, G_FILE_TEST_EXISTS) ||
            (g_file_test(file, G_FILE_TEST_IS_REGULAR) &&
             bbbm_dialogs_question(parent, "Overwrite?", message)))
        {
            if (save(data, file))
            {
                g_free(file);
                g_free(message);
                break;
            }
            g_free(message);
            message = g_strconcat("Could not save to '", file, "'", NULL);
            bbbm_dialogs_error(parent, message);
        }
        g_free(message);
        dir = (g_file_test(file, G_FILE_TEST_IS_DIR) ?
               g_strconcat(file, "/", NULL) :  bbbm_util_dirname(file));
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
        g_free(dir);
        g_free(file);
    }
    gtk_widget_destroy(chooser);
}

guint bbbm_dialogs_options(GtkWindow *parent, struct options *opts)
{
    guint result = 0;
    GtkWidget *dialog, *frame, *table, *label, *set_entry, *view_entry,
              *hbox, *width_entry, *height_entry, *cols_entry,
              *label_check, *title_check;
    GtkObject *adj;

    dialog = gtk_dialog_new_with_buttons("Options", parent,
                                         GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);
    frame = gtk_frame_new("Commands");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    table = gtk_table_new(2, 2, FALSE);
    label = gtk_label_new("Set command:");
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, PADDING, 0);
    set_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(set_entry), opts->set_cmd);
    gtk_table_attach(GTK_TABLE(table), set_entry, 1, 2, 0, 1,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, 0);
    label = gtk_label_new("View command:");
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, 0, 0, PADDING, 0);
    view_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(view_entry), opts->view_cmd);
    gtk_table_attach(GTK_TABLE(table), view_entry, 1, 2, 1, 2,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show_all(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       FALSE, FALSE, 0);

    frame = gtk_frame_new("Thumbnails");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    table = gtk_table_new(2, 2, FALSE);
    label = gtk_label_new("Thumbnail size:");
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, PADDING, 0);
    hbox = gtk_hbox_new(FALSE, PADDING);
    gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, 0);
    adj = gtk_adjustment_new(opts->thumb_width, 1, MAX_WIDTH, 1, 1, 1);
    width_entry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox), width_entry, TRUE, TRUE, 0);
    label = gtk_label_new("x");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    adj = gtk_adjustment_new(opts->thumb_height, 1, MAX_HEIGHT, 1, 1, 1);
    height_entry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox), height_entry, TRUE, TRUE, 0);
    label = gtk_label_new("Thumbnail columns:");
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, 0, 0, PADDING, 0);
    adj = gtk_adjustment_new(opts->thumb_cols, 1, MAX_COLS, 1, 1, 1);
    cols_entry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_table_attach(GTK_TABLE(table), cols_entry, 1, 2, 1, 2,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show_all(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       FALSE, FALSE, 0);
 
    frame = gtk_frame_new("Menu options");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    table = gtk_table_new(2, 1, FALSE);
    label_check = gtk_check_button_new_with_mnemonic("Use filename as _label");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label_check),
                                 opts->filename_label);
    gtk_table_attach(GTK_TABLE(table), label_check, 0, 1, 0, 1,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, 0);
    title_check = gtk_check_button_new_with_mnemonic("Use filename as _title");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(title_check),
                                 opts->filename_title);
    gtk_table_attach(GTK_TABLE(table), title_check, 0, 1, 1, 2,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, 0);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show_all(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       FALSE, FALSE, 0);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        const gchar *set_cmd = gtk_entry_get_text(GTK_ENTRY(set_entry));
        const gchar *view_cmd = gtk_entry_get_text(GTK_ENTRY(view_entry));
        gint thumb_width =
                gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(width_entry));
        gint thumb_height =
               gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(height_entry));
        gint thumb_cols =
                 gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(cols_entry));
        gboolean filename_label =
                  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(label_check));
        gboolean filename_title =
                  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(title_check));
        if (strcmp(set_cmd, opts->set_cmd))
        {
            g_free(opts->set_cmd);
            opts->set_cmd = g_strdup(set_cmd);
            result |= OPTIONS_SET_CMD_CHANGED;
        }
        if (strcmp(view_cmd, opts->view_cmd))
        {
            g_free(opts->view_cmd);
            opts->view_cmd = g_strdup(view_cmd);
            result |= OPTIONS_VIEW_CMD_CHANGED;
        }
        if (thumb_width != opts->thumb_width ||
            thumb_height != opts->thumb_height)
        {
            opts->thumb_width = thumb_width;
            opts->thumb_height = thumb_height;
            result |= OPTIONS_THUMB_SIZE_CHANGED;
        }
        if (thumb_cols != opts->thumb_cols)
        {
            opts->thumb_cols = thumb_cols;
            result |= OPTIONS_THUMB_COLS_CHANGED;
        }
        if (filename_label != opts->filename_label)
        {
            opts->filename_label = filename_label;
            result |= OPTIONS_FILENAME_LABEL_CHANGED;
        }
        if (filename_title != opts->filename_title)
        {
            opts->filename_title = filename_title;
            result |= OPTIONS_FILENAME_TITLE_CHANGED;
        }
    }
    gtk_widget_destroy(dialog);
    return result;
}

void bbbm_dialogs_about(GtkWindow *parent)
{
    static const gchar *about = "bbbm "VERSION"\nWritten by Rob Spoor\n\n"
                                "CopyRight 2004 Rob Spoor";
    GtkWidget *dialog = gtk_message_dialog_new(parent, 0, GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK, about);
    gtk_window_set_title(GTK_WINDOW(dialog), "About");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

gint bbbm_dialogs_move(GtkWindow *parent, const gchar *title, guint limit)
{
    gint result = -1;
    GtkWidget *dialog, *frame, *table, *label, *entry;
    GtkObject *adj;

    dialog = gtk_dialog_new_with_buttons(title, parent,
                                         GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);
    frame = gtk_frame_new(title);
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    table = gtk_table_new(1, 2, FALSE);
    label = gtk_label_new("Moving step:");
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, PADDING, 0);
    adj = gtk_adjustment_new(1, 1, limit, 1, 1, 1);
    entry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 0, 1,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show_all(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       FALSE, FALSE, 0);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
        result = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(entry));
    gtk_widget_destroy(dialog);
    return result;
}

gchar *bbbm_dialogs_edit(GtkWindow *parent, const gchar *initial)
{
    gchar *result = NULL;
    GtkWidget *dialog, *frame, *table, *entry;

    dialog = gtk_dialog_new_with_buttons("Edit description", parent,
                                         GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);
    frame = gtk_frame_new("Description:");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    table = gtk_table_new(1, 1, FALSE);
    entry = gtk_entry_new();
    if (initial)
        gtk_entry_set_text(GTK_ENTRY(entry), initial);
    gtk_table_attach(GTK_TABLE(table), entry, 0, 1, 1, 2,
                     GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show_all(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       FALSE, FALSE, 0);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
        result = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    gtk_widget_destroy(dialog);
    return result;
}
