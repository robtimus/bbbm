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
#include <sys/stat.h>
#include <sys/wait.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include "bbbm.h"
#include "options.h"

static void clean_up_child_process(int signal_number)
{
    wait(NULL);
}

int main(int argc, char *argv[])
{
    static const struct option long_opts[] =
    {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    static const gchar *short_opts = "hv";
    gint c;
    gchar *homedir, *config;
    struct options *opts;
    BBBM *bbbm;

    struct sigaction sigchld_action;
    memset(&sigchld_action, 0, sizeof(sigchld_action));
    sigchld_action.sa_handler = clean_up_child_process;
    sigaction(SIGCHLD, &sigchld_action, NULL);

    gtk_init(&argc, &argv);

    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
        switch (c)
        {
            case 'h':
                printf("Usage: bbbm [OPTIONS] [COLLECTION]\n");
                printf("Opens the Blackbox background manager.\n");
                printf("\n");
                printf("  -h, --help     Output this help and exit\n");
                printf("  -v, --version  ");
                printf("Output version information and exit\n");
                return 0;
            case 'v':
                printf("bbbm %s\n", VERSION);
                printf("Written by Rob Spoor\n");
                printf("\n");
                printf("CopyRight 2004 Rob Spoor\n");
                return 0;
            default:
                fprintf(stderr, "Try `bbbm --help` for more information.\n");
                return 1;
        }
    homedir = g_strconcat(g_getenv("HOME"), BBBM_DIR, NULL);
    if (!g_file_test(homedir, G_FILE_TEST_IS_DIR))
    {
        if (!g_file_test(homedir, G_FILE_TEST_EXISTS))
            mkdir(homedir, 0755);
        else
        {
            fprintf(stderr, "bbbm: '%s' is not a directory\n", homedir);
            g_free(homedir);
            return 1;
        }
    }
    config = g_strconcat(homedir, BBBM_CONFIG, NULL);
    g_free(homedir);
    if (!g_file_test(config, G_FILE_TEST_EXISTS))
    {
        opts = bbbm_options_new();
        if (bbbm_options_write(opts, config))
            fprintf(stderr, "bbbm: could not write to '%s'\n", config);
        /* do not exit */
    }
    else if (!(opts = bbbm_options_read(config)))
    {
        /* bbbm_options_read printed the error */
        g_free(config);
        return 1;
    }
    /* else file exists and read successfully */
    bbbm = bbbm_new(opts, config, (optind < argc ? argv[optind] : NULL));

    gtk_main();
    bbbm_destroy(bbbm);
    if (bbbm_options_write(opts, config))
        fprintf(stderr, "bbbm: could not write to '%s'\n", config);
    /* do not exit */
    bbbm_options_destroy(opts);
    g_free(config);
    return 0;
}
