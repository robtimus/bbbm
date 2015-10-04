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
#include <getopt.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include "options.h"
#include "bbbm.h"

gchar *EXTENSIONS[] = BBBM_EXTENSIONS;

static void clean_up_child_process(int signal_number)
{
    // wait for any child
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
    static const char *short_opts = "hv";
    int c;
    struct options opts = {NULL, NULL, NULL, 0};
    gchar homedir[PATH_MAX];
    gchar config[PATH_MAX];
    gchar thumbsdir[PATH_MAX];
    BBBM *bbbm;

    struct sigaction sigchld_action;
    memset(&sigchld_action, 0, sizeof(sigchld_action));
    sigchld_action.sa_handler = clean_up_child_process;
    sigaction(SIGCHLD, &sigchld_action, NULL);

    gtk_init(&argc, &argv);

    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) > -1)
    {
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
    }
    // assume no paths longer than PATH_MAX (which is 4092, large enough)
    g_stpcpy(g_stpcpy(homedir, getenv("HOME")), BBBM_DIR);
    g_stpcpy(g_stpcpy(config, homedir), BBBM_CONFIG);
    g_stpcpy(g_stpcpy(thumbsdir, homedir), BBBM_THUMBS);

    if (!g_file_test(homedir, G_FILE_TEST_IS_DIR))
    {
        if (!g_file_test(homedir, G_FILE_TEST_EXISTS))
            mkdir(homedir, 0755);
        else
        {
            fprintf(stderr, "bbbm: '%s' is not a directory\n", homedir);
            return 1;
        }
    }
    if (!g_file_test(config, G_FILE_TEST_EXISTS))
    {
        create_default_options(&opts);
        if (write_options(&opts, config))
            fprintf(stderr, "bbbm: could not write to '%s'\n", config);
    }
    else if (read_options(&opts, config))
        return 1;
    // else file exists and read successfully

    bbbm = bbbm_new(&opts, config, thumbsdir,
                    (optind < argc ? argv[optind] : NULL));

    gtk_main();

    bbbm_destroy(bbbm);

    // all along, a pointer to opts is used, so just use opts again
    if (write_options(&opts, config))
        fprintf(stderr, "bbbm: could not write to '%s'\n", config);
    destroy_options(&opts);

    if (g_file_test(thumbsdir, G_FILE_TEST_IS_DIR))
    {
        gchar command[PATH_MAX + 10];
        g_stpcpy(g_stpcpy(g_stpcpy(command, "rm -rf "), thumbsdir), "/*");
        if (system(command) == -1)
            fprintf(stderr, "bbbm: could not remove thumbs in %s\n",
                    thumbsdir);
    }

    return 0;
}
