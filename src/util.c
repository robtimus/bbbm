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
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include "config.h"
#include "util.h"

static gboolean bbbm_util_has_ext(const gchar *file, const gchar *ext);

gboolean bbbm_str_empty(const gchar *str) {
    return str == NULL || *str == '\0';
}

gboolean bbbm_str_equals(const gchar *str1, const gchar *str2) {
    if (str1 == NULL && str2 == NULL) {
        return TRUE;
    }
    if (str1 == NULL || str2 == NULL) {
        return FALSE;
    }
    return strcmp(str1, str2) == 0;
}

gboolean bbbm_util_is_image(const gchar *filename) {
    static const gchar *extensions[] = { ".jpg", ".jpeg", ".gif", ".ppm", ".pgm", NULL };
    guint i;

    if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
        return FALSE;
    }
    for (i = 0; extensions[i] != NULL; ++i) {
        if (bbbm_util_has_ext(filename, extensions[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

gchar *bbbm_util_get_command(const gchar *command, const gchar *filename) {
    gchar *start, *end;

    start = (gchar *) command;
    end = strstr(start, "%1");

    if (end != NULL) {
        GString *result = g_string_sized_new(1024);
        while (end != NULL) {
            while (start != end) {
                result = g_string_append_c(result, *start++);
            }
            result = g_string_append(result, filename);
            start += 2;
            end = strstr(start, "%1");
        }
        /* copy the rest */
        while (*start != '\0') {
            result = g_string_append_c(result, *start++);
        }
        return g_string_free(result, FALSE);
    } else {
        /* no %1 in the command, so add the filename to the end */
        return g_strdup_printf("%s \"%s\"", command, filename);
    }
}

void bbbm_util_execute(const gchar *command, const gchar *filename) {
    gchar *cmd;

    cmd = bbbm_util_get_command(command, filename);
    bbbm_util_execute_cmd(cmd);
    g_free(cmd);
}

void bbbm_util_execute_cmd(const gchar *command) {
    pid_t pid;

    g_debug("executing command '%s'", command);
    if (bbbm_str_empty(command)) {
        return;
    }
#if HAVE_FORK != 1
    #error fork is not defined
#endif
#if HAVE_WORKING_FORK != 1
    #error fork does not work
#endif
    pid = fork();
    if (pid == 0) {
        /* use system instead of exec* for less strict commands */
        gint result;

        result = system(command);
        if (result == -1) {
            g_critical("could not execute '%s': %s", command, g_strerror(errno));
        }
        exit(WEXITSTATUS(result));
    } else if (pid == -1) {
        g_critical("could not execute '%s': %s", command, g_strerror(errno));
    }
}

gchar *bbbm_util_dirname(const gchar *filename) {
    gchar *dir, *dirname;

    dir = g_path_get_dirname(filename);
    dirname = g_strconcat(dir, "/", NULL);
    g_free(dir);
    return dirname;
}

gchar *bbbm_util_absolute_path(const gchar *path) {
    if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
        gchar *current, *dir;

        current = g_get_current_dir();
        if (chdir(path) == -1) {
            g_warning("could not change directory to '%s': %s", path, g_strerror(errno));
            g_free(current);
            return g_strdup(path);
        }
        dir = g_get_current_dir();
        if (chdir(current) == -1) {
           g_warning("could not change directory to '%s': %s", current, g_strerror(errno));
        }
        g_free(current);
        return dir;
    } else {
        gchar *current, *dir, *file, *absolute_file;

        current = g_get_current_dir();
        dir = g_path_get_dirname(path);
        if (chdir(dir) == -1) {
            g_warning("could not change directory to '%s': %s", dir, g_strerror(errno));
            g_free(current);
            g_free(dir);
            return g_strdup(path);
        }
        g_free(dir);
        dir = g_get_current_dir();
        file = g_path_get_basename(path);
        absolute_file = g_strjoin("/", dir, file, NULL);
        if (chdir(current) == -1) {
           g_warning("could not change directory to '%s': %s", current, g_strerror(errno));
        }
        g_free(current);
        g_free(dir);
        g_free(file);
        return absolute_file;
    }
}

GList *bbbm_util_listdir(const gchar *dir) {
    GList *files = NULL;
    GError *error = NULL;
    const gchar *entry;
    GDir *d;

    d = g_dir_open(dir, 0, &error);
    if (d == NULL) {
        g_critical("could not open dir '%s' for reading: %s", dir, error->message);
        g_error_free(error);
        return NULL;
    }
    while ((entry = g_dir_read_name(d)) != NULL) {
        gchar *file;

        file = g_strjoin("/", dir, entry, NULL);
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR)) {
            files = g_list_append(files, bbbm_util_absolute_path(file));
        }
        g_free(file);
    }
    g_dir_close(d);
    return files;
}

static gboolean bbbm_util_has_ext(const gchar *file, const gchar *ext) {
    /* code inspired by GLib's g_str_has_suffix */
    size_t file_len, ext_len;

    if (bbbm_str_empty(file)) {
        return FALSE;
    }
    if (bbbm_str_empty(ext)) {
        return TRUE;
    }
    file_len = strlen(file);
    ext_len = strlen(ext);
    if (file_len < ext_len) {
        return FALSE;
    }
    return strcmp(file + file_len - ext_len, ext) == 0;
}
