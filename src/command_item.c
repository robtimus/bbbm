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
#include <gtk/gtk.h>
#include "config.h"
#include "command_item.h"
#include "util.h"
#include "compat.h"

static void bbbm_command_item_class_init(BBBMCommandItemClass *klass);
static void bbbm_command_item_init(BBBMCommandItem *item);
static void bbbm_command_item_destroy(GtkObject *object);
static BBBMCommandItem *bbbm_command_item_new_no_command(const gchar *label);

static GtkEventBoxClass *bbbm_command_item_parent_class = NULL;

GType bbbm_command_item_get_type() {
    static GType type = 0;
    if (type == 0) {
        static const GTypeInfo info = {
            sizeof(BBBMCommandItemClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) bbbm_command_item_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(BBBMCommandItem),
            0, /* n_preallocs */
            (GInstanceInitFunc) bbbm_command_item_init
        };
        type = g_type_register_static(GTK_TYPE_MENU_ITEM, "BBBMCommandItem", &info, 0);
    }
    return type;
}

static void bbbm_command_item_class_init(BBBMCommandItemClass *klass) {
    GtkObjectClass *object_class;

    object_class = GTK_OBJECT_CLASS(klass);
    bbbm_command_item_parent_class = g_type_class_peek_parent(klass);
    object_class->destroy = bbbm_command_item_destroy;
}

static void bbbm_command_item_init(BBBMCommandItem *item) {
    item->command = NULL;
}

static void bbbm_command_item_destroy(GtkObject *object) {
    BBBMCommandItem *item;

    item = BBBM_COMMAND_ITEM(object);

    g_free(item->command);
    item->command = NULL;

    (* GTK_OBJECT_CLASS(bbbm_command_item_parent_class)->destroy) (object);
}

static BBBMCommandItem *bbbm_command_item_new_no_command(const gchar *label) {
    BBBMCommandItem *item;
#if HAVE_GTK_MENU_ITEM_LABEL == 0
    GtkWidget *label_widget;

    g_debug("gtk_menu_item's label property is not available, adding a label widget instead");
    item = BBBM_COMMAND_ITEM(g_object_new(BBBM_TYPE_COMMAND_ITEM, NULL));
    label_widget = gtk_label_new(label);
    gtk_misc_set_alignment(GTK_MISC(label_widget), 0.0, 0.5);
    gtk_container_add(GTK_CONTAINER(item), label_widget);
    gtk_widget_show(label_widget);
#else

    g_debug("gtk_menu_item's label property is available, using it");
    item = BBBM_COMMAND_ITEM(g_object_new(BBBM_TYPE_COMMAND_ITEM, "label", label, NULL));
#endif
    return item;
}

GtkWidget *bbbm_command_item_new(const gchar *label, const gchar *command) {
    BBBMCommandItem *item;

    g_return_val_if_fail(label != NULL, NULL);
    g_return_val_if_fail(command != NULL, NULL);

    item = bbbm_command_item_new_no_command(label);
    item->command = g_strdup(command);

    return GTK_WIDGET(item);
}

GtkWidget *bbbm_command_item_new_for_file(const gchar *label, const gchar *command, const gchar *filename) {
    BBBMCommandItem *item;

    g_return_val_if_fail(label != NULL, NULL);
    g_return_val_if_fail(command != NULL, NULL);
    g_return_val_if_fail(filename != NULL, NULL);

    item = bbbm_command_item_new_no_command(label);
    item->command = bbbm_util_get_command(command, filename);

    return GTK_WIDGET(item);
}

const gchar *bbbm_command_item_get_command(BBBMCommandItem *item) {
    g_return_val_if_fail(BBBM_IS_COMMAND_ITEM(item), NULL);
    return item->command;
}
