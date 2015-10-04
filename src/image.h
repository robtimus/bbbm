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
#define BBBM_WIDGET(obj)          (((BBBMImage *)(obj))->box)

typedef struct
{
    BBBM *bbbm; // the BBBM parent window
    GtkWidget *box; // the event box that is the actual widget
    gchar *filename; // the filename
    gchar *description; // the description
} BBBMImage;

/* Creates a new image with the given filename and description, and thumb,
   and returns it. */
BBBMImage *bbbm_image_new(BBBM *bbbm, const gchar *filename,
                          const gchar *description, const gchar *thumb);

/* Destroys the specified image. */
void bbbm_image_destroy(BBBMImage *);

#endif /* __BBBM_IMAGE_H_ */
