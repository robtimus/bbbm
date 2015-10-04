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
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "options.h"
#include "util.h"

struct options *bbbm_options_new()
{
    struct options *opts = g_malloc(sizeof(struct options));
    opts->set_cmd = g_strdup(SET_COMMAND);
    opts->view_cmd = g_strdup(VIEW_COMMAND);
    opts->thumb_width = THUMB_WIDTH;
    opts->thumb_height = THUMB_HEIGHT;
    opts->thumb_cols = THUMB_COLS;
    return opts;
}

void bbbm_options_destroy(struct options *opts)
{
    g_free(opts->set_cmd);
    g_free(opts->view_cmd);
    g_free(opts);
}

gint bbbm_options_write(struct options *opts, const gchar *filename)
{
    guint i;
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "bbbm: could not write to '%s': %s\n",
                filename, g_strerror(errno));
        return 1;
    }
    fprintf(file, "# WARNING: editing this file is not recommended.\n");
    fprintf(file, "# This file will be overwritten when bbbm closes.\n\n");
    fprintf(file, "set_command = %s\n", opts->set_cmd);
    fprintf(file, "view_command = %s\n", opts->view_cmd);
    fprintf(file, "thumb_size = %dx%d\n", opts->thumb_width,
                                          opts->thumb_height);
    fprintf(file, "thumb_cols = %d\n", opts->thumb_cols);
    fprintf(file, "filename_label = %s\n",
            (opts->filename_label ? "TRUE" : "FALSE"));
    fprintf(file, "filename_title = %s\n",
            (opts->filename_title ? "TRUE" : "FALSE"));
    for (i = 0; i < MAX_COMMANDS; i++)
    {
        if (opts->commands[i])
            fprintf(file, "command%d = %s\n", i, opts->commands[i]);
        if (opts->cmd_labels[i])
            fprintf(file, "cmd_label%d = %s\n", i, opts->cmd_labels[i]);
    }
    return fclose(file);
}

struct options *bbbm_options_read(const gchar *filename)
{
    guint i;
    struct options *opts = NULL;
    gchar *thumb_size = NULL;
    gchar *thumb_cols = NULL;
    gchar *filename_label = NULL;
    gchar *filename_title = NULL;
    guint lineno = 0;
    gchar line[PATH_MAX];
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "bbbm: could not read '%s': %s\n",
                filename, g_strerror(errno));
        return NULL;
    }
    opts = g_malloc(sizeof(struct options));
    memset(opts, 0, sizeof(struct options));
    while (++lineno && fgets(line, PATH_MAX, file))
    {
        gchar **optval;
        if (line[0] == '\n' || line[0] == '#')
            continue;
        optval = g_strsplit(line, "=", 2);
        if (!optval[0] || !optval[1])
        {
            fprintf(stderr, "bbbm: error on line %d of '%s': %s\n",
                    lineno, filename, line);
            fclose(file);
            g_strfreev(optval);
            bbbm_options_destroy(opts);
            return NULL;
        }
        g_strstrip(optval[0]);
        g_strstrip(optval[1]);
        if (!strcmp(optval[0], "set_command"))
        {
            if (!opts->set_cmd)
                opts->set_cmd = optval[1];
            else
            {
                fprintf(stderr, "bbbm: duplicate value of option '%s' on line"
                        " %d of '%s': %s. Using first value %s\n",
                        optval[0], lineno, filename, optval[1], opts->set_cmd);
                g_free(optval[1]);
            }
            g_free(optval[0]);
            g_free(optval);
            continue;
        }
        if (!strcmp(optval[0], "view_command"))
        {
            if (!opts->view_cmd)
                opts->view_cmd = optval[1];
            else
            {
                fprintf(stderr, "bbbm: duplicate value of option '%s' on line"
                        " %d of '%s': %s. Using first value %s\n",
                        optval[0], lineno, filename, optval[1],
                        opts->view_cmd);
                g_free(optval[1]);
            }
            g_free(optval[0]);
            g_free(optval);
            continue;
        }
        if (!strcmp(optval[0], "thumb_size"))
        {
            if (!thumb_size)
            {
                gchar *mid, *end;
                thumb_size = optval[1];
                opts->thumb_width = strtod(thumb_size, &mid);
                if (*mid != 'x' || mid == thumb_size)
                {
                    fprintf(stderr, "bbbm: illegal value of option '%s' on "
                            "line %d: %s\n", optval[0], lineno, optval[1]);
                    fclose(file);
                    g_strfreev(optval);
                    g_free(thumb_cols);
                    g_free(filename_label);
                    g_free(filename_title);
                    bbbm_options_destroy(opts);
                    return NULL;
                }
                /* skip past the x */
                ++mid;
                opts->thumb_height = strtod(mid, &end);
                if (*end || end == mid)
                {
                    fprintf(stderr, "bbbm: illegal value of option '%s' on "
                            "line %d: %s\n", optval[0], lineno, optval[1]);
                    fclose(file);
                    g_strfreev(optval);
                    g_free(thumb_cols);
                    g_free(filename_label);
                    g_free(filename_title);
                    bbbm_options_destroy(opts);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "bbbm: duplicate value of option '%s' on line"
                        " %d of '%s': %s. Using first value %s\n",
                        optval[0], lineno, filename, optval[1], thumb_size);
                g_free(optval[1]);
            }
            g_free(optval[0]);
            g_free(optval);
            continue;
        }
        if (!strcmp(optval[0], "thumb_cols"))
        {
            if (!thumb_cols)
            {
                gchar *end;
                thumb_cols = optval[1];
                opts->thumb_cols = strtod(thumb_cols, &end);
                if (*end || end == thumb_cols)
                {
                    fprintf(stderr, "bbbm: illegal value of option '%s' on "
                            "line %d: %s\n", optval[0], lineno, optval[1]);
                    fclose(file);
                    g_strfreev(optval);
                    g_free(thumb_size);
                    g_free(filename_label);
                    g_free(filename_title);
                    bbbm_options_destroy(opts);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "bbbm: duplicate value of option '%s' on line"
                        " %d of '%s': %s. Using first value %s\n",
                        optval[0], lineno, filename, optval[1], thumb_cols);
                g_free(optval[1]);
            }
            g_free(optval[0]);
            g_free(optval);
            continue;
        }
        if (!strcmp(optval[0], "filename_label"))
        {
            if (!filename_label)
            {
                filename_label = optval[1];
                if (!strcasecmp(optval[1], "true"))
                    opts->filename_label = TRUE;
                else if (!strcasecmp(optval[1], "false"))
                    opts->filename_label = FALSE;
                else
                {
                    fprintf(stderr, "bbbm: illegal value of option '%s' on "
                            "line %d: %s\n", optval[0], lineno, optval[1]);
                    fclose(file);
                    g_strfreev(optval);
                    g_free(thumb_size);
                    g_free(thumb_cols);
                    g_free(filename_title);
                    bbbm_options_destroy(opts);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "bbbm: duplicate value of option '%s' on line"
                        " %d of '%s': %s. Using first value %s\n",
                        optval[0], lineno, filename, optval[1],
                        filename_label);
                g_free(optval[1]);
            }
            g_free(optval[0]);
            g_free(optval);
            continue;
        }
        if (!strcmp(optval[0], "filename_title"))
        {
            if (!filename_title)
            {
                filename_title = optval[1];
                if (!strcasecmp(optval[1], "true"))
                    opts->filename_title = TRUE;
                else if (!strcasecmp(optval[1], "false"))
                    opts->filename_title = FALSE;
                else
                {
                    fprintf(stderr, "bbbm: illegal value of option '%s' on "
                            "line %d: %s\n", optval[0], lineno, optval[1]);
                    fclose(file);
                    g_strfreev(optval);
                    g_free(filename_label);
                    g_free(thumb_size);
                    g_free(thumb_cols);
                    bbbm_options_destroy(opts);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "bbbm: duplicate value of option '%s' on line"
                        " %d of '%s': %s. Using first value %s\n",
                        optval[0], lineno, filename, optval[1],
                        filename_label);
                g_free(optval[1]);
            }
            g_free(optval[0]);
            g_free(optval);
            continue;
        }
        for (i = 0; i < MAX_COMMANDS; i++)
        {
            gchar label[20];
            snprintf(label, 19, "command%d", i);
            if (!strcmp(optval[0], label))
            {
                if (opts->commands[i])
                {
                    // ignore the new one
                    g_strfreev(optval);
                }
                else
                {
                    opts->commands[i] = optval[1];
                    g_free(optval[0]);
                    g_free(optval);
                }
                // I so hate goto, but C does not allow to continue the outer
                // loop without them...
                goto iteration_end;
            }
            snprintf(label, 19, "cmd_label%d", i);
            if (!strcmp(optval[0], label))
            {
                if (opts->cmd_labels[i])
                {
                    // ignore the new one
                    g_strfreev(optval);
                }
                else
                {
                    opts->cmd_labels[i] = optval[1];
                    g_free(optval[0]);
                    g_free(optval);
                }
                // I so hate goto, but C does not allow to continue the outer
                // loop without them...
                goto iteration_end;
            }
        }
        // still here, so nothing freed yet. Free the entire array now
        g_strfreev(optval);
iteration_end:
        // don't need to do a thing, but some versions of GCC need a statement
        i = 0;
    }
    if (fclose(file))
        fprintf(stderr, "bbbm: could not close '%s': %s\n",
                filename, g_strerror(errno));
    if (!opts->set_cmd)
    {
        fprintf(stderr, "bbbm: option 'set_command' missing. "
                "Using default value '%s'\n", SET_COMMAND);
        opts->set_cmd = g_strdup(SET_COMMAND);
    }
    if (!opts->view_cmd)
    {
        fprintf(stderr, "bbbm: option 'view_command' missing. "
                "Using default value '%s'\n", VIEW_COMMAND);
        opts->view_cmd = g_strdup(VIEW_COMMAND);
    }
    if (!thumb_size)
    {
        fprintf(stderr, "bbbm: option 'thumb_size' missing. "
                "Using default value '%dx%d'\n", THUMB_WIDTH, THUMB_HEIGHT);
        opts->thumb_width = THUMB_WIDTH;
        opts->thumb_height = THUMB_HEIGHT;
    }
    else
    {
        g_free(thumb_size);
        if (opts->thumb_width <= 0)
        {
            fprintf(stderr, "bbbm: thumb width <= 0. Using value 1\n");
            opts->thumb_width = 1;
        }
        else if (opts->thumb_width > MAX_WIDTH)
        {
            fprintf(stderr, "bbbm: thumb width > %d. Using value %d\n",
                    MAX_WIDTH, MAX_WIDTH);
            opts->thumb_width = MAX_WIDTH;
        }
        if (opts->thumb_height <= 0)
        {
            fprintf(stderr, "bbbm: thumb height <= 0. Using value 1\n");
            opts->thumb_height = 1;
        }
        else if (opts->thumb_height > MAX_HEIGHT)
        {
            fprintf(stderr, "bbbm: thumb height > %d. Using value %d\n",
                    MAX_HEIGHT, MAX_HEIGHT);
            opts->thumb_height = MAX_HEIGHT;
        }
    }
    if (!thumb_cols)
    {
        fprintf(stderr, "bbbm: option 'thumb_cols' missing. "
                "Using default value '%d'\n", THUMB_COLS);
        opts->thumb_cols = THUMB_COLS;
    }
    else
    {
        g_free(thumb_cols);
        if (opts->thumb_cols <= 0)
        {
            fprintf(stderr, "bbbm: option 'thumb_cols' <= 0. Using value 1\n");
            opts->thumb_cols = 1;
        }
        else if (opts->thumb_cols > MAX_COLS)
        {
            fprintf(stderr, "bbbm: option 'thumb_cols' > %d. Using value %d\n",
                    MAX_COLS, MAX_COLS);
            opts->thumb_cols = MAX_COLS;
        }
    }
    if (!filename_label)
    {
        fprintf(stderr, "bbbm: option 'filename_label' missing. "
                "Using default value 'FALSE'\n");
        opts->filename_label = FALSE;
    }
    else
        g_free(filename_label);
    if (!filename_title)
    {
        fprintf(stderr, "bbbm: option 'filename_title' missing. "
                "Using default value 'FALSE'\n");
        opts->filename_title = FALSE;
    }
    else
        g_free(filename_title);
    return opts;
}
