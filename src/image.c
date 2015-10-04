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
#include "image.h"
#include "bbbm.h"

BBBMImage *bbbm_image_new(BBBM *bbbm, const gchar *filename,
                          const gchar *description, const gchar *thumb)
{
    GtkWidget *img;
    BBBMImage *image = g_malloc(sizeof(BBBMImage));
    image->bbbm = bbbm;
    image->filename = g_strdup(filename);
    image->description = g_strdup(description);
    image->box = gtk_event_box_new();
    img = gtk_image_new_from_file(thumb);
    gtk_widget_show(img);
    gtk_container_add(GTK_CONTAINER(image->box), img);
    return image;
}

void bbbm_image_destroy(BBBMImage *image)
{
    g_free(image->filename);
    g_free(image->description);
    gtk_widget_destroy(image->box);
    g_free(image);
}

const gchar *bbbm_image_get_filename(BBBMImage *image)
{
    return image->filename;
}

const gchar *bbbm_image_get_description(BBBMImage *image)
{
    return image->description;
}

void bbbm_image_set_description(BBBMImage *image, const gchar *description)
{
    if (!description)
        return;
    g_free(image->description);
    image->description = g_strdup(description);
}

gint bbbm_image_compare_filename(BBBMImage *image1, BBBMImage *image2)
{
    return strcmp(image1->filename, image2->filename);
}

gint bbbm_image_compare_description(BBBMImage *image1, BBBMImage *image2)
{
    return strcmp(image1->description, image2->description);
}
