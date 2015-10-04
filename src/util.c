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
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include "util.h"

static gboolean bbbm_util_has_ext(const gchar *file, const gchar *ext);

gboolean bbbm_util_is_image(const gchar *filename)
{
    static const gchar *extensions[] =
                               {".jpg", ".jpeg", ".gif", ".ppm", ".pgm", NULL};
    guint i;
    if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR))
        return FALSE;
    for (i = 0; extensions[i]; ++i)
        if (bbbm_util_has_ext(filename, extensions[i]))
            return TRUE;
    return FALSE;
}

gchar *bbbm_util_get_command(const gchar *command, const gchar *filename)
{
    // FIXME: I still want %1 replacement, which can be put in here
    // For now, return the command with the filename appended between ""
    return g_strconcat(command, " \"", filename, "\"", NULL);
}

void bbbm_util_execute(const gchar *command, const gchar *filename)
{
    gchar *cmd = bbbm_util_get_command(command, filename);
    bbbm_util_execute_cmd(cmd);
    g_free(cmd);
}

void bbbm_util_execute_cmd(const gchar *command)
{
    pid_t pid;
    if (!command || !strcmp(command, ""))
    {
        return;
    }
    if (!(pid = fork()))
    {
        gint ret;
        /* use system instead of exec* for less strict commands */
       if ((ret = system(command)) == -1)
       {
           fprintf(stderr, "bbbm: could not execute '%s': %s\n",
                   command, g_strerror(errno));
       }
       exit(WEXITSTATUS(ret));
    }
    else if (pid == -1)
    {
        fprintf(stderr, "bbbm: could not execute '%s': %s\n",
                command, g_strerror(errno));
    }
}

gchar *bbbm_util_dirname(const gchar *filename)
{
    gchar *dir = g_path_get_dirname(filename);
    gchar *dirname = g_strconcat(dir, "/", NULL);
    g_free(dir);
    return dirname;
}

gchar *bbbm_util_absolute_path(const gchar *path)
{
    if (g_file_test(path, G_FILE_TEST_IS_DIR))
    {
        gchar *current = g_get_current_dir();
        gchar *dir;
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
        gchar *current = g_get_current_dir();
        gchar *dir = g_path_get_dirname(path);
        gchar *file, *abs;
        if (chdir(dir) == -1)
        {
            g_free(current);
            g_free(dir);
            return g_strdup(path);
        }
        g_free(dir);
        dir = g_get_current_dir();
        file = g_path_get_basename(path);
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
    GList *files = NULL;
    GError *error = NULL;
    const gchar *entry;
    GDir *d = g_dir_open(dir, 0, &error);
    if (!d)
    {
        fprintf(stderr, "bbbm: could not open dir '%s' for reading: %s\n",
                dir, error->message);
        g_error_free(error);
        return NULL;
    }
    while ((entry = g_dir_read_name(d)))
    {
        gchar *file = g_strconcat(dir, "/", entry, NULL);
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
            files = g_list_append(files, bbbm_util_absolute_path(file));
        g_free(file);
    }
    g_dir_close(d);
    return files;
}

static gboolean bbbm_util_has_ext(const gchar *file, const gchar *ext)
{
    /* code inspired by GLib's g_str_has_suffix */
    gint file_len, ext_len;

    if (!file)
        return FALSE;
    if (!ext)
        return TRUE;
    file_len = strlen(file);
    ext_len = strlen(ext);
    if (file_len < ext_len)
        return FALSE;
    return strcmp(file + file_len - ext_len, ext) == 0;
}

