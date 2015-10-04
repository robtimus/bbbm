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

#include <string.h>
#include <stdarg.h>
#include <gtk/gtk.h>
#include "config.h"
#include "dialogs.h"
#include "options.h"
#include "bbbm.h"
#include "util.h"

#define PADDING                5
#define OPTION_LABEL_ALIGN_X   1
#define OPTION_LABEL_ALIGN_Y   0.5

static gboolean bbbm_dialogs_confirm_overwrite(GtkWindow *parent, const gchar *file);

gboolean bbbm_dialogs_question(GtkWindow *parent, const gchar *title, const gchar *format, ...) {
    gboolean result;
    va_list args;
    gchar *message;
    GtkWidget *dialog;

    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);

    dialog = gtk_message_dialog_new(parent,
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    result = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES;
    gtk_widget_destroy(dialog);
    g_free(message);
    return result;
}

void bbbm_dialogs_error(GtkWindow *parent, const gchar *format, ...) {
    va_list args;
    gchar *message;
    GtkWidget *dialog;

    va_start(args, format);
    message = g_strdup_vprintf(format, args);
    va_end(args);

    dialog = gtk_message_dialog_new(parent,
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(message);
}

GList *bbbm_dialogs_get_files(GtkWindow *parent, const gchar *title) {
    GList *result = NULL;
    GtkWidget *file_selection;

    file_selection = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(file_selection), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(file_selection), TRUE);
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(file_selection));

    while (gtk_dialog_run(GTK_DIALOG(file_selection)) == GTK_RESPONSE_OK) {
        gchar *dir;
        gchar **files;

        files = gtk_file_selection_get_selections(GTK_FILE_SELECTION(file_selection));
        if (files != NULL && files[0] != NULL && g_file_test(files[0], G_FILE_TEST_IS_REGULAR)) {
            int i;
            for (i = 0; files[i] != NULL; ++i) {
                if (g_file_test(files[i], G_FILE_TEST_IS_REGULAR)) {
                    result = g_list_append(result, bbbm_util_absolute_path(files[i]));
                }
            }
            g_strfreev(files);
            break;
        }
        dir = g_file_test(files[0], G_FILE_TEST_IS_DIR)
                ? g_strconcat(files[0], "/", NULL)
                : bbbm_util_dirname(files[0]);
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection), dir);
        g_free(dir);
        g_strfreev(files);
    }
    gtk_widget_destroy(file_selection);
    return result;
}

GList *bbbm_dialogs_get_files_dir(GtkWindow *parent, const gchar *title) {
    GList *result = NULL;
    GtkWidget *file_selection;

    file_selection = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(file_selection), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(file_selection), FALSE);
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(file_selection));

    while (gtk_dialog_run(GTK_DIALOG(file_selection)) == GTK_RESPONSE_OK) {
        gchar *dir;
        gchar *file;

        file = bbbm_util_absolute_path(gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection)));
        if (g_file_test(file, G_FILE_TEST_IS_DIR)) {
            result = bbbm_util_listdir(file);
            g_free(file);
            break;
        }
        dir = bbbm_util_dirname(file);
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection), dir);
        g_free(dir);
        g_free(file);
    }
    gtk_widget_destroy(file_selection);
    return result;
}

gchar *bbbm_dialogs_get_file(GtkWindow *parent, const gchar *title) {
    gchar *result = NULL;
    GtkWidget *file_selection;

    file_selection = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(file_selection), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(file_selection), FALSE);
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(file_selection));

    while (gtk_dialog_run(GTK_DIALOG(file_selection)) == GTK_RESPONSE_OK) {
        gchar *dir;
        gchar *file;

        file = bbbm_util_absolute_path(gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection)));
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR)) {
            result = file;
            break;
        }
        dir = g_file_test(file, G_FILE_TEST_IS_DIR)
                ? g_strconcat(file, "/", NULL)
                : bbbm_util_dirname(file);
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection), dir);
        g_free(dir);
        g_free(file);
    }
    gtk_widget_destroy(file_selection);
    return result;
}

void bbbm_dialogs_save(GtkWindow *parent, const gchar *title, bbbm_save_function save, gpointer data) {
    GtkWidget *file_selection;

    file_selection = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(file_selection), parent);
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(file_selection), FALSE);

    while (gtk_dialog_run(GTK_DIALOG(file_selection)) == GTK_RESPONSE_OK) {
        gchar *dir;
        gchar *file;

        file = bbbm_util_absolute_path(gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection)));
        if (!g_file_test(file, G_FILE_TEST_EXISTS)
                || (g_file_test(file, G_FILE_TEST_IS_REGULAR) && bbbm_dialogs_confirm_overwrite(parent, file))) {

            if (save(data, file)) {
                g_free(file);
                break;
            }
            bbbm_dialogs_error(parent, "Could not save to '%s'", file);
        }
        dir = g_file_test(file, G_FILE_TEST_IS_DIR)
                ? g_strconcat(file, "/", NULL)
                : bbbm_util_dirname(file);
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection), dir);
        g_free(dir);
        g_free(file);
    }
    gtk_widget_destroy(file_selection);
}

guint bbbm_dialogs_options(GtkWindow *parent, BBBMOptions *options) {
    guint result = 0;
    GtkWidget *dialog, *notebook, *vbox, *hbox, *frame, *table, *label, *scrolled_window;
    GtkWidget *set_command_entry,
              *thumb_width_entry, *thumb_height_entry, *thumb_column_count_entry,
              *filename_as_label_check_button, *filename_as_title_check_button;
    GtkSizeGroup *group;
    GtkWidget **commands, **command_labels;
    guint command_count;
    guint i;

    dialog = gtk_dialog_new_with_buttons("Options", parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);

    group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    notebook = gtk_notebook_new();

    /* General tab */
    label = gtk_label_new("General");
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);

    /* General tab, Background frame */
    frame = gtk_frame_new("Backgrounds");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(2, 1, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* General tab, Background frame, Set command */
    label = gtk_label_new("Set command:");
    gtk_misc_set_alignment(GTK_MISC(label), OPTION_LABEL_ALIGN_X, OPTION_LABEL_ALIGN_Y);
    gtk_size_group_add_widget(group, label);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, PADDING, 0);

    set_command_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(set_command_entry), bbbm_options_get_set_command(options));
    gtk_table_attach(GTK_TABLE(table), set_command_entry, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, PADDING, 0);

    /* General tab, Thumbnails frame */
    frame = gtk_frame_new("Thumbnails");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(2, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* General tab, Thumbnails frame, Thumbnail size (WxH) */
    label = gtk_label_new("Thumbnail size:");
    gtk_misc_set_alignment(GTK_MISC(label), OPTION_LABEL_ALIGN_X, OPTION_LABEL_ALIGN_Y);
    gtk_size_group_add_widget(group, label);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, PADDING, 0);

    hbox = gtk_hbox_new(FALSE, PADDING);
    gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, PADDING, 0);

    thumb_width_entry = gtk_spin_button_new_with_range(1, BBBM_OPTIONS_MAX_THUMB_WIDTH, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(thumb_width_entry), bbbm_options_get_thumb_width(options));
    gtk_box_pack_start(GTK_BOX(hbox), thumb_width_entry, TRUE, TRUE, 0);

    label = gtk_label_new("x");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    thumb_height_entry = gtk_spin_button_new_with_range(1, BBBM_OPTIONS_MAX_THUMB_HEIGHT, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(thumb_height_entry), bbbm_options_get_thumb_height(options));
    gtk_box_pack_start(GTK_BOX(hbox), thumb_height_entry, TRUE, TRUE, 0);

    /* General tab, Thumbnails frame, Thumbnail columns */
    label = gtk_label_new("Thumbnail columns:");
    gtk_misc_set_alignment(GTK_MISC(label), OPTION_LABEL_ALIGN_X, OPTION_LABEL_ALIGN_Y);
    gtk_size_group_add_widget(group, label);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, 0, 0, PADDING, 0);

    thumb_column_count_entry = gtk_spin_button_new_with_range(1, BBBM_OPTIONS_MAX_THUMB_COLUMN_COUNT, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(thumb_column_count_entry), bbbm_options_get_thumb_column_count(options));
    gtk_table_attach(GTK_TABLE(table), thumb_column_count_entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);

    /* General tab, Menu options frame */
    frame = gtk_frame_new("Menu options");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(2, 1, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* General tab, Menu options frame, Use filename as label */
    filename_as_label_check_button = gtk_check_button_new_with_mnemonic("Use filename as _label");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(filename_as_label_check_button), bbbm_options_get_filename_as_label(options));
    gtk_table_attach(GTK_TABLE(table), filename_as_label_check_button, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, 0, PADDING, 0);

    /* General tab, Menu options frame, Use filename as title */
    filename_as_title_check_button = gtk_check_button_new_with_mnemonic("Use filename as _title");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(filename_as_title_check_button), bbbm_options_get_filename_as_title(options));
    gtk_table_attach(GTK_TABLE(table), filename_as_title_check_button, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, 0, PADDING, 0);

    /* Commands tab */
    command_count = bbbm_options_get_current_command_count(options);
    commands       = g_malloc(command_count * sizeof(GtkWidget *));
    command_labels = g_malloc(command_count * sizeof(GtkWidget *));
    /* clear the commands and command labels */
    memset(commands,       0, command_count * sizeof(GtkWidget *));
    memset(command_labels, 0, command_count * sizeof(GtkWidget *));

    label = gtk_label_new("Commands");
    table = gtk_table_new(command_count + 1, 2, FALSE);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), table);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, label);

    label = gtk_label_new("Label");
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, 0, PADDING, 0);
    label = gtk_label_new("Command");
    gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, PADDING, 0);

    for (i = 0; i < command_count; i++) {
        const gchar *command       = bbbm_options_get_command(options, i);
        const gchar *command_label = bbbm_options_get_command_label(options, i);

        command_labels[i] = gtk_entry_new();
        if (command_label) {
            gtk_entry_set_text(GTK_ENTRY(command_labels[i]), command_label);
        }
        gtk_table_attach(GTK_TABLE(table), command_labels[i], 0, 1, i + 1, i + 2, GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);
        commands[i] = gtk_entry_new();
        if (command) {
            gtk_entry_set_text(GTK_ENTRY(commands[i]), command);
        }
        gtk_table_attach(GTK_TABLE(table), commands[i], 1, 2, i + 1, i + 2, GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);
    }

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, FALSE, FALSE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *set_command;
        gint thumb_width;
        gint thumb_height;
        gint thumb_column_count;
        gboolean filename_as_label;
        gboolean filename_as_title;

        set_command        = gtk_entry_get_text(GTK_ENTRY(set_command_entry));
        thumb_width        = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(thumb_width_entry));
        thumb_height       = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(thumb_height_entry));
        thumb_column_count = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(thumb_column_count_entry));
        filename_as_label  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filename_as_label_check_button));
        filename_as_title  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filename_as_title_check_button));

        if (bbbm_options_set_set_command(options, set_command)) {
            result |= OPTIONS_SET_COMMAND_CHANGED;
        }
        if (bbbm_options_set_thumb_size(options, thumb_width, thumb_height)) {
            result |= OPTIONS_THUMB_SIZE_CHANGED;
        }
        if (bbbm_options_set_thumb_column_count(options, thumb_column_count)) {
            result |= OPTIONS_THUMB_COLUMN_COUNT_CHANGED;
        }
        if (bbbm_options_set_filename_as_label(options, filename_as_label)) {
            result |= OPTIONS_FILENAME_AS_LABEL_CHANGED;
        }
        if (bbbm_options_set_filename_as_title(options, filename_as_title)) {
            result |= OPTIONS_FILENAME_AS_TITLE_CHANGED;
        }
        for (i = 0; i < command_count; i++) {
            const gchar *command;
            const gchar *command_label;

            command       = gtk_entry_get_text(GTK_ENTRY(commands[i]));
            command_label = gtk_entry_get_text(GTK_ENTRY(command_labels[i]));
            if (bbbm_options_set_command(options, i, command)) {
                result |= OPTIONS_COMMANDS_CHANGED;
            }
            if (bbbm_options_set_command_label(options, i, command_label)) {
                result |= OPTIONS_COMMANDS_CHANGED;
            }
        }
    }
    gtk_widget_destroy(dialog);
    g_free(commands);
    g_free(command_labels);

    return result;
}

void bbbm_dialogs_about(GtkWindow *parent) {
    static const gchar *about = PACKAGE_STRING"\nWritten by Rob Spoor\n\n"BBBM_COPYRIGHT;

    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(parent, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", about);
    gtk_window_set_title(GTK_WINDOW(dialog), "About");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

gint bbbm_dialogs_move(GtkWindow *parent, const gchar *title, guint limit) {
    gint result = -1;
    GtkWidget *dialog, *frame, *table, *label, *entry;

    dialog = gtk_dialog_new_with_buttons(title, parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);

    label = gtk_label_new("Moving step:");

    entry = gtk_spin_button_new_with_range(1, limit, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry), 1);

    table = gtk_table_new(1, 2, FALSE);
    gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, PADDING, 0);
    gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);

    frame = gtk_frame_new(title);
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    gtk_container_add(GTK_CONTAINER(frame), table);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, FALSE, FALSE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        result = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(entry));
    }
    gtk_widget_destroy(dialog);
    return result;
}

gchar *bbbm_dialogs_edit_description(GtkWindow *parent, const gchar *initial) {
    gchar *result = NULL;
    GtkWidget *dialog, *frame, *table, *entry;

    dialog = gtk_dialog_new_with_buttons("Edit description", parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);

    entry = gtk_entry_new();
    if (initial != NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), initial);
    }

    table = gtk_table_new(1, 1, FALSE);
    gtk_table_attach(GTK_TABLE(table), entry, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, 0, PADDING, PADDING);

    frame = gtk_frame_new("Description:");
    gtk_container_set_border_width(GTK_CONTAINER(frame), PADDING);
    gtk_container_add(GTK_CONTAINER(frame), table);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, FALSE, FALSE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        result = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    }
    gtk_widget_destroy(dialog);
    return result;
}

static gboolean bbbm_dialogs_confirm_overwrite(GtkWindow *parent, const gchar *file) {
    return bbbm_dialogs_question(parent, "Overwrite?", "File '%s' exists. Overwrite?", file);
}
