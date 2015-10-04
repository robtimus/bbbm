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
#include <gtk/gtk.h>
#include "bbbm.h"
#include "image.h"
#include "dialogs.h"
#include "options.h"
#include "util.h"

#define PADDING         5

static gboolean bbbm_exit_window(GtkWidget *widget, GdkEvent *event,
                                 BBBM *bbbm);

static void bbbm_open(BBBM *bbbm);
static inline gboolean bbbm_open0(BBBM *bbbm, const gchar *filename);
static void bbbm_save(BBBM *bbbm);
static void bbbm_save_as(BBBM *bbbm);
static gboolean bbbm_save_as0(BBBM *bbbm, const gchar *filename);
static void bbbm_close(BBBM *bbbm);
static inline void bbbm_close0(BBBM *bbbm);
static void bbbm_exit(BBBM *bbbm);
static void bbbm_add_image(BBBM *bbbm);
static gboolean bbbm_add_image0(BBBM *bbbm, const gchar *filename,
                                const gchar *description, gint index);
static void bbbm_add_directory(BBBM *bbbm);
static void bbbm_add_collection(BBBM *bbbm);
static gboolean bbbm_add_collection0(BBBM *bbbm, const gchar *filename);
static void bbbm_sort_filename(BBBM *bbbm);
static void bbbm_sort_description(BBBM *bbbm);
static void bbbm_create_list(BBBM *bbbm);
static gboolean bbbm_create_list0(BBBM *bbbm, const gchar *filename);
static void bbbm_create_menu(BBBM *bbbm);
static gboolean bbbm_create_menu0(BBBM *bbbm, const gchar *filename);
static inline void bbbm_write_string(FILE *file, const gchar *string);
static void bbbm_random_background(BBBM *bbbm);
static void bbbm_options(BBBM *bbbm);
static void bbbm_about(BBBM *bbbm);

static inline GtkWidget *bbbm_create_menubar(BBBM *bbbm);
static gboolean bbbm_can_close(BBBM *bbbm);
static void bbbm_set_modified(BBBM *bbbm, gboolean modified);
static void bbbm_reset_images(BBBM *bbbm, guint index);
static void bbbm_resize_thumbs(BBBM *bbbm);

static gboolean bbbm_set_image(GtkWidget *widget, GdkEventButton *event,
                               BBBMImage *image);
static gboolean bbbm_popup(GtkWidget *widget, GdkEventButton *event,
                           BBBMImage *image);
static gboolean bbbm_set_status(GtkWidget *widget, GdkEventCrossing *event,
                                BBBMImage *image);
static gboolean bbbm_clear_status(GtkWidget *widget, GdkEventCrossing *event,
                                  BBBMImage *image);

static void bbbm_set(BBBMImage *image);
static void bbbm_view(BBBMImage *image);
static void bbbm_move_back(BBBMImage *image, guint index);
static void bbbm_move_forward(BBBMImage *image, guint index);
static void bbbm_edit(BBBMImage *image);
static void bbbm_insert(BBBMImage *image, guint index);
static void bbbm_delete(BBBMImage *image, guint index);

BBBM *bbbm_new(struct options *opts, const gchar *config,
               const gchar *thumbdir, const gchar *file)
{
    GtkWidget *vbox, *hbox, *menubar, *scroll;
    BBBM *bbbm = g_malloc(sizeof(BBBM));
    bbbm->opts = opts;
    bbbm->config = config;
    bbbm->thumbdir = thumbdir;
    bbbm->filename = NULL;
    bbbm->modified = FALSE;
    bbbm->images = NULL;

    bbbm->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(bbbm->window), 640, 480);
    gtk_window_set_title(GTK_WINDOW(bbbm->window), "bbbm "VERSION);
    g_signal_connect(G_OBJECT(bbbm->window), "delete-event",
                     G_CALLBACK(bbbm_exit_window), bbbm);
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
    gtk_container_add(GTK_CONTAINER(bbbm->window), vbox);
    menubar = bbbm_create_menubar(bbbm);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    bbbm->table = gtk_table_new(1, 1, TRUE);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll),
                                          bbbm->table);
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    bbbm->file_bar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(bbbm->file_bar), FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), bbbm->file_bar, TRUE, TRUE, 0);
    bbbm->file_cid =
       gtk_statusbar_get_context_id(GTK_STATUSBAR(bbbm->file_bar), "Filename");
    gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid,
                       "Untitled");
    bbbm->file_mod_cid =
                    gtk_statusbar_get_context_id(GTK_STATUSBAR(bbbm->file_bar),
                                                 "FilenameModified");
    bbbm->image_bar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(bbbm->image_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), bbbm->image_bar, TRUE, TRUE, 0);
    bbbm->image_cid =
         gtk_statusbar_get_context_id(GTK_STATUSBAR(bbbm->image_bar), "Image");
    gtk_widget_show_all(bbbm->window);

    if (file && !g_file_test(file, G_FILE_TEST_IS_REGULAR))
    {
        gchar *message = g_strconcat("Could not open '", file, "'", NULL);
        bbbm_dialogs_error(GTK_WINDOW(bbbm->window), message);
        g_free(message);
    }
    else if (file)
        bbbm_open0(bbbm, file);
    return bbbm;
}

void bbbm_destroy(BBBM *bbbm)
{
    /* closing the window already destroyed all fields */
    g_free(bbbm);
}

static gboolean bbbm_exit_window(GtkWidget *widget, GdkEvent *event,
                                 BBBM *bbbm)
{
    if (!bbbm_can_close(bbbm))
        /* block other handlers, like closing the window! */
        return TRUE;
    bbbm_close0(bbbm);
    gtk_widget_destroy(bbbm->window);
    gtk_main_quit();
    return FALSE;
}

static void bbbm_open(BBBM *bbbm)
{
    gchar *filename;

    if (!bbbm_can_close(bbbm))
        return;
    filename = bbbm_dialogs_get_file(GTK_WINDOW(bbbm->window),
                                     "Open a collection");
    if (filename)
    {
        bbbm_open0(bbbm, filename);
        g_free(filename);
    }
}

static inline gboolean bbbm_open0(BBBM *bbbm, const gchar *filename)
{
    bbbm_close0(bbbm);
    if (bbbm_add_collection0(bbbm, filename))
    {
        bbbm_set_modified(bbbm, FALSE);
        bbbm->filename = bbbm_util_absolute_path(filename);
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid,
                           bbbm->filename);
        return TRUE;
    }
    else
    {
        gchar *message = g_strconcat("Could not open '", filename,
                                     "' propertly", NULL);
        bbbm_dialogs_error(GTK_WINDOW(bbbm->window), message);
        g_free(message);
        /* some images might be added, remove them */
        bbbm_close0(bbbm);
        return FALSE;
    }
}

static void bbbm_save(BBBM *bbbm)
{
    if (bbbm->filename && !bbbm_save_as0(bbbm, bbbm->filename))
    {
        gchar *message = g_strconcat("Could not save '", bbbm->filename, "'",
                                     NULL);
        bbbm_dialogs_error(GTK_WINDOW(bbbm->window), message);
        g_free(message);
    }
    else if (!bbbm->filename)
        bbbm_save_as(bbbm);
}

static void bbbm_save_as(BBBM *bbbm)
{
    bbbm_dialogs_save(GTK_WINDOW(bbbm->window), "Save collection as",
                      (save_function)bbbm_save_as0, bbbm);
}

static gboolean bbbm_save_as0(BBBM *bbbm, const gchar *filename)
{
    GList *iter;
    FILE *file;

    /* first save, in case any error occurs */
    if (!(file = fopen(filename, "w")))
        return FALSE;
    for (iter = bbbm->images; iter; iter = iter->next)
        fprintf(file, "%s\n%s\n",
                bbbm_image_get_filename(BBBM_IMAGE(iter->data)),
                bbbm_image_get_description(BBBM_IMAGE(iter->data)));
    fclose(file);
    bbbm_set_modified(bbbm, FALSE);
    if (!bbbm->filename || strcmp(filename, bbbm->filename))
    {
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid);
        g_free(bbbm->filename);
        bbbm->filename = bbbm_util_absolute_path(filename);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid,
                           bbbm->filename);
    }
    return TRUE;
}

static void bbbm_close(BBBM *bbbm)
{
    if (bbbm_can_close(bbbm))
        bbbm_close0(bbbm);
}

static inline void bbbm_close0(BBBM *bbbm)
{
    while (bbbm->images)
    {
        BBBMImage *image = BBBM_IMAGE(bbbm->images->data);
        bbbm->images = g_list_remove(bbbm->images, image);
        bbbm_image_destroy(image);
    }
    g_free(bbbm->filename);
    bbbm->filename = NULL;
    bbbm_set_modified(bbbm, FALSE);
    gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid);
    gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid,
                       "Untitled");
}

static void bbbm_exit(BBBM *bbbm)
{
    g_signal_emit_by_name(G_OBJECT(bbbm->window), "delete-event", NULL, bbbm);
}

static void bbbm_add_image(BBBM *bbbm)
{
    GList *files = bbbm_dialogs_get_files(GTK_WINDOW(bbbm->window),
                                          "Add images");
    while (files)
    {
        gchar *file = (gchar *)files->data;
        files = g_list_remove(files, file);
        if (bbbm_util_is_image(file))
            if (!bbbm_add_image0(bbbm, file, NULL, -1))
            {
                gchar *message = g_strconcat("Could not add image '", file,
                                             "'", NULL);
                bbbm_dialogs_error(GTK_WINDOW(bbbm->window), message);
                g_free(message);
            }
        g_free(file);
    }
}

static gboolean bbbm_add_image0(BBBM *bbbm, const gchar *filename,
                                const gchar *description, gint index)
{
    gchar *thumb, *thumbdir, *command;
    BBBMImage *image;
    guint col, row;

    if (!description)
        description = filename;
    thumb = g_strconcat(bbbm->thumbdir, "/", filename, NULL);
    thumbdir = g_path_get_dirname(thumb);
    if (bbbm_util_makedirs(thumbdir))
    {
        fprintf(stderr, "bbbm: could not create directory '%s': %s\n",
                thumbdir, g_strerror(errno));
        g_free(thumbdir);
        g_free(thumb);
        return FALSE;
    }
    g_free(thumbdir);

    command = g_strconcat("convert -size ", bbbm->opts->thumb_size,
                          " -resize ", bbbm->opts->thumb_size, " \"", filename,
                          "\" \"", thumb, "\"", NULL);
    if (system(command))
    {
        g_free(thumb);
        g_free(command);
        return FALSE;
    }
    g_free(command);

    image = bbbm_image_new(bbbm, filename, description, thumb);
    g_free(thumb);
    g_signal_connect(G_OBJECT(image->box), "button-press-event",
                     G_CALLBACK(bbbm_set_image), image);
    g_signal_connect(G_OBJECT(image->box), "button-release-event",
                     G_CALLBACK(bbbm_popup), image);
    g_signal_connect(G_OBJECT(image->box), "enter-notify-event",
                     G_CALLBACK(bbbm_set_status), image);
    g_signal_connect(G_OBJECT(image->box), "leave-notify-event",
                     G_CALLBACK(bbbm_clear_status), image);
    gtk_widget_show(image->box);
    g_object_ref(image->box);
    if (index == -1)
    {
        guint img_num = g_list_length(bbbm->images);
        col = img_num % bbbm->opts->thumb_cols;
        row = img_num / bbbm->opts->thumb_cols;
        bbbm->images = g_list_append(bbbm->images, image);
    }
    else
    {
        col = index % bbbm->opts->thumb_cols;
        row = index / bbbm->opts->thumb_cols;
        bbbm->images = g_list_insert(bbbm->images, image, index);
        bbbm_reset_images(bbbm, index + 1);
    }
    gtk_table_attach(GTK_TABLE(bbbm->table), image->box,
                     col, col + 1, row, row + 1, 0, 0, PADDING, PADDING);
    bbbm_set_modified(bbbm, TRUE);
    return TRUE;
}

static void bbbm_add_directory(BBBM *bbbm)
{
    GList *files = bbbm_dialogs_get_files_dir(GTK_WINDOW(bbbm->window),
                                              "Add a directory");
    while (files)
    {
        gchar *file = (gchar *)files->data;
        files = g_list_remove(files, file);
        if (bbbm_util_is_image(file))
            if (!bbbm_add_image0(bbbm, file, NULL, -1))
            {
                gchar *message = g_strconcat("Could not add image '", file,
                                             "'", NULL);
                bbbm_dialogs_error(GTK_WINDOW(bbbm->window), message);
                g_free(message);
            }
        g_free(file);
    }
}

static void bbbm_add_collection(BBBM *bbbm)
{
    GList *files = bbbm_dialogs_get_files(GTK_WINDOW(bbbm->window),
                                              "Add collections");
    while (files)
    {
        gchar *file = (gchar *)files->data;
        files = g_list_remove(files, file);
        if (!bbbm_add_collection0(bbbm, file))
        {
            gchar *message = g_strconcat("Could not add collection '", file,
                                         "' properly", NULL);
            bbbm_dialogs_error(GTK_WINDOW(bbbm->window), message);
            g_free(message);
        }
        g_free(file);
    }
}

static gboolean bbbm_add_collection0(BBBM *bbbm, const gchar *filename)
{
    gboolean result = TRUE;

    gchar file[PATH_MAX], desc[PATH_MAX];
    FILE *f = fopen(filename, "r");
    if (!file)
        return FALSE;
    while (fgets(file, PATH_MAX, f))
    {
        g_strstrip(file);
        if (fgets(desc, PATH_MAX, f))
            g_strstrip(desc);
        else
            strcpy(desc, file);
        if (bbbm_util_is_image(file))
            result = result && bbbm_add_image0(bbbm, file, desc, -1);
        else
            result = FALSE;
    }
    fclose(f);
    return result;
}

static void bbbm_sort_filename(BBBM *bbbm)
{
    if (!bbbm->images)
        return;
    bbbm->images = g_list_sort(bbbm->images,
                               (GCompareFunc)bbbm_image_compare_filename);
    bbbm_reset_images(bbbm, 0);
    bbbm_set_modified(bbbm, TRUE);
}

static void bbbm_sort_description(BBBM *bbbm)
{
    if (!bbbm->images)
        return;
    bbbm->images = g_list_sort(bbbm->images,
                               (GCompareFunc)bbbm_image_compare_description);
    bbbm_reset_images(bbbm, 0);
    bbbm_set_modified(bbbm, TRUE);
}

static void bbbm_create_list(BBBM *bbbm)
{
    bbbm_dialogs_save(GTK_WINDOW(bbbm->window), "Create background list",
                      (save_function)bbbm_create_list0, bbbm);
}

static gboolean bbbm_create_list0(BBBM *bbbm, const gchar *filename)
{
    GList *iter;
    FILE *file = fopen(filename, "w");
    if (!file)
        return FALSE;
    for (iter = bbbm->images; iter; iter = iter->next)
        fprintf(file, "%s\n", bbbm_image_get_filename(BBBM_IMAGE(iter->data)));
    fclose(file);
    return TRUE;
}

static void bbbm_create_menu(BBBM *bbbm)
{
    bbbm_dialogs_save(GTK_WINDOW(bbbm->window), "Create background menu",
                      (save_function)bbbm_create_menu0, bbbm);
}

static gboolean bbbm_create_menu0(BBBM *bbbm, const gchar *filename)
{
    GList *iter;
    FILE *file = fopen(filename, "w");
    if (!file)
        return FALSE;
    fprintf(file, "[submenu] (");
    if (bbbm->filename &&
        (bbbm->opts->filename_label || bbbm->opts->filename_title))
    {
        gchar *name = g_path_get_basename(bbbm->filename);
        if (bbbm->opts->filename_label)
        {
            bbbm_write_string(file, name);
            fprintf(file, ")");
        }
        else
            fprintf(file, "Backgrounds)");
        if (bbbm->opts->filename_title)
        {
            fprintf(file, " {");
            bbbm_write_string(file, name);
            fprintf(file, "}\n");
        }
        else
            fprintf(file, "\n");
        g_free(name);
    }
    else
        fprintf(file, "Backgrounds)\n");
    for (iter = bbbm->images; iter; iter = iter->next)
    {
        fprintf(file, "  [exec] (");
        bbbm_write_string(file,
                          bbbm_image_get_description(BBBM_IMAGE(iter->data)));
        fprintf(file, ") {");
        bbbm_write_string(file, bbbm->opts->set_cmd);
        fprintf(file, " \"");
        bbbm_write_string(file,
                          bbbm_image_get_filename(BBBM_IMAGE(iter->data)));
        fprintf(file, "\"}\n");
    }
    fclose(file);
    return TRUE;
}

static inline void bbbm_write_string(FILE *file, const gchar *string)
{
    while (*string)
        switch (*string)
        {
            case '\\': /* fallthrough */
            case '(': /* fallthrough */
            case ')': /* fallthrough */
            case '{': /* fallthrough */
            case '}':
                fputc('\\', file); 
                /* fallthrough */
            default:
                fputc(*string++, file);
                break;
        }
}

static void bbbm_random_background(BBBM *bbbm)
{
    if (bbbm->images)
    {
        GRand *rand = g_rand_new();
        guint32 index = g_rand_int_range(rand, 0, g_list_length(bbbm->images));
        BBBMImage *image = BBBM_IMAGE(g_list_nth_data(bbbm->images, index));
        bbbm_util_execute(bbbm->opts->set_cmd, bbbm_image_get_filename(image));
        g_rand_free(rand);
    }
}

static void bbbm_options(BBBM *bbbm)
{
    guint changed = bbbm_dialogs_options(GTK_WINDOW(bbbm->window), bbbm->opts);
    if (changed & OPTIONS_THUMB_SIZE_CHANGED)
        bbbm_resize_thumbs(bbbm);
    if (changed & OPTIONS_THUMB_COLS_CHANGED)
        bbbm_reset_images(bbbm, 0);
    if (changed)
        bbbm_options_write(bbbm->opts, bbbm->config);
}

static void bbbm_about(BBBM *bbbm)
{
    bbbm_dialogs_about(GTK_WINDOW(bbbm->window));
}

static inline GtkWidget *bbbm_create_menubar(BBBM *bbbm)
{
    static guint n_items = 22;
    static GtkItemFactoryEntry items[] =
    {
        {"/_File", NULL, NULL, 0, "<Branch>"},
        {"/File/_Open...", "<ctrl>O", bbbm_open, 0, NULL},
        {"/File/_Save", "<ctrl>S", bbbm_save, 0, NULL},
        {"/File/Save _As...", "<ctrl><shift>S", bbbm_save_as, 0, NULL},
        {"/File/_Close", "<ctrl>X", bbbm_close, 0, NULL},
        {"/File/sep", NULL, NULL, 0, "<Separator>"},
        {"/File/E_xit", "<alt>F4", bbbm_exit, 0, NULL},
        {"/_Edit", NULL, NULL, 0, "<Branch>"},
        {"/Edit/_Add Images...", "<ctrl>A", bbbm_add_image, 0, NULL},
        {"/Edit/Add _Directory...", "<ctrl>D", bbbm_add_directory, 0, NULL},
        {"/Edit/Add _Collections...", "<ctrl>C", bbbm_add_collection, 0, NULL},
        {"/Edit/sep", NULL, NULL, 0, "<Separator>"},
        {"/Edit/Sort On _Filename", "<ctrl><shift>F",
         bbbm_sort_filename, 0, NULL},
        {"/Edit/Sort On D_escription", "<ctrl><shift>D",
         bbbm_sort_description, 0, NULL},
        {"/_Tools", NULL, NULL, 0, "<Branch>"},
        {"/Tools/Create _List...", "<ctrl>L", bbbm_create_list, 0, NULL},
        {"/Tools/Create _Menu...", "<ctrl>M", bbbm_create_menu, 0, NULL},
        {"/Tools/_Random Background", "<ctrl>R",
         bbbm_random_background, 0, NULL},
        {"/Tools/sep", NULL, NULL, 0, "<Separator>"},
        {"/Tools/_Options...", "<alt>P", bbbm_options, 0, NULL},
        {"/_Help", NULL, NULL, 0, "<Branch>"},
        {"/Help/_About", NULL, bbbm_about, 0, NULL}
    };
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    GtkItemFactory *factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
                                                   accel_group);
    gtk_item_factory_create_items(factory, n_items, items, bbbm);
    gtk_window_add_accel_group(GTK_WINDOW(bbbm->window), accel_group);
    return gtk_item_factory_get_widget(factory, "<main>");
}

static gboolean bbbm_can_close(BBBM *bbbm)
{
    if (bbbm->modified)
    {
        static const gchar *text = "Colletion has been modified. "
                                   "Close anyway?";
        return bbbm_dialogs_question(GTK_WINDOW(bbbm->window),
                                     "Close collection?", text);
    }
    return TRUE;
}

static void bbbm_set_modified(BBBM *bbbm, gboolean modified)
{
    if (modified == bbbm->modified)
        return;
    if ((bbbm->modified = modified) && bbbm->filename)
    {
        gchar *filename = g_strconcat(bbbm->filename, "*", NULL);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_mod_cid,
                           filename);
        g_free(filename);
    }
    else if (bbbm->modified)
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_mod_cid,
                           "Untitled*");
    else
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_mod_cid);
}

static void bbbm_reset_images(BBBM *bbbm, guint index)
{
    GList *iter = g_list_nth(bbbm->images, index);
    guint num_imgs = g_list_length(bbbm->images);
    guint num_cols = bbbm->opts->thumb_cols;
    guint cols = MIN(num_cols, num_imgs);
    guint rows = num_imgs / num_cols + (num_imgs % num_cols == 0 ? 0 : 1);
    for ( ; iter; iter = iter->next, ++index)
    {
        guint c = index % num_cols;
        guint r = index / num_cols;
        gtk_container_remove(GTK_CONTAINER(bbbm->table),
                             BBBM_IMAGE(iter->data)->box);
        gtk_table_attach(GTK_TABLE(bbbm->table), BBBM_IMAGE(iter->data)->box,
                         c, c + 1, r, r + 1, 0, 0, PADDING, PADDING);
    }
    gtk_table_resize(GTK_TABLE(bbbm->table), MAX(rows, 1), MAX(cols, 1));
}

static void bbbm_resize_thumbs(BBBM *bbbm)
{
    gboolean was_modified = bbbm->modified;
    GList *old = bbbm->images;
    bbbm->images = NULL;
    while (old)
    {
        BBBMImage *image = BBBM_IMAGE(old->data);
        old = g_list_remove(old, image);
        bbbm_add_image0(bbbm, bbbm_image_get_filename(image),
                        bbbm_image_get_description(image), -1);
        bbbm_image_destroy(image);
    }
    bbbm_set_modified(bbbm, was_modified);
}

static gboolean bbbm_set_image(GtkWidget *widget, GdkEventButton *event,
                               BBBMImage *image)
{
    if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
        bbbm_util_execute(image->bbbm->opts->set_cmd,
                          bbbm_image_get_filename(image));
    return FALSE;
}

static gboolean bbbm_popup(GtkWidget *widget, GdkEventButton *event,
                           BBBMImage *image)
{
    if (event->type == GDK_BUTTON_RELEASE && event->button == 3)
    {
        GtkWidget *popup;
        gint index = g_list_index(image->bbbm->images, image);
        static guint n_items = 9;
        GtkItemFactoryEntry items[] =
        {
            {"/_Set", NULL, bbbm_set, 0, NULL},
            {"/_View", NULL, bbbm_view, 0, NULL},
            {"/sep", NULL, NULL, 0, "<Separator>"},
            {"/Move _Back...", NULL, bbbm_move_back, index, NULL},
            {"/Move _Forward...", NULL, bbbm_move_forward, index, NULL},
            {"/sep", NULL, NULL, 0, "<Separator>"},
            {"/_Edit Description...", NULL, bbbm_edit, 0, NULL},
            {"/_Insert Images..", NULL, bbbm_insert, index, NULL},
            {"/_Delete", NULL, bbbm_delete, index, NULL}
        };
        GtkItemFactory *factory = gtk_item_factory_new(GTK_TYPE_MENU,
                                                       "<popup>", NULL);
        gtk_item_factory_create_items(factory, n_items, items, image);
        popup = gtk_item_factory_get_widget(factory, "<popup>");
        if (index == 0)
            gtk_widget_set_sensitive(
                   gtk_item_factory_get_item(factory, "/Move Back..."), FALSE);
        if (index == g_list_length(image->bbbm->images) - 1)
            gtk_widget_set_sensitive(
                gtk_item_factory_get_item(factory, "/Move Forward..."), FALSE);
        gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL,
                       event->button, event->time);
    }
    return FALSE;
}

static gboolean bbbm_set_status(GtkWidget *widget, GdkEventCrossing *event,
                                BBBMImage *image)
{
    gtk_statusbar_push(GTK_STATUSBAR(image->bbbm->image_bar),
                       image->bbbm->image_cid,
                       bbbm_image_get_description(image));
    return FALSE;
}

static gboolean bbbm_clear_status(GtkWidget *widget, GdkEventCrossing *event,
                                  BBBMImage *image)
{
    gtk_statusbar_pop(GTK_STATUSBAR(image->bbbm->image_bar),
                      image->bbbm->image_cid);
    gtk_statusbar_pop(GTK_STATUSBAR(image->bbbm->image_bar),
                      image->bbbm->image_cid);
    return FALSE;
}

static void bbbm_set(BBBMImage *image)
{
    bbbm_util_execute(image->bbbm->opts->set_cmd,
                      bbbm_image_get_filename(image));
}

static void bbbm_view(BBBMImage *image)
{
    bbbm_util_execute(image->bbbm->opts->view_cmd,
                      bbbm_image_get_filename(image));
}

static void bbbm_move_back(BBBMImage *image, guint index)
{
    guint new_pos;
    gint move = bbbm_dialogs_move(GTK_WINDOW(image->bbbm->window), "Move back",
                                  index);
    if (move == -1)
        return;
    new_pos = index - move;
    image->bbbm->images = g_list_remove(image->bbbm->images, image);
    image->bbbm->images = g_list_insert(image->bbbm->images, image, new_pos);
    bbbm_reset_images(image->bbbm, new_pos);
    bbbm_set_modified(image->bbbm, TRUE);
}

static void bbbm_move_forward(BBBMImage *image, guint index)
{
    guint new_pos;
    guint limit = g_list_length(image->bbbm->images) - index - 1;
    gint move = bbbm_dialogs_move(GTK_WINDOW(image->bbbm->window),
                                  "Move forward", limit);
    if (move == -1)
        return;
    new_pos = index + move;
    image->bbbm->images = g_list_remove(image->bbbm->images, image);
    image->bbbm->images = g_list_insert(image->bbbm->images, image, new_pos);
    bbbm_reset_images(image->bbbm, index);
    bbbm_set_modified(image->bbbm, TRUE);
}

static void bbbm_edit(BBBMImage *image)
{
    const gchar *old_desc = bbbm_image_get_description(image);
    gchar *new_desc = bbbm_dialogs_edit(GTK_WINDOW(image->bbbm->window),
                                        old_desc);
    if (new_desc && strcmp(new_desc, old_desc))
    {
        /* normally we would call bbbm_image_set_description.
           However, why do duplicate new_desc only to free it? */
        g_free(image->description);
        image->description = new_desc;
        bbbm_set_modified(image->bbbm, TRUE);
    }
    else if (new_desc)
        g_free(new_desc);
}

static void bbbm_insert(BBBMImage *image, guint index)
{
    GList *files = bbbm_dialogs_get_files(GTK_WINDOW(image->bbbm->window),
                                          "Insert images");
    while (files)
    {
        gchar *file = (gchar *)files->data;
        files = g_list_remove(files, file);
        if (bbbm_util_is_image(file))
            if (!bbbm_add_image0(image->bbbm, file, NULL, index++))
            {
                gchar *message = g_strconcat("Could not add image '", file,
                                             "'", NULL);
                bbbm_dialogs_error(GTK_WINDOW(image->bbbm->window), message);
                g_free(message);
                /* index got increased, decrease again */
                --index;
            }
        g_free(file);
    }
}

static void bbbm_delete(BBBMImage *image, guint index)
{
    BBBM *bbbm = image->bbbm;
    bbbm->images = g_list_remove(bbbm->images, image);
    bbbm_image_destroy(image);
    if (index != g_list_length(bbbm->images))
        bbbm_reset_images(bbbm, index);
    bbbm_set_modified(bbbm, TRUE);
}
