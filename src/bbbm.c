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
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "options.h"
#include "image.h"
#include "bbbm.h"
#include "dialogs.h"
#include "util.h"

#define BBBM_SAVE       1
#define BBBM_LIST       2
#define BBBM_MENU       3

#define FILE_CONTEXT    "Filename"
#define IMAGE_CONTEXT   "Image"

typedef gint (*create_func)(BBBM *, const gchar *);
static gint bbbm_save_to(BBBM *, const gchar *);
static gint bbbm_create_list(BBBM *, const gchar *);
static gint bbbm_create_menu(BBBM *, const gchar *);

static inline gboolean bbbm_ask_overwrite(BBBM *, const gchar *);
static inline void bbbm_error_message(BBBM *, const gchar *);
static inline gboolean bbbm_is_image(const gchar *);
static inline GtkWidget *create_chooser(BBBM *, const gchar *, gboolean,
                                        gboolean);
static inline void bbbm_execute(const gchar *, gchar *);

static GtkWidget *bbbm_create_menu_bar(BBBM *);
static void bbbm_set_status(GtkWidget *, GdkEvent *, BBBMImage *);
static void bbbm_clear_status(GtkWidget *, GdkEvent *, BBBM *);
static void bbbm_open(BBBM *);
static void bbbm_open_collection(BBBM *, const gchar *);
static void bbbm_save(BBBM *);
static void bbbm_close(BBBM *);
static void bbbm_exit_window(GtkWidget *, GdkEvent *, BBBM *);
static void bbbm_exit(BBBM *);
static void bbbm_add_image(BBBM *);
static void bbbm_add_image0(BBBM *, gchar *, gchar *, gint);
static void bbbm_add_directory(BBBM *);
static void bbbm_add_collection(BBBM *);
static void bbbm_add_collection0(BBBM *, const gchar *);

static void bbbm_create(BBBM *, gint);
static void bbbm_random_background(BBBM *);
static void bbbm_set_image(GtkWidget *, GdkEvent *, BBBMImage *);
static void bbbm_popup(GtkWidget *, GdkEventButton *, BBBMImage *);
static void bbbm_set(BBBMImage *);
static void bbbm_view(BBBMImage *);
static void bbbm_move_back(BBBMImage *, gint);
static void bbbm_move_forward(BBBMImage *, gint);
static void bbbm_insert(BBBMImage *);
static void bbbm_delete(BBBMImage *);

BBBM *bbbm_new(struct options *opts, const gchar *config,
               const gchar *thumbsdir, const gchar *file)
{
    GtkWidget *vbox, *hbox, *menu_bar, *scroll;

    BBBM *bbbm = g_malloc(sizeof(BBBM));
    bbbm->padding = 5;
    bbbm->config = config;
    bbbm->thumbsdir = thumbsdir;
    bbbm->filename = NULL;
    bbbm->opts = opts;
    bbbm->images = NULL;

    bbbm->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(bbbm->window), 640, 480);
    gtk_window_set_title(GTK_WINDOW(bbbm->window), CAPTION);
    gtk_signal_connect(GTK_OBJECT(bbbm->window), "delete-event",
                       GTK_SIGNAL_FUNC(bbbm_exit_window), bbbm);
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
    menu_bar = bbbm_create_menu_bar(bbbm);
    gtk_widget_show(menu_bar);
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    bbbm->table = gtk_table_new(1, 1, TRUE);
    gtk_widget_show(bbbm->table);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll),
                                          bbbm->table);
    gtk_widget_show(scroll);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    hbox = gtk_hbox_new(TRUE, 0);
    bbbm->file_statusbar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(bbbm->file_statusbar),
                                      FALSE);
    gtk_widget_show(bbbm->file_statusbar);
    gtk_box_pack_start(GTK_BOX(hbox), bbbm->file_statusbar, TRUE, TRUE, 0);
    bbbm->image_statusbar = gtk_statusbar_new();
    gtk_widget_show(bbbm->image_statusbar);
    gtk_box_pack_start(GTK_BOX(hbox), bbbm->image_statusbar, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(bbbm->window), vbox);
    gtk_widget_show(bbbm->window);

    if (file)
    {
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
            bbbm_open_collection(bbbm, file);
        else
            fprintf(stderr, "bbbm: could not find '%s'\n", file);
    }
    return bbbm;
}

void bbbm_destroy(BBBM *bbbm)
{
    // bbbm_close should already been called when exiting the window,
    // just like destroying the window.
    // bbbm_close removes bbbm->images and bbbm->filename
    g_free(bbbm);
    // bbbm->opts is the only remaining thing, but that is created outside
    // the constructor; let it be destroyed there
}

void bbbm_statusbar_clear(BBBM *bbbm)
{
    guint context_id = gtk_statusbar_get_context_id(
                          GTK_STATUSBAR(bbbm->image_statusbar), IMAGE_CONTEXT);
    gtk_statusbar_pop(GTK_STATUSBAR(bbbm->image_statusbar), context_id);
}

void bbbm_reorder(BBBM *bbbm, guint index)
{
    GList *iter;
    guint num_imgs = g_list_length(bbbm->images);
    guint i;
    guint cols, rows;
    iter = g_list_nth(bbbm->images, index);
    for (i = index; iter; iter = iter->next, ++i)
    {
        guint c, r;
        gtk_container_remove(GTK_CONTAINER(bbbm->table),
                             BBBM_WIDGET(iter->data));
        c = i % bbbm->opts->thumb_cols;
        r = i / bbbm->opts->thumb_cols;
        gtk_table_attach(GTK_TABLE(bbbm->table), BBBM_WIDGET(iter->data),
                         c, c + 1, r, r + 1, 0, 0,
                         bbbm->padding, bbbm->padding);
    }
    cols = (bbbm->opts->thumb_cols < num_imgs ? bbbm->opts->thumb_cols :
                                                num_imgs);
    rows = num_imgs / bbbm->opts->thumb_cols +
           (num_imgs % bbbm->opts->thumb_cols != 0);
    gtk_table_resize(GTK_TABLE(bbbm->table),
                     (rows > 1 ? rows : 1), (cols > 1 ? cols : 1));
}

void bbbm_resize_thumbs(BBBM *bbbm)
{
    GList *iter;
    GList *old_imgs = bbbm->images;
    bbbm->images = NULL;
    for (iter = old_imgs; iter; iter = iter->next)
    {
        // steps to take: add new, destroy (removes from table)
        bbbm_add_image0(bbbm, BBBM_IMAGE(iter->data)->filename,
                        BBBM_IMAGE(iter->data)->description, -1);
        bbbm_image_destroy(BBBM_IMAGE(iter->data));
    }
    g_list_free(old_imgs);
}

static gint bbbm_save_to(BBBM *bbbm, const gchar *file)
{
    GList *iter;
    FILE *f = fopen(file, "w");
    if (!f)
        return 1;
    for (iter = bbbm->images; iter; iter = iter->next)
        fprintf(f, "%s\n%s\n", BBBM_IMAGE(iter->data)->filename,
                               BBBM_IMAGE(iter->data)->description);
    fclose(f);
    // change filename
    if (!bbbm->filename || strcmp(bbbm->filename, file))
    {
        guint context_id = gtk_statusbar_get_context_id(
                            GTK_STATUSBAR(bbbm->file_statusbar), FILE_CONTEXT);
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_statusbar), context_id);
        g_free(bbbm->filename); // freeing NULL is ok
        bbbm->filename = g_strdup(file);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_statusbar), context_id,
                           bbbm->filename);
    }
    // else bbbm->filename == file
    return 0;
}

static gint bbbm_create_list(BBBM *bbbm, const gchar *file)
{
    GList *iter;
    FILE *f = fopen(file, "w");
    if (!f)
        return 1;
    for (iter = bbbm->images; iter; iter = iter->next)
        fprintf(f, "%s\n", BBBM_IMAGE(iter->data)->filename);
    fclose(f);
    return 0;
}

static gint bbbm_create_menu(BBBM *bbbm, const gchar *file)
{
    GList *iter;
    FILE *f = fopen(file, "w");
    if (!f)
        return 1;
    fprintf(f, "[submenu] (Backgrounds)\n");
    for (iter = bbbm->images; iter; iter = iter->next)
        fprintf(f, "  [exec] (%s) {%s %s}\n",
                BBBM_IMAGE(iter->data)->description,
                bbbm->opts->set_command,
                BBBM_IMAGE(iter->data)->filename);
    fclose(f);
    return 0;
}

static inline gboolean bbbm_ask_overwrite(BBBM *bbbm, const gchar *file)
{
    gboolean result;
    gchar *text = g_strconcat("File '", file, "' exists. Overwrite?", NULL);
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(bbbm->window), 0,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_YES_NO, text);
    gtk_window_set_title(GTK_WINDOW(dialog), "Overwrite?");
    result = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES;
    gtk_widget_destroy(dialog);
    g_free(text);
    return result;
}

static inline void bbbm_error_message(BBBM *bbbm, const gchar *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(bbbm->window), 0,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK, message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static inline gboolean bbbm_is_image(const gchar *file)
{
    const gchar *ext = extension(file);
    guint i;
    if (!ext)
        // ext is NULL, so no extensions
        return FALSE;
    for (i = 0; EXTENSIONS[i]; ++i)
        if (!strcmp(ext, EXTENSIONS[i]))
            return TRUE;
    return FALSE;
}

static inline GtkWidget *create_chooser(BBBM *bbbm, const gchar *title,
                                        gboolean mult, gboolean hide_op)
{
    GtkWidget *chooser;

    bbbm_statusbar_clear(bbbm);
    chooser = gtk_file_selection_new(title);
    gtk_window_set_transient_for(GTK_WINDOW(chooser),
                                 GTK_WINDOW(bbbm->window));
    gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(chooser), mult);
    if (hide_op)
        gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(chooser));
    return chooser;
}

static inline void bbbm_execute(const gchar *command, gchar *file)
{
    if (!strcmp(command, ""))
        return;
    if (!fork())
    {
        // use X as a file to make sure it does not get split, replace later
        gchar *com = g_strconcat(command, " X", NULL);
        gchar **cmd = g_strsplit(com, " ", -1);
        guint i;
        g_free(com);
        for (i = 0; cmd[i]; ++i)
            ;
        // cmd[i] == NULL, so cmd[i - 1] should be X
        cmd[i - 1] = file;
        execvp(cmd[0], cmd);
    }
}

static GtkWidget *bbbm_create_menu_bar(BBBM *bbbm)
{
    static guint num_items = 19;
    static GtkItemFactoryEntry menu_items[] =
    {
        {"/_File", NULL, NULL, 0, "<Branch>"},
        {"/File/_Open...", "<ctrl>O", bbbm_open, 0, NULL},
        {"/File/_Save", "<ctrl>S", bbbm_save, 0, NULL},
        {"/File/Save _As...", "<ctrl><shift>S", bbbm_create, BBBM_SAVE, NULL},
        {"/File/_Close", "<ctrl>X", bbbm_close, 0, NULL},
        {"/File/sep", NULL, NULL, 0, "<Separator>"},
        {"/File/E_xit", "<alt>F4", bbbm_exit, 0, NULL},
        {"/_Edit", NULL, NULL, 0, "<Branch>"},
        {"/Edit/_Add Image...", "<ctrl>A", bbbm_add_image, 0, NULL},
        {"/Edit/Add _Directory...", "<ctrl>D", bbbm_add_directory, 0, NULL},
        {"/Edit/Add _Collection...", "<ctrl>C", bbbm_add_collection, 0, NULL},
        {"/_Tools", NULL, NULL, 0, "<Branch>"},
        {"/Tools/Create _List...", "<ctrl>L", bbbm_create, BBBM_LIST, NULL},
        {"/Tools/Create _Menu...", "<ctrl>M", bbbm_create, BBBM_MENU, NULL},
        {"/Tools/_Random Background", "<ctrl>R",
         bbbm_random_background, 0, NULL},
        {"/Tools/sep", NULL, NULL, 0, "<Separator>"},
        {"/Tools/_Options...", "<alt>P", bbbm_options_dialog, 0, NULL},
        {"/_Help", NULL, NULL, 0, "<Branch>"},
        {"/Help/_About", NULL, bbbm_about_dialog, 0, NULL}
    };
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    GtkItemFactory *item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
                                                        "<main>", accel_group);
    gtk_item_factory_create_items(item_factory, num_items, menu_items, bbbm);
    gtk_window_add_accel_group(GTK_WINDOW(bbbm->window), accel_group);
    return gtk_item_factory_get_widget(item_factory, "<main>");
}

static void bbbm_set_status(GtkWidget *widget, GdkEvent *event,
                            BBBMImage *image)
{
    guint context_id = gtk_statusbar_get_context_id(
                   GTK_STATUSBAR(image->bbbm->image_statusbar), IMAGE_CONTEXT);
    gtk_statusbar_push(GTK_STATUSBAR(image->bbbm->image_statusbar), context_id,
                                     image->description);
}

static void bbbm_clear_status(GtkWidget *widget, GdkEvent *event, BBBM *bbbm)
{
    bbbm_statusbar_clear(bbbm);
}

static void bbbm_open(BBBM *bbbm)
{
    GtkWidget *chooser = create_chooser(bbbm, "Open a collection",
                                        FALSE, TRUE);
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar file[PATH_MAX];
        strncpy(file,
                gtk_file_selection_get_filename(GTK_FILE_SELECTION(chooser)),
                PATH_MAX);
        file[PATH_MAX - 1] = 0;
        if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
        {
            bbbm_open_collection(bbbm, file);
            break;
        }
        else
        {
            gchar dir[PATH_MAX];
            if (g_file_test(file, G_FILE_TEST_IS_DIR))
                g_stpcpy(g_stpcpy(dir, file), "/");
            else if (!g_file_test(file, G_FILE_TEST_EXISTS))
            {
                dirname(file, dir);
                strcat(dir, "/");
            }
            gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
        }
    }
    gtk_widget_destroy(chooser);    
}

static void bbbm_open_collection(BBBM *bbbm, const gchar *file)
{
    guint context_id = gtk_statusbar_get_context_id(
                            GTK_STATUSBAR(bbbm->file_statusbar), FILE_CONTEXT);
    // first close the current file, then set filename and add new
    bbbm_close(bbbm);
    bbbm->filename = g_strdup(file);
    gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_statusbar), context_id,
                       bbbm->filename);
    bbbm_add_collection0(bbbm, file);
}

static void bbbm_save(BBBM *bbbm)
{
    if (bbbm->filename)
    {
        if (bbbm_save_to(bbbm, bbbm->filename))
        {
            gchar *message = g_strconcat("Could not save to '", bbbm->filename,
                                         "'", NULL);
            bbbm_error_message(bbbm, message);
            fprintf(stderr, "bbbm: could save to '%s'\n", bbbm->filename);
            g_free(message);
        }
    }
    else
        bbbm_create(bbbm, BBBM_SAVE);
}

static void bbbm_close(BBBM *bbbm)
{
    GList *iter;
    guint context_id = gtk_statusbar_get_context_id(
                            GTK_STATUSBAR(bbbm->file_statusbar), FILE_CONTEXT);
    for (iter = bbbm->images; iter; iter = iter->next)
        bbbm_image_destroy(BBBM_IMAGE(iter->data));
    g_list_free(bbbm->images);
    bbbm->images = NULL;
    g_free(bbbm->filename);
    bbbm->filename = NULL;
    gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_statusbar), context_id);
    bbbm_statusbar_clear(bbbm);
}

static void bbbm_exit_window(GtkWidget *widget, GdkEvent *event, BBBM *bbbm)
{
    bbbm_exit(bbbm);
}

static void bbbm_exit(BBBM *bbbm)
{
    bbbm_close(bbbm);
    gtk_widget_destroy(bbbm->window);
    gtk_main_quit();
}

static void bbbm_add_image(BBBM *bbbm)
{
    GtkWidget *chooser = create_chooser(bbbm, "Add images", TRUE, TRUE);
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        guint i;
        gchar **files = gtk_file_selection_get_selections(
                                                  GTK_FILE_SELECTION(chooser));
        if (!files || !files[0] ||
            !g_file_test(files[0], G_FILE_TEST_IS_REGULAR))
            // either no files, or a directory, or non-existing
        {
            if (files && files[0])
            {
                gchar dir[PATH_MAX];
                if (g_file_test(files[0], G_FILE_TEST_IS_DIR))
                    g_stpcpy(g_stpcpy(dir, files[0]), "/");
                else if (!g_file_test(files[0], G_FILE_TEST_EXISTS))
                {
                    dirname(files[0], dir);
                    strcat(dir, "/");
                }
                gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser),
                                                dir);
            }
            g_strfreev(files);
            continue;
        }
        for (i = 0; files[i]; ++i)
        {
            if (g_file_test(files[i], G_FILE_TEST_IS_REGULAR) &&
                bbbm_is_image(files[i]))
                bbbm_add_image0(bbbm, files[i], NULL, -1);
        }
        g_strfreev(files);
        break;
    }
    gtk_widget_destroy(chooser);
}

static void bbbm_add_image0(BBBM *bbbm, gchar *filename, gchar *description,
                            gint index)
{
    gchar thumb[PATH_MAX], thumb_dir[2 * PATH_MAX];
    pid_t pid;

    if (!description)
        description = filename;
    g_stpcpy(g_stpcpy(g_stpcpy(thumb, bbbm->thumbsdir), "/"), filename);
    dirname(thumb, thumb_dir);
    // makedir will also create bbbm->thumbsdir if it didn't exist yet
    makedirs(thumb_dir);

    if ((pid = fork()) == 0)
        execlp("convert", "convert", "-size", bbbm->opts->thumb_size,
               "-resize", bbbm->opts->thumb_size, filename, thumb, NULL);
    else if (pid > 0)
    {
        BBBMImage *image;
        guint col, row;
        waitpid(pid, NULL, 0);
        image = bbbm_image_new(bbbm, filename, description, thumb);
        gtk_signal_connect(GTK_OBJECT(BBBM_WIDGET(image)),
                           "button-press-event", 
                           GTK_SIGNAL_FUNC(bbbm_set_image), image);
        gtk_signal_connect(GTK_OBJECT(BBBM_WIDGET(image)),
                           "button-release-event",
                           GTK_SIGNAL_FUNC(bbbm_popup), image);
        gtk_signal_connect(GTK_OBJECT(BBBM_WIDGET(image)),
                           "enter-notify-event",
                           GTK_SIGNAL_FUNC(bbbm_set_status), image);
        gtk_signal_connect(GTK_OBJECT(BBBM_WIDGET(image)),
                           "leave-notify-event",
                           GTK_SIGNAL_FUNC(bbbm_clear_status), image->bbbm);
        gtk_widget_show(BBBM_WIDGET(image));
        // we might remove it from the table, so increase refcount
        g_object_ref(BBBM_WIDGET(image));
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
            bbbm_reorder(bbbm, index + 1);
        }
        gtk_table_attach(GTK_TABLE(bbbm->table), BBBM_WIDGET(image),
                         col, col + 1, row, row + 1, 0, 0, 
                         bbbm->padding, bbbm->padding);
    }
    else
    {
        bbbm_error_message(bbbm, "Could not create thumb");
    }
}

static void bbbm_add_directory(BBBM *bbbm)
{
    GtkWidget *chooser = create_chooser(bbbm, "Add a directory", FALSE, TRUE);
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar file[PATH_MAX];
        strncpy(file,
                gtk_file_selection_get_filename(GTK_FILE_SELECTION(chooser)),
                PATH_MAX);
        file[PATH_MAX - 1] = 0;
        if (!g_file_test(file, G_FILE_TEST_IS_DIR))
        {
            gchar dir[PATH_MAX];
            dirname(file, dir);
            strcat(dir, "/");
            gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
            continue;
        }
        if (g_file_test(file, G_FILE_TEST_EXISTS))
        {
            GSList *iter;
            GSList *files = listdir(file);
            for (iter = files; iter; iter = iter->next)
            {
                gchar *file = (gchar *)iter->data;
                if (bbbm_is_image(file))
                    bbbm_add_image0(bbbm, file, file, -1);
                // the string is copied inside bbbm_add_image0, so free it
                g_free(file);
            }
            g_slist_free(files);
            break;
        }
    }
    gtk_widget_destroy(chooser);
}

static void bbbm_add_collection(BBBM *bbbm)
{
    GtkWidget *chooser = create_chooser(bbbm, "Add collections", TRUE, TRUE);
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        guint i;
        gchar **files = gtk_file_selection_get_selections(
                                                  GTK_FILE_SELECTION(chooser));
        if (!files || !files[0] ||
            !g_file_test(files[0], G_FILE_TEST_IS_REGULAR))
            // either no files, or a directory, or non-existing
        {
            if (files && files[0])
            {
                gchar dir[PATH_MAX];
                if (g_file_test(files[0], G_FILE_TEST_IS_DIR))
                    g_stpcpy(g_stpcpy(dir, files[0]), "/");
                else if (!g_file_test(files[0], G_FILE_TEST_EXISTS))
                {
                    dirname(files[0], dir);
                    strcat(dir, "/");
                }
                gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser),
                                                dir);
            }
            continue;
        }
        for (i = 0; files[i]; ++i)
        {
            if (g_file_test(files[i], G_FILE_TEST_IS_REGULAR))
                bbbm_add_collection0(bbbm, files[i]);
        }
        g_strfreev(files);
        break;
    }
    gtk_widget_destroy(chooser);
}

static void bbbm_add_collection0(BBBM *bbbm, const gchar *file)
{
    gchar filename[PATH_MAX], description[PATH_MAX];
    FILE *f = fopen(file, "r");
    if (!f)
        return;
    while (fgets(filename, PATH_MAX, f))
    {
        filename[strlen(filename) - 1] = 0;
        if (!fgets(description, PATH_MAX, f))
            strcpy(description, filename);
        else
            description[strlen(description) - 1] = 0;
        if (g_file_test(filename, G_FILE_TEST_IS_REGULAR) &&
            bbbm_is_image(filename))
            bbbm_add_image0(bbbm, filename, description, -1);
    }
    fclose(f);
}

static void bbbm_create(BBBM *bbbm, gint type)
{
    create_func func;
    GtkWidget *chooser = NULL;
    switch (type)
    {
        case BBBM_SAVE:
            chooser = create_chooser(bbbm, "Save collection as",
                                     FALSE, FALSE);
            func = bbbm_save_to;
            break;
        case BBBM_LIST:
            chooser = create_chooser(bbbm, "Create background list",
                                     FALSE, FALSE);
            func = bbbm_create_list;
            break;
        case BBBM_MENU:
            chooser = create_chooser(bbbm, "Create background menu",
                                     FALSE, FALSE);
            func = bbbm_create_menu;
            break;
        default:
            return;
    }
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        gchar file[PATH_MAX];
        strncpy(file,
                gtk_file_selection_get_filename(GTK_FILE_SELECTION(chooser)),
                PATH_MAX);
        file[PATH_MAX - 1] = 0;
        if (!g_file_test(file, G_FILE_TEST_EXISTS) ||
            (g_file_test(file, G_FILE_TEST_IS_REGULAR) &&
             bbbm_ask_overwrite(bbbm, file)))
        {
            if (func(bbbm, file))
            {
                gchar *message = g_strconcat("Could not save to '", file, "'",
                                             NULL);
                bbbm_error_message(bbbm, message);
                g_free(message);
            }
            else
                break;
        }
        else
        {
            // is a dir or could not overwrite; set path
            gchar dir[PATH_MAX];
            if (g_file_test(file, G_FILE_TEST_IS_DIR))
                g_stpcpy(g_stpcpy(dir, file), "/");
            else
            {
                dirname(file, dir);
                strcat(dir, "/");
            }
            gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser), dir);
        }
    }
    gtk_widget_destroy(chooser);
}

static void bbbm_random_background(BBBM *bbbm)
{
    if (bbbm->images)
    {
        GRand *rand = g_rand_new();
        guint32 index = g_rand_int_range(rand, 0, g_list_length(bbbm->images));
        BBBMImage *image = BBBM_IMAGE(g_list_nth_data(bbbm->images, index));
        bbbm_set(image);
        g_rand_free(rand);
    }
}

static void bbbm_set_image(GtkWidget *widget, GdkEvent *event, BBBMImage *image)
{
    if (event->type == GDK_2BUTTON_PRESS)
        bbbm_set(image);
}

static void bbbm_popup(GtkWidget *widget, GdkEventButton *event,
                       BBBMImage *image)
{
    if (event->type == GDK_BUTTON_RELEASE && event->button == 3)
    {
        GtkWidget *popup;
        gint index = g_list_index(image->bbbm->images, image);
        guint num_items = 9;
        GtkItemFactoryEntry menu_items[] =
        {
            {"/_Set", NULL, bbbm_set, 0, NULL},
            {"/_View", NULL, bbbm_view, 0, NULL},
            {"/sep", NULL, NULL, 0, "<Separator>"},
            {"/Move _Back...", NULL, bbbm_move_back, index, NULL},
            {"/Move _Forward...", NULL, bbbm_move_forward, index, NULL},
            {"/sep", NULL, NULL, 0, "<Separator>"},
            {"/_Edit Description...", NULL, bbbm_edit_description_dialog,
             0, NULL},
            {"/_Insert...", NULL, bbbm_insert, 0, NULL},
            {"/_Delete", NULL, bbbm_delete, 0, NULL}
        };
        GtkItemFactory *item_factory = gtk_item_factory_new(GTK_TYPE_MENU,
                                                            "<popup>", NULL);
        gtk_item_factory_create_items(item_factory, num_items, menu_items,
                                      image);
        popup = gtk_item_factory_get_widget(item_factory, "<popup>");
        if (index == 0)
            gtk_widget_set_sensitive(
              gtk_item_factory_get_item(item_factory, "/Move Back..."), FALSE);
        if (index == g_list_length(image->bbbm->images) - 1)
            gtk_widget_set_sensitive(
           gtk_item_factory_get_item(item_factory, "/Move Forward..."), FALSE);
        gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL,
                       event->button, event->time);
    }
}

static void bbbm_set(BBBMImage *image)
{
    bbbm_execute(image->bbbm->opts->set_command, image->filename);
}

static void bbbm_view(BBBMImage *image)
{
    bbbm_execute(image->bbbm->opts->view_command, image->filename);
}

static void bbbm_move_back(BBBMImage *image, gint index)
{
    bbbm_move_dialog(image, index, FALSE);
}

static void bbbm_move_forward(BBBMImage *image, gint index)
{
    bbbm_move_dialog(image, index, TRUE);
}

static void bbbm_insert(BBBMImage *image)
{
    GtkWidget *chooser = create_chooser(image->bbbm, "Insert images",
                                        TRUE, TRUE);
    while (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    {
        guint i;
        gint index;
        gchar **files = gtk_file_selection_get_selections(
                                                  GTK_FILE_SELECTION(chooser));
        if (!files || !files[0] ||
            !g_file_test(files[0], G_FILE_TEST_IS_REGULAR))
            // either no files, or a directory, or non-existing
        {
            if (files && files[0])
            {
                gchar dir[PATH_MAX];
                if (g_file_test(files[0], G_FILE_TEST_IS_DIR))
                    g_stpcpy(g_stpcpy(dir, files[0]), "/");
                else if (!g_file_test(files[0], G_FILE_TEST_EXISTS))
                {
                    dirname(files[0], dir);
                    strcat(dir, "/");
                }
                gtk_file_selection_set_filename(GTK_FILE_SELECTION(chooser),
                                                dir);
            }
            continue;
        }
        index = g_list_index(image->bbbm->images, image);
        for (i = 0; files[i]; i++)
        {
            if (g_file_test(files[i], G_FILE_TEST_IS_REGULAR) &&
                bbbm_is_image(files[i]))
                bbbm_add_image0(image->bbbm, files[i], NULL, index++);
        }
        g_strfreev(files);
        break;
    }
    gtk_widget_destroy(chooser);
}

static void bbbm_delete(BBBMImage *image)
{
    gint index = g_list_index(image->bbbm->images, image);
    // we're going to destroy image, so keep a copy of its BBBM
    BBBM *bbbm = image->bbbm;
    bbbm->images = g_list_remove(bbbm->images, image);
    bbbm_image_destroy(image);
    if (index != g_list_length(bbbm->images))
        // dit not remove the last one
        bbbm_reorder(bbbm, index);
}
