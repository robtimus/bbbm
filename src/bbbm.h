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

#ifndef __BBBM_H_
#define __BBBM_H_

#define VERSION         "0.4.1"
#define BBBM_DIR        "/.bbbm"
#define BBBM_CONFIG     "/bbbm.cfg"
#define BBBM_THUMBS     "/thumbs"

#define BBBM_EXTENSIONS {".jpg", ".jpeg", ".gif", ".ppm", ".pgm", NULL}
#define MAX_COLS        10

#include <gtk/gtk.h>
#include "options.h"

extern gchar *EXTENSIONS[];

typedef struct
{
    guint padding; // the padding between the images
    const gchar *config; // the configuration file used
    const gchar *thumbsdir; // the thumbs directory
    gchar *filename; // the current filename, or NULL
    struct options *opts; // the options
    gboolean modified; // whether the current collection was modified
    GList *images; // a list of images
    GtkWidget *window; // the window
    GtkWidget *table; // the table
    GtkWidget *file_statusbar; // the statusbar for the current filename
    GtkWidget *image_statusbar; // the statusbar for image descriptions
} BBBM;

/* Creates a new BBBM application, shows it, and returns it.
   If file is not NULL, it is opened. */
BBBM *bbbm_new(struct options *opts, const gchar *config,
               const gchar *thumbsdir, const gchar *file);

/* Destroys the given BBBM application. */
void bbbm_destroy(BBBM *bbbm);

/* Clears the statusbar of the given BBBM application. */
void bbbm_statusbar_clear(BBBM *bbbm);

/* Reorders all images of the given BBBM application, starting at the given
   index. */
void bbbm_reorder(BBBM *bbbm, guint index);

/* Resizes all thumbs of the given BBBM application. */
void bbbm_resize_thumbs(BBBM *bbbm);

/* Sets the modified status of the given BBBM application. */
void bbbm_set_modified(BBBM *bbbm, gboolean modified);

#endif /* __BBBM_H_ */
