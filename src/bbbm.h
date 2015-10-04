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

#define VERSION         "0.6.1"
#define BBBM_DIR        "/.bbbm"
#define BBBM_CONFIG     "/bbbm.cfg"
#define BBBM_THUMBS     "/thumbs/"

#include <gtk/gtk.h>
#include "options.h"

typedef struct
{
    struct options *opts;
    const gchar *config;
    gchar *filename;
    gboolean modified;
    GList *images;
    GtkWidget *window;
    GtkWidget *table;
    GtkWidget *file_bar;
    guint file_cid;
    guint file_mod_cid;
    GtkWidget *image_bar;
    guint image_cid;
} BBBM;

BBBM *bbbm_new(struct options *opts, const gchar *config, const gchar *file);

void bbbm_destroy(BBBM *bbbm);

#endif /* __BBBM_H_ */
