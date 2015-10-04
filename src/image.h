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

#define BBBM_IMAGE(obj)         ((BBBMImage *)(obj))

typedef struct
{
    BBBM *bbbm;
    GtkWidget *box;
    gchar *filename;
    gchar *description;
} BBBMImage;

BBBMImage *bbbm_image_new(BBBM *bbbm, const gchar *filename,
                          const gchar *description, const gchar *thumb);

void bbbm_image_destroy(BBBMImage *image);

const gchar *bbbm_image_get_filename(BBBMImage *image);

const gchar *bbbm_image_get_description(BBBMImage *image);

void bbbm_image_set_description(BBBMImage *image, const gchar *description);

gint bbbm_image_compare_filename(BBBMImage *image1, BBBMImage *image2);

gint bbbm_image_compare_description(BBBMImage *image1, BBBMImage *image2);

#endif /* __BBBM_IMAGE_H_ */
