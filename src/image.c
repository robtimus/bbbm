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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "image.h"
#include "bbbm.h"

static void bbbm_image_class_init(BBBMImageClass *klass);
static void bbbm_image_init(BBBMImage *image);
static void bbbm_image_destroy(GtkObject *object);

static GtkEventBoxClass *parent_class = NULL;

GType bbbm_image_get_type()
{
    static GType type = 0;
    if (!type)
    {
        static const GTypeInfo info =
        {
            sizeof(BBBMImageClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)bbbm_image_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(BBBMImage),
            0, /* n_preallocs */
            (GInstanceInitFunc)bbbm_image_init
        };
        type = g_type_register_static(GTK_TYPE_EVENT_BOX, "BBBMImage",
                                      &info, 0);
    }
    return type;
}

static void bbbm_image_class_init(BBBMImageClass *klass)
{
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    parent_class = g_type_class_peek_parent(klass);
    object_class->destroy = bbbm_image_destroy;
}

static void bbbm_image_init(BBBMImage *image)
{
    image->image = gtk_image_new();
    gtk_widget_show(image->image);
    gtk_container_add(GTK_CONTAINER(image), image->image);
    image->bbbm = NULL;
    image->filename = NULL;
    image->description = NULL;
}

static void bbbm_image_destroy(GtkObject *object)
{
    BBBMImage *image = BBBM_IMAGE(object);
    if (image->filename)
    {
        g_free(image->filename);
        image->filename = NULL;
    }
    if (image->description)
    {
        g_free(image->description);
        image->description = NULL;
    }
    (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}

GtkWidget *bbbm_image_new(BBBM *bbbm, const gchar *filename,
                          const gchar *description, guint width, guint height)
{
    GtkWidget *image = GTK_WIDGET(g_object_new(BBBM_TYPE_IMAGE, NULL));
    BBBMImage *img = BBBM_IMAGE(image);
    img->bbbm = bbbm;
    img->filename = g_strdup(filename);
    img->description = g_strdup(description);
    bbbm_image_resize(img, width, height);
    return image;
}

G_CONST_RETURN gchar *bbbm_image_get_filename(BBBMImage *image)
{
    g_return_val_if_fail(BBBM_IS_IMAGE(image), NULL);
    return image->filename;
}

G_CONST_RETURN gchar *bbbm_image_get_description(BBBMImage *image)
{
    g_return_val_if_fail(BBBM_IS_IMAGE(image), NULL);
    return image->description;
}

void bbbm_image_set_description(BBBMImage *image, const gchar *description)
{
    g_return_if_fail(BBBM_IS_IMAGE(image));
    if (image->description)
        g_free(image->description);
    image->description = g_strdup(description);
}

void bbbm_image_set_description_ref(BBBMImage *image, gchar *description)
{
    g_return_if_fail(BBBM_IS_IMAGE(image));
    if (image->description)
        g_free(image->description);
    image->description = description;
}

void bbbm_image_resize(BBBMImage *image, guint width, guint height)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    g_return_if_fail(BBBM_IS_IMAGE(image));

    /* always get from file, because decreasing size loses information */
    pixbuf = gdk_pixbuf_new_from_file(image->filename, &error);
    if (error)
    {
        fprintf(stderr, "bbbm: error resizing image '%s': %s\n",
                image->filename, error->message);
        g_error_free(error);
        return;
    }
    if (width != gdk_pixbuf_get_width(pixbuf) ||
        height != gdk_pixbuf_get_height(pixbuf))
    {
        GdkPixbuf *pb;
        gint w = gdk_pixbuf_get_width(pixbuf);
        gint h = gdk_pixbuf_get_height(pixbuf);
        gfloat wf = (gfloat)width / w;
        gfloat hf = (gfloat)height / h;
        if (wf > hf)
            /* height = hf * h = height */
            width = hf * w;
        else if (wf < hf)
            /* width = wf * w = width */
            height = wf * h;
        /* else do nothing; perfect ratio */
        pb = gdk_pixbuf_scale_simple(pixbuf, width, height,
                                     GDK_INTERP_BILINEAR);
        g_object_unref(pixbuf);
        pixbuf = pb;
    }
    gtk_image_set_from_pixbuf(GTK_IMAGE(image->image), pixbuf);
    g_object_unref(pixbuf);
}

gint bbbm_image_compare_filename(BBBMImage *image1, BBBMImage *image2)
{
    g_return_val_if_fail(BBBM_IS_IMAGE(image1) && BBBM_IS_IMAGE(image2), 0);
    return strcmp(image1->filename, image2->filename);
}

gint bbbm_image_compare_description(BBBMImage *image1, BBBMImage *image2)
{
    g_return_val_if_fail(BBBM_IS_IMAGE(image1) && BBBM_IS_IMAGE(image2), 0);
    return strcmp(image1->description, image2->description);
}
