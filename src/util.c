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
#include <glib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "util.h"

const gchar *extension(const gchar *file)
{
    guint len = strlen(file);
    while (--len > 0 && file[len] != '.' && file[len] != '/')
        ;
    // len is now either 0, or files[len] is . or /
    // only if files[len] is . it is an extension
    return (file[len] == '.' ? file + len : NULL);
}

void dirname(const gchar *file, gchar *dir)
{
    guint len = strlen(file);
    while (--len > 0 && file[len] != '/')
        ;
    // len is either 0, or file[len] is /
    // file up till len is the dirname
    strncpy(dir, file, len);
    dir[len] = 0;
}

gint makedirs(const gchar *dir)
{
    if (!g_file_test(dir, G_FILE_TEST_EXISTS))
    {
        guint len = strlen(dir);
        gchar parent[PATH_MAX];
        if (dir[len - 1] == '/')
        {
            // chop of the trailing / first without altering dir
            gchar dir2[PATH_MAX];
            strncpy(dir2, dir, len - 1);
            dirname(dir2, parent);
        }
        else
            dirname(dir, parent);
        if (strcmp(parent, ""))
        {
            // there is a parent; create it if necessary
            gint rc;
            if ((rc = makedirs(parent)))
                return rc;
        }
        // return the result of mkdir, which is 0 upon success
        return mkdir(dir, 0755);
    }
    return 0;
}

GSList *listdir(const gchar *dir)
{
    DIR *d;
    struct dirent *entry;
    GSList *files = NULL;

    if (!(d = opendir(dir)))
        // files is still NULL
        return files;
    while ((entry = readdir(d)))
    {
        gchar *file = g_strconcat(dir, "/", entry->d_name, NULL);
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
        {
            files = g_slist_append(files, file);
        }
        else
        {
            // not a file, so free it now
            g_free(file);
        }
    }
    closedir(d);
    return files;
}
