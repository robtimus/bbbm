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

#include <gtk/gtk.h>
#include "image.h"

BBBMImage *bbbm_image_new(BBBM* bbbm, const gchar *filename,
                          const gchar *description, const gchar *thumb)
{
    BBBMImage *bbbm_image = g_malloc(sizeof(BBBMImage));
    GtkWidget *image = gtk_image_new_from_file(thumb); // the actual image
    gtk_widget_show(image);
    bbbm_image->bbbm = bbbm;
    bbbm_image->box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(BBBM_WIDGET(bbbm_image)), image);
    bbbm_image->filename = g_strdup(filename);
    bbbm_image->description = g_strdup(description);
    return bbbm_image;
}

void bbbm_image_destroy(BBBMImage *image)
{
    g_free(image->filename);
    g_free(image->description);
    gtk_widget_destroy(image->box);
    g_free(image);
}
