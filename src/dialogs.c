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
#include "options.h"
#include "image.h"
#include "bbbm.h"
#include "dialogs.h"

#define ABOUT           "bbbm "VERSION"\nWritten by Rob Spoor\n\n"\
                        "CopyRight 2004 Rob Spoor"

void bbbm_options_dialog(BBBM *bbbm)
{
    GtkWidget *dialog, *frame, *table, *label,
              *set_entry, *view_entry, *size_entry, *cols_entry;
    GtkObject *adj;

    bbbm_statusbar_clear(bbbm);
    dialog = gtk_dialog_new_with_buttons("Options", GTK_WINDOW(bbbm->window),
                                         GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);
    frame = gtk_frame_new("Commands");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
    table = gtk_table_new(2, 2, TRUE);
    label = gtk_label_new("Set command:");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    set_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(set_entry), bbbm->opts->set_command);
    gtk_widget_show(set_entry);
    gtk_table_attach_defaults(GTK_TABLE(table), set_entry, 1, 2, 0, 1);
    label = gtk_label_new("View command:");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
    view_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(view_entry), bbbm->opts->view_command);
    gtk_widget_show(view_entry);
    gtk_table_attach_defaults(GTK_TABLE(table), view_entry, 1, 2, 1, 2);
    gtk_widget_show(table);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       TRUE, TRUE, 0);

    frame = gtk_frame_new("Thumbnails");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
    table = gtk_table_new(2, 2, TRUE);
    label = gtk_label_new("Thumbnail size:");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    size_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(size_entry), bbbm->opts->thumb_size);
    gtk_widget_show(size_entry);
    gtk_table_attach_defaults(GTK_TABLE(table), size_entry, 1, 2, 0, 1);
    label = gtk_label_new("Thumbnail columns:");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
    adj = gtk_adjustment_new(bbbm->opts->thumb_cols, 1, MAX_COLS, 1, 1, 1);
    cols_entry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_widget_show(cols_entry);
    gtk_table_attach_defaults(GTK_TABLE(table), cols_entry, 1, 2, 1, 2);
    gtk_widget_show(table);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       TRUE, TRUE, 0);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        const gchar *set_command;
        const gchar *view_command;
        const gchar *thumb_size;
        gint thumb_cols;
        if (strcmp((set_command = gtk_entry_get_text(GTK_ENTRY(set_entry))),
                   bbbm->opts->set_command))
        {
            g_free(bbbm->opts->set_command);
            bbbm->opts->set_command = g_strdup(set_command);
        }
        if (strcmp((view_command = gtk_entry_get_text(GTK_ENTRY(view_entry))),
                   bbbm->opts->view_command))
        {
            g_free(bbbm->opts->view_command);
            bbbm->opts->view_command = g_strdup(view_command);
        }
        if (strcmp((thumb_size = gtk_entry_get_text(GTK_ENTRY(size_entry))),
                   bbbm->opts->thumb_size))
        {
            g_free(bbbm->opts->thumb_size);
            bbbm->opts->thumb_size = g_strdup(thumb_size);
            bbbm_resize_thumbs(bbbm);
        }
        if ((thumb_cols = gtk_spin_button_get_value_as_int(
             GTK_SPIN_BUTTON(cols_entry))) != bbbm->opts->thumb_cols)
        {
            bbbm->opts->thumb_cols = thumb_cols;
            bbbm_reorder(bbbm, 0);
        }
        write_options(bbbm->opts, bbbm->config);
    }
    gtk_widget_destroy(dialog);
}

void bbbm_about_dialog(BBBM *bbbm)
{
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(bbbm->window),
                                               0, GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK, ABOUT);
    gtk_window_set_title(GTK_WINDOW(dialog), "About");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void bbbm_move_dialog(BBBMImage *image, gint index, gboolean forward)
{
    GtkWidget *dialog, *frame, *table, *label, *entry;
    GtkObject *adj;
    guint limit;
    const gchar *title;

    title = (forward ? "Move forward" : "Move back");
    dialog = gtk_dialog_new_with_buttons(title,
                                         GTK_WINDOW(image->bbbm->window),
                                         GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);
    frame = gtk_frame_new(title);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
    table = gtk_table_new(1, 2, TRUE);
    label = gtk_label_new("Moving step:");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    limit = (forward ? g_list_length(image->bbbm->images) - index - 1 : index);
    adj = gtk_adjustment_new(1, 1, limit, 1, 1, 1);
    entry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_widget_show(entry);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 0, 1);
    gtk_widget_show(table);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       TRUE, TRUE, 0);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        gint pos = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(entry)) *
                   (forward ? 1 : -1) + index;
        image->bbbm->images = g_list_remove(image->bbbm->images, image);
        image->bbbm->images = g_list_insert(image->bbbm->images, image, pos);
        bbbm_reorder(image->bbbm, (forward ? index : pos));
    }
    gtk_widget_destroy(dialog);
}

void bbbm_edit_description_dialog(BBBMImage *image)
{
    GtkWidget *dialog, *frame, *table, *label, *entry;

    bbbm_statusbar_clear(image->bbbm);
    dialog = gtk_dialog_new_with_buttons("Edit description",
                                         GTK_WINDOW(image->bbbm->window),
                                         GTK_DIALOG_NO_SEPARATOR,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         NULL);
    frame = gtk_frame_new("Description");
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
    table = gtk_table_new(2, 1, TRUE);
    label = gtk_label_new("Description:");
    gtk_widget_show(label);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), image->description);
    gtk_widget_show(entry);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 0, 1, 1, 2);
    gtk_widget_show(table);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,
                       TRUE, TRUE, 0);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        const gchar *desc;
        if (strcmp((desc = gtk_entry_get_text(GTK_ENTRY(entry))),
                   image->description))
        {
            g_free(image->description);
            image->description = g_strdup(desc);
            bbbm_set_modified(image->bbbm, TRUE);
        }
    }
    gtk_widget_destroy(dialog);
}
