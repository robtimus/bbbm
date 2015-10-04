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

#ifndef __BBBM_IMAGE_H_
#define __BBBM_IMAGE_H_

#include <gtk/gtk.h>
#include "bbbm.h"

#define BBBM_TYPE_IMAGE             (bbbm_image_get_type())
#define BBBM_IMAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),\
                                     BBBM_TYPE_IMAGE, BBBMImage))
#define BBBM_IMAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),\
                                     BBBM_TYPE_IMAGE, BBBMImageClass))
#define BBBM_IS_IMAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),\
                                     BBBM_TYPE_IMAGE))
#define BBBM_IS_IMAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),\
                                     BBBM_TYPE_IMAGE))
#define BBBM_IMAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),\
                                     BBBM_TYPE_IMAGE, BBBMImageClass))

typedef struct _BBBMImage      BBBMImage;
typedef struct _BBBMImageClass BBBMImageClass;

struct _BBBMImage
{
    GtkEventBox parent;
    BBBM *bbbm;
    GtkWidget *image;
    gchar *filename;
    gchar *description;
};

struct _BBBMImageClass
{
    GtkEventBoxClass parent_class;
};

GType bbbm_image_get_type();

GtkWidget *bbbm_image_new(BBBM *bbbm, const gchar *filename,
                          const gchar *description, guint width, guint height);

G_CONST_RETURN gchar *bbbm_image_get_filename(BBBMImage *image);

G_CONST_RETURN gchar *bbbm_image_get_description(BBBMImage *image);

void bbbm_image_set_description(BBBMImage *image, const gchar *description);

void bbbm_image_set_description_ref(BBBMImage *image, gchar *description);

void bbbm_image_resize(BBBMImage *image, guint width, guint height);

gint bbbm_image_compare_filename(BBBMImage *image1, BBBMImage *image2);

gint bbbm_image_compare_description(BBBMImage *image1, BBBMImage *image2);

#endif /* __BBBM_IMAGE_H_ */
