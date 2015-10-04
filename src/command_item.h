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

#ifndef __BBBM_COMMAND_ITEM_H_
#define __BBBM_COMMAND_ITEM_H_

#include <gtk/gtk.h>
#include "bbbm.h"

#define BBBM_TYPE_COMMAND_ITEM             (bbbm_command_item_get_type())
#define BBBM_COMMAND_ITEM(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), BBBM_TYPE_COMMAND_ITEM, BBBMCommandItem))
#define BBBM_COMMAND_ITEM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), BBBM_TYPE_COMMAND_ITEM, BBBMCommandItemClass))
#define BBBM_IS_COMMAND_ITEM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), BBBM_TYPE_COMMAND_ITEM))
#define BBBM_IS_COMMAND_ITEM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), BBBM_TYPE_COMMAND_ITEM))
#define BBBM_COMMAND_ITEM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), BBBM_TYPE_COMMAND_ITEM, BBBMCommandItemClass))

typedef struct _BBBMCommandItem      BBBMCommandItem;
typedef struct _BBBMCommandItemClass BBBMCommandItemClass;

struct _BBBMCommandItem {
    GtkMenuItem parent;
    gchar *command;
};

struct _BBBMCommandItemClass {
    GtkMenuItemClass parent_class;
};

GType bbbm_command_item_get_type();

/* Creates a new command item with the given label and command.
   The returned widget must be destroyed with gtk_widget_destroy when no longer needed */
GtkWidget *bbbm_command_item_new(const gchar *label, const gchar *command);

/* Creates a new command item with the given label and command for the given filename.
   The returned widget must be destroyed with gtk_widget_destroy when no longer needed */
GtkWidget *bbbm_command_item_new_for_file(const gchar *label, const gchar *command, const gchar *filename);

const gchar *bbbm_command_item_get_command(BBBMCommandItem *item);

#endif /* __BBBM_COMMAND_ITEM_H_ */
