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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "options.h"
#include "bbbm.h"

void create_default_options(struct options *opts)
{
    if (!opts->set_command)
        opts->set_command = g_strdup(SET_COMMAND);
    if (!opts->view_command)
        opts->view_command = g_strdup(VIEW_COMMAND);
    if (!opts->thumb_size)
        opts->thumb_size = g_strdup(THUMB_SIZE);
    if (!opts->thumb_cols)
        opts->thumb_cols = THUMB_COLS;
}

void destroy_options(struct options *opts)
{
    g_free(opts->set_command);
    opts->set_command = NULL;
    g_free(opts->view_command);
    opts->view_command = NULL;
    g_free(opts->thumb_size);
    opts->thumb_size = NULL;
    opts->thumb_cols = 0;
}

gint write_options(struct options *opts, const gchar *filename)
{
    FILE *f;
    if (!(f = fopen(filename, "w")))
    {
        fprintf(stderr, "bbbm: could not write to %s\n", filename);
        return 1;
    }
    fprintf(f, "# WARNING: editing this file is not recommended.\n");
    fprintf(f, "# This file will be overwritten when bbbm closes.\n\n");
    fprintf(f, "set_command = %s\n", opts->set_command);
    fprintf(f, "view_command = %s\n", opts->view_command);
    fprintf(f, "thumb_size = %s\n", opts->thumb_size);
    fprintf(f, "thumb_cols = %d\n", opts->thumb_cols);
    return fclose(f);
}

gint read_options(struct options *opts, const gchar *filename)
{
    FILE *f;
    gchar *set_command = NULL; // temporary value
    gchar *view_command = NULL; // temporary value
    gchar *thumb_size = NULL; // temporary value
    gdouble thumb_cols = 0; // temporary value
    guint lineno = 0; // the current line number
    gchar line[PATH_MAX]; // the current line

    if (!opts)
    {
        opts = g_malloc(sizeof(struct options));
        memset(opts, 0, sizeof(struct options));
    }

    if (!(f = fopen(filename, "r")))
    {
        fprintf(stderr, "bbbm: could not read '%s'\n", filename);
        return 1;
    }
    while (++lineno && fgets(line, PATH_MAX, f))
    {
        gchar **opt_val;
        if (line[0] == '\n' || line[0] == '#')
            continue;
        opt_val = g_strsplit(line, "=", 2);
        if (!opt_val[0] || !opt_val[1])
        {
            fprintf(stderr, "bbbm: error on line %d of '%s': %s\n",
                    lineno, filename, line);
            fclose(f);
            g_strfreev(opt_val);
            return 1;
        }
        g_strstrip(opt_val[0]);
        g_strstrip(opt_val[1]);
        if (!strcmp(opt_val[0], "set_command"))
        {
            if (!set_command)
                set_command = opt_val[1];
            else
                g_free(opt_val[1]);
            g_free(opt_val[0]);
            g_free(opt_val);
            continue;
        }
        if (!strcmp(opt_val[0], "view_command"))
        {
            if (!view_command)
                view_command = opt_val[1];
            else
                g_free(opt_val[1]);
            g_free(opt_val[0]);
            g_free(opt_val);
            continue;
        }
        if (!strcmp(opt_val[0], "thumb_size"))
        {
            if (!thumb_size)
                thumb_size = opt_val[1];
            else
                g_free(opt_val[1]);
            g_free(opt_val[0]);
            g_free(opt_val);
            continue;
        }
        if (!strcmp(opt_val[0], "thumb_cols"))
        {
            if (!thumb_cols)
            {
                gchar *end = NULL;
                thumb_cols = strtod(opt_val[1], &end);
                if (!end || *end)
                {
                    fprintf(stderr, "bbbm: illegal value of 'thumb_cols`: %s.",
                            opt_val[1]);
                    fprintf(stderr, " Using default value\n");
                    thumb_cols = THUMB_COLS;
                }
                // else either end !- NULL and end != '\0'
            }
            g_strfreev(opt_val);
            continue;
        }
    }
    if (fclose(f))
    {
        fprintf(stderr, "bbbm: could not close '%s'\n", filename);
        return 1;
    }
    if (!set_command)
    {
        fprintf(stderr, "bbbm: option 'set_command' missing.");
        fprintf(stderr, " Using default value\n");
        opts->set_command = g_strdup(SET_COMMAND);
    }
    else
        opts->set_command = set_command;
    if (!view_command)
    {
        fprintf(stderr, "bbbm: option 'view_command' missing.");
        fprintf(stderr, " Using default value\n");
        opts->set_command = g_strdup(VIEW_COMMAND);
    }
    else
        opts->view_command = view_command;
    if (!thumb_size)
    {
        fprintf(stderr, "bbbm: option 'thumb_size' missing.");
        fprintf(stderr, " Using default value\n");
        opts->thumb_size = g_strdup(THUMB_SIZE);
    }
    else
        opts->thumb_size = thumb_size;
    if (!thumb_cols)
    {
        fprintf(stderr, "bbbm: option 'thumb_cols' missing or 0.");
        fprintf(stderr, " Using default value\n");
        opts->thumb_cols = THUMB_COLS;
    }
    else if (thumb_cols < 0)
    {
        fprintf(stderr, "bbbm: option 'thumb_cols' is negative.");
        fprintf(stderr, " Using value 1\n");
        opts->thumb_cols = 1;
    }
    else if (thumb_cols > MAX_COLS)
    {
        fprintf(stderr, "bbbm: option 'thumb_cols' is bigger than %d.",
                MAX_COLS);
        fprintf(stderr, " Using value %d\n", MAX_COLS);
        opts->thumb_cols = MAX_COLS;
    }
    else
        opts->thumb_cols = (guint)thumb_cols;
    return 0;
}
