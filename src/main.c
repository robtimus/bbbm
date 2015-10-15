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
#include <sys/stat.h>
#include <sys/wait.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include "config.h"
#include "bbbm.h"
#include "options.h"
#include "util.h"
#include "compat.h"

static void clean_up_child_process(int signal_number) {
    wait(NULL);
}

int main(int argc, char *argv[]) {
    static const struct option long_opts[] = {
        {"config",  required_argument, 0, 'c'},
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {0,         0,                 0,  0},
    };
    static const gchar *short_opts = "c:hv";
    gint c;
    gchar *home_dir;
    gchar *config_file = NULL;
    BBBMOptions *options;
    BBBM *bbbm;
    struct sigaction sigchld_action;

    memset(&sigchld_action, 0, sizeof(sigchld_action));
    sigchld_action.sa_handler = clean_up_child_process;
    sigaction(SIGCHLD, &sigchld_action, NULL);

    gtk_init(&argc, &argv);

#if HAVE_GETOPT_LONG != 1
    #error getopt_long is not defined
#endif
    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (c) {
            case 'c':
                g_debug("using config file '%s'", optarg);
                config_file = bbbm_util_absolute_path(optarg);
                break;
            case 'h':
                printf("Usage: "PACKAGE" [OPTIONS] [COLLECTION]\n");
                printf("Opens the Blackbox background manager.\n");
                printf("\n");
                printf("  -c, --config <file>  Use <file> as configuration file\n");
                printf("  -h, --help           Output this help and exit\n");
                printf("  -v, --version        Output version information and exit\n");
                return 0;
            case 'v':
                printf(PACKAGE_STRING"\n");
                printf(BBBM_COPYRIGHT"\n");
                printf("License GPLv2: GNU GPL version 2 <http://www.gnu.org/licenses/gpl.html>\n");
                printf("This is free software; you are free to change and redistribute it.\n");
                printf("There is NO WARRANTY, to the extent permitted by law.\n");
                printf("Written by "BBBM_AUTHOR"\n");
                return 0;
            default:
                fprintf(stderr, "Try `"PACKAGE" --help` for more information.\n");
                return 1;
        }
    }

    if (config_file == NULL) {
        config_file = g_strjoin("/", g_get_home_dir(), BBBM_HOME_DIR, BBBM_CONFIG_FILE, NULL);
    }
    home_dir = g_path_get_dirname (config_file);
    if (!g_file_test(home_dir, G_FILE_TEST_IS_DIR)) {
        if (!g_file_test(home_dir, G_FILE_TEST_EXISTS)) {
#if HAVE_MKDIR != 1
    #error mkdir is not defined
#endif
            mkdir(home_dir, 0755);
        } else {
            g_critical("'%s' is not a directory", home_dir);
            g_free(config_file);
            g_free(home_dir);
            return 1;
        }
    }
    g_free(home_dir);

    if (!g_file_test(config_file, G_FILE_TEST_EXISTS)) {
        options = bbbm_options_new();
        bbbm_options_write_to_file(options, config_file);
        /* do not exit */
    } else {
        options = bbbm_options_read_from_file(config_file);
        if (options == NULL) {
            /* bbbm_options_read printed the error */
            g_free(config_file);
            return 1;
        }
    }
    bbbm = bbbm_new(options, config_file, optind < argc ? argv[optind] : NULL);

    gtk_main();
    bbbm_destroy(bbbm);
    bbbm_options_write_to_file(options, config_file);
    /* do not exit */
    bbbm_options_destroy(options);
    g_free(config_file);
    return 0;
}
