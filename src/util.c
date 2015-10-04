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
#include <string.h>
#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.h"

const gchar *bbbm_util_extension(const gchar *file)
{
    guint len = strlen(file);
    while (--len > 0 && file[len] != '.' && file[len] != '/')
        ;
    // len is now either 0, or files[len] is . or /
    // only if files[len] is . it is an extension
    return (file[len] == '.' ? file + len : NULL);
}

gchar *bbbm_util_dirname(const gchar *file)
{
    gchar *dir = g_path_get_dirname(file);
    gchar *dirname = g_strconcat(dir, "/", NULL);
    g_free(dir);
    return dirname;
}

gint bbbm_util_makedirs(const gchar *dir)
{
    if (!g_file_test(dir, G_FILE_TEST_EXISTS))
    {
        gchar *parent = g_path_get_dirname(dir);
        if (strcmp(parent, "/"))
        {
            // there is a parent; create it if necessary
            gint rc;
            if ((rc = bbbm_util_makedirs(parent)))
            {
                g_free(parent);
                return rc;
            }
        }
        g_free(parent);
        // return the result of mkdir, which is 0 upon success
        return mkdir(dir, 0755);
    }
    return 0;
}

gchar *bbbm_util_absolute_path(const gchar *path)
{
    gchar *dir, *file, *abs, *current;
    if (g_file_test(path, G_FILE_TEST_IS_DIR))
    {
        current = g_get_current_dir();
        if (chdir(path) == -1)
        {
            g_free(current);
            return g_strdup(path);
        }
        dir = g_get_current_dir();
        chdir(current);
        g_free(current);
        return dir;
    }
    else
    {
        dir = g_path_get_dirname(path);
        file = g_path_get_basename(path);
        current = g_get_current_dir();
        if (chdir(dir) == -1)
        {
            g_free(dir);
            g_free(file);
            g_free(current);
            return g_strdup(path);
        }
        g_free(dir);
        dir = g_get_current_dir();
        abs = g_strjoin("/", dir, file, NULL);
        chdir(current);
        g_free(current);
        g_free(dir);
        g_free(file);
        return abs;
    }
}

GList *bbbm_util_listdir(const gchar *dir)
{
    GDir *d;
    const gchar *entry;
    GList *files = NULL;
    GError *error = NULL;

    if (!(d = g_dir_open(dir, 0, &error)))
    {
        fprintf(stderr, "bbbm: could not open dir '%s' for reading: %s\n",
                dir, error->message);
        g_error_free(error);
        // files is still NULL
        return files;
    }
    while ((entry = g_dir_read_name(d)))
    {
        gchar *file = g_strconcat(dir, "/", entry, NULL);
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
        {
            gchar *file2 = bbbm_util_absolute_path(file);
            g_free(file);
            files = g_list_append(files, file2);
        }
        else
        {
            // not a file, so free it now
            g_free(file);
        }
    }
    g_dir_close(d);
    return files;
}
