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
#include "config.h"
#include "bbbm.h"
#include "command.h"
#include "image.h"
#include "command_item.h"
#include "dialogs.h"
#include "options.h"
#include "util.h"
#include "compat.h"

#define PADDING  5

/* create utility functions */
static inline GtkWidget *bbbm_create_menubar(BBBM *bbbm);

/* window callbacks */
static gboolean bbbm_delete_window(GtkWidget *widget, GdkEvent *event, BBBM *bbbm);

/* menu callbacks */
static void bbbm_menu_file_open(BBBM *bbbm);
static void bbbm_menu_file_save(BBBM *bbbm);
static void bbbm_menu_file_save_as(BBBM *bbbm);
static void bbbm_menu_file_close(BBBM *bbbm);
static void bbbm_menu_file_exit(BBBM *bbbm);
static void bbbm_menu_edit_add_images(BBBM *bbbm);
static void bbbm_menu_edit_add_directory(BBBM *bbbm);
static void bbbm_menu_edit_add_collections(BBBM *bbbm);
static void bbbm_menu_edit_add_image_lists(BBBM *bbbm);
static void bbbm_menu_edit_sort_on_filename(BBBM *bbbm);
static void bbbm_menu_edit_sort_on_description(BBBM *bbbm);
static void bbbm_menu_tools_create_list(BBBM *bbbm);
static void bbbm_menu_tools_create_menu(BBBM *bbbm);
static void bbbm_menu_tools_random_background(BBBM *bbbm);
static void bbbm_menu_tools_options(BBBM *bbbm);
static void bbbm_menu_help_about(BBBM *bbbm);

/* image callbacks */
static gboolean bbbm_image_mouse_press(GtkWidget *widget, GdkEventButton *event, BBBMImage *image);
static gboolean bbbm_image_mouse_release(GtkWidget *widget, GdkEventButton *event, BBBMImage *image);
static gboolean bbbm_image_mouse_enter(GtkWidget *widget, GdkEventCrossing *event, BBBMImage *image);
static gboolean bbbm_image_mouse_leave(GtkWidget *widget, GdkEventCrossing *event, BBBMImage *image);

/* image popup callbacks */
static void bbbm_image_popup_set(BBBMImage *image);
static void bbbm_image_popup_execute_command(BBBMCommandItem *item);
static void bbbm_image_popup_move_back(BBBMImage *image);
static void bbbm_image_popup_move_forward(BBBMImage *image);
static void bbbm_image_popup_edit_description(BBBMImage *image);
static void bbbm_image_popup_insert_images(BBBMImage *image);
static void bbbm_image_popup_delete(BBBMImage *image);

/* utility functions */
static void bbbm_update_item_enabled_states(BBBM *bbbm);
static void bbbm_set_modified(BBBM *bbbm, gboolean modified);

static gboolean bbbm_can_close(BBBM *bbbm);

static gboolean bbbm_open_collection(BBBM *bbbm, const gchar *filename);
static gboolean bbbm_save_collection(BBBM *bbbm, const gchar *filename);
static void bbbm_close_collection(BBBM *bbbm);
static gboolean bbbm_add_image(BBBM *bbbm, const gchar *filename, const gchar *description, gint index);
static gboolean bbbm_add_collection(BBBM *bbbm, const gchar *filename);
static gboolean bbbm_add_image_list(BBBM *bbbm, const gchar *filename);
static gboolean bbbm_create_list(BBBM *bbbm, const gchar *filename);
static gboolean bbbm_create_menu(BBBM *bbbm, const gchar *filename);
static inline void bbbm_write_string(FILE *file, const gchar *string);

/* image utility functions */
static inline void bbbm_attach_image(GtkTable *table, GtkWidget *image, guint x, guint y);
static void bbbm_reset_images(BBBM *bbbm, guint index);
static inline void bbbm_resize_thumbs(BBBM *bbbm);


BBBM *bbbm_new(BBBMOptions *options, const gchar *config_file, const gchar *collection_file) {
    GtkWidget *vbox, *hbox, *menubar, *scrolled_window;

    /* create and initialize the new BBBM object */
    BBBM *bbbm = g_malloc(sizeof(BBBM));
    bbbm->options     = options;
    bbbm->config_file = config_file;
    bbbm->filename    = NULL;
    bbbm->modified    = FALSE;
    bbbm->images      = NULL;

    /* the window */
    bbbm->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(bbbm->window), 640, 480);
    gtk_window_set_title(GTK_WINDOW(bbbm->window), PACKAGE_STRING);
    g_signal_connect(G_OBJECT(bbbm->window), "delete-event", G_CALLBACK(bbbm_delete_window), bbbm);

    /* a vertical box with menu bar, image area and status bar */
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
    gtk_container_add(GTK_CONTAINER(bbbm->window), vbox);

    /* the menu bar */
    menubar = bbbm_create_menubar(bbbm);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    /* the image area: a table in a scrolled window */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    bbbm->table = gtk_table_new(1, 1, TRUE);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), bbbm->table);

    /* the status bar: file info + image info, packed in a horizontal box */
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    /* the file info, with a non-modified and a modified context id */
    bbbm->file_bar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(bbbm->file_bar), FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), bbbm->file_bar, TRUE, TRUE, 0);
    bbbm->file_cid = gtk_statusbar_get_context_id(GTK_STATUSBAR(bbbm->file_bar), "Filename");
    gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid, "Untitled");
    bbbm->file_mod_cid = gtk_statusbar_get_context_id(GTK_STATUSBAR(bbbm->file_bar), "FilenameModified");

    /* the image info */
    bbbm->image_bar = gtk_statusbar_new();
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(bbbm->image_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), bbbm->image_bar, TRUE, TRUE, 0);
    bbbm->image_cid = gtk_statusbar_get_context_id(GTK_STATUSBAR(bbbm->image_bar), "Image");

    gtk_widget_show_all(bbbm->window);

    bbbm_update_item_enabled_states(bbbm);

    /* attempt to open the given file (if any) */
    if (collection_file != NULL) {
        if (!g_file_test(collection_file, G_FILE_TEST_IS_REGULAR)) {
            bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not open '%s'", collection_file);
        } else {
            gchar *absolute_file = bbbm_util_absolute_path(collection_file);
            bbbm_open_collection(bbbm, absolute_file);
            g_free(absolute_file);
        }
    }
    return bbbm;
}

static inline GtkWidget *bbbm_create_menubar(BBBM *bbbm) {
    static guint n_items = 23;
    static GtkItemFactoryEntry items[] = {
        {"/_File",                     NULL,             NULL,                               0, "<Branch>"},
        {"/File/_Open...",             "<ctrl>O",        bbbm_menu_file_open,                0, NULL},
        {"/File/_Save",                "<ctrl>S",        bbbm_menu_file_save,                0, NULL},
        {"/File/Save _As...",          "<ctrl><shift>S", bbbm_menu_file_save_as,             0, NULL},
        {"/File/_Close",               "<ctrl>X",        bbbm_menu_file_close,               0, NULL},
        {"/File/sep",                  NULL,             NULL,                               0, "<Separator>"},
        {"/File/E_xit",                "<alt>F4",        bbbm_menu_file_exit,                0, NULL},
        {"/_Edit",                     NULL,             NULL,                               0, "<Branch>"},
        {"/Edit/_Add Images...",       "<ctrl>A",        bbbm_menu_edit_add_images,          0, NULL},
        {"/Edit/Add _Directory...",    "<ctrl>D",        bbbm_menu_edit_add_directory,       0, NULL},
        {"/Edit/Add _Collections...",  "<ctrl>C",        bbbm_menu_edit_add_collections,     0, NULL},
        {"/Edit/Add _Image Lists...",  "<ctrl>I",        bbbm_menu_edit_add_image_lists,     0, NULL},
        {"/Edit/sep",                  NULL,             NULL,                               0, "<Separator>"},
        {"/Edit/Sort On _Filename",    "<ctrl><shift>F", bbbm_menu_edit_sort_on_filename,    0, NULL},
        {"/Edit/Sort On D_escription", "<ctrl><shift>D", bbbm_menu_edit_sort_on_description, 0, NULL},
        {"/_Tools",                    NULL,             NULL,                               0, "<Branch>"},
        {"/Tools/Create _List...",     "<ctrl>L",        bbbm_menu_tools_create_list,        0, NULL},
        {"/Tools/Create _Menu...",     "<ctrl>M",        bbbm_menu_tools_create_menu,        0, NULL},
        {"/Tools/_Random Background", "<ctrl>R",         bbbm_menu_tools_random_background,  0, NULL},
        {"/Tools/sep",                 NULL,             NULL,                               0, "<Separator>"},
        {"/Tools/_Options...",         "<alt>P",         bbbm_menu_tools_options,            0, NULL},
        {"/_Help",                     NULL,             NULL,                               0, "<Branch>"},
        {"/Help/_About...",            NULL,             bbbm_menu_help_about,               0, NULL},
    };
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new();
    bbbm->factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
    gtk_item_factory_create_items(bbbm->factory, n_items, items, bbbm);
    gtk_window_add_accel_group(GTK_WINDOW(bbbm->window), accel_group);

    return gtk_item_factory_get_widget(bbbm->factory, "<main>");
}

void bbbm_destroy(BBBM *bbbm) {
    /* options and config_file are not owned by the instance, do not destroy them */
    g_free(bbbm->filename);
    g_list_foreach(bbbm->images, (GFunc) g_object_unref, NULL);
    g_list_free(bbbm->images);
    /* closing the window already destroyed the window, table and status bars */
    g_object_unref(bbbm->factory);
    g_free(bbbm);
}

static gboolean bbbm_delete_window(GtkWidget *widget, GdkEvent *event, BBBM *bbbm) {
    if (!bbbm_can_close(bbbm)) {
        /* block other handlers, like closing the window! */
        return TRUE;
    }
    bbbm_close_collection(bbbm);
    gtk_widget_destroy(bbbm->window);
    gtk_main_quit();
    return FALSE;
}

static void bbbm_menu_file_open(BBBM *bbbm) {
    if (bbbm_can_close(bbbm)) {
        gchar *filename;

        filename = bbbm_dialogs_get_file(GTK_WINDOW(bbbm->window), "Open a collection");
        if (filename != NULL) {
            bbbm_open_collection(bbbm, filename);
            g_free(filename);
        }
    }
}

static void bbbm_menu_file_save(BBBM *bbbm) {
    if (bbbm->filename != NULL && !bbbm_save_collection(bbbm, bbbm->filename)) {
        bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not save '%s'", bbbm->filename);
    } else if (bbbm->filename == NULL) {
        bbbm_menu_file_save_as(bbbm);
    }
}

static void bbbm_menu_file_save_as(BBBM *bbbm) {
    bbbm_dialogs_save(GTK_WINDOW(bbbm->window), "Save collection as", (bbbm_save_function) bbbm_save_collection, bbbm);
}

static void bbbm_menu_file_close(BBBM *bbbm) {
    if (bbbm_can_close(bbbm)) {
        bbbm_close_collection(bbbm);
    }
}

static void bbbm_menu_file_exit(BBBM *bbbm) {
    /* mimic closing the window from the title bar */
    g_signal_emit_by_name(G_OBJECT(bbbm->window), "delete-event", NULL, bbbm);
}

static void bbbm_menu_edit_add_images(BBBM *bbbm) {
    GList *files;

    files = bbbm_dialogs_get_files(GTK_WINDOW(bbbm->window), "Add images");
    while (files != NULL) {
        gchar *file;

        file = (gchar *) files->data;
        files = g_list_remove(files, file);
        if (bbbm_util_is_image(file) && !bbbm_add_image(bbbm, file, NULL, -1)) {
            bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not add image '%s'", file);
        }
        g_free(file);
    }
}

static void bbbm_menu_edit_add_directory(BBBM *bbbm) {
    GList *files;

    files = bbbm_dialogs_get_files_dir(GTK_WINDOW(bbbm->window), "Add a directory");
    while (files != NULL) {
        gchar *file;

        file = (gchar *) files->data;
        files = g_list_remove(files, file);
        if (bbbm_util_is_image(file) && !bbbm_add_image(bbbm, file, NULL, -1)) {
            bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not add image '%s'", file);
        }
        g_free(file);
    }
}

static void bbbm_menu_edit_add_collections(BBBM *bbbm) {
    GList *files;

    files = bbbm_dialogs_get_files(GTK_WINDOW(bbbm->window), "Add collections");
    while (files != NULL) {
        gchar *file;

        file = (gchar *) files->data;
        files = g_list_remove(files, file);
        if (!bbbm_add_collection(bbbm, file)) {
            bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not add collection '%s' properly", file);
        }
        g_free(file);
    }
}

static void bbbm_menu_edit_add_image_lists(BBBM *bbbm) {
    GList *files;

    files = bbbm_dialogs_get_files(GTK_WINDOW(bbbm->window), "Add image lists");
    while (files != NULL) {
        gchar *file;

        file = (gchar *) files->data;
        files = g_list_remove(files, file);
        if (!bbbm_add_image_list(bbbm, file)) {
            bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not add image list '%s' properly", file);
        }
        g_free(file);
    }
}

static void bbbm_menu_edit_sort_on_filename(BBBM *bbbm) {
    if (bbbm->images == NULL) {
        return;
    }
    bbbm->images = g_list_sort(bbbm->images, (GCompareFunc) bbbm_image_compare_filename);
    bbbm_reset_images(bbbm, 0);
    bbbm_set_modified(bbbm, TRUE);
}

static void bbbm_menu_edit_sort_on_description(BBBM *bbbm) {
    if (bbbm->images == NULL) {
        return;
    }
    bbbm->images = g_list_sort(bbbm->images, (GCompareFunc) bbbm_image_compare_description);
    bbbm_reset_images(bbbm, 0);
    bbbm_set_modified(bbbm, TRUE);
}

static void bbbm_menu_tools_create_list(BBBM *bbbm) {
    bbbm_dialogs_save(GTK_WINDOW(bbbm->window), "Create background list", (bbbm_save_function) bbbm_create_list, bbbm);
}

static void bbbm_menu_tools_create_menu(BBBM *bbbm) {
    bbbm_dialogs_save(GTK_WINDOW(bbbm->window), "Create background menu", (bbbm_save_function) bbbm_create_menu, bbbm);
}

static void bbbm_menu_tools_random_background(BBBM *bbbm) {
    if (bbbm->images != NULL) {
        GRand *rand;
        guint32 index;
        BBBMImage *image;

        rand = g_rand_new();
        index = g_rand_int_range(rand, 0, g_list_length(bbbm->images));
        image = BBBM_IMAGE(g_list_nth_data(bbbm->images, index));

        bbbm_util_execute(bbbm_options_get_set_command(bbbm->options),
                          bbbm_image_get_filename(image));

        g_rand_free(rand);
    }
}

static void bbbm_menu_tools_options(BBBM *bbbm) {
    guint changed;

    changed = bbbm_dialogs_options(GTK_WINDOW(bbbm->window), bbbm->options);
    if ((changed & OPTIONS_THUMB_SIZE_CHANGED) != 0) {
        bbbm_resize_thumbs(bbbm);
    }
    if ((changed & OPTIONS_THUMB_COLUMN_COUNT_CHANGED) != 0) {
        bbbm_reset_images(bbbm, 0);
    }
    if (changed != 0) {
        bbbm_options_write_to_file(bbbm->options, bbbm->config_file);
    }
}

static void bbbm_menu_help_about(BBBM *bbbm) {
    bbbm_dialogs_about(GTK_WINDOW(bbbm->window));
}

static gboolean bbbm_image_mouse_press(GtkWidget *widget, GdkEventButton *event, BBBMImage *image) {
    if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
        bbbm_util_execute(bbbm_options_get_set_command(image->bbbm->options),
                          bbbm_image_get_filename(image));
    }
    return FALSE;
}

static gboolean bbbm_image_mouse_release(GtkWidget *widget, GdkEventButton *event, BBBMImage *image) {
    if (event->type == GDK_BUTTON_RELEASE && event->button == 3) {
        static guint n_items = 8;
        static GtkItemFactoryEntry items[] = {
            {"/_Set",                 NULL, bbbm_image_popup_set,              0, NULL},
            {"/sep",                  NULL, NULL,                              0, "<Separator>"},
            {"/Move _Back...",        NULL, bbbm_image_popup_move_back,        0, NULL},
            {"/Move _Forward...",     NULL, bbbm_image_popup_move_forward,     0, NULL},
            {"/sep",                  NULL, NULL,                              0, "<Separator>"},
            {"/_Edit Description...", NULL, bbbm_image_popup_edit_description, 0, NULL},
            {"/_Insert Images..",     NULL, bbbm_image_popup_insert_images,    0, NULL},
            {"/_Delete",              NULL, bbbm_image_popup_delete,           0, NULL},
        };
        GtkWidget *popup;
        GtkWidget *item;
        GtkItemFactory *factory;
        const GList *iterator;
        gint index;

        factory = gtk_item_factory_new(GTK_TYPE_MENU, "<popup>", NULL);
        gtk_item_factory_create_items(factory, n_items, items, image);
        popup = gtk_item_factory_get_widget(factory, "<popup>");

        index = g_list_index(image->bbbm->images, image);
        if (index == 0) {
            gtk_widget_set_sensitive(gtk_item_factory_get_item(factory, "/Move Back..."), FALSE);
        }
        if (index == g_list_length(image->bbbm->images) - 1) {
            gtk_widget_set_sensitive(gtk_item_factory_get_item(factory, "/Move Forward..."), FALSE);
        }
        /* add the commands */
        index = 2;
        for (iterator = bbbm_options_get_commands(image->bbbm->options); iterator != NULL; iterator = iterator->next) {
            BBBMCommand *cmd;
            const gchar *command, *label;

            cmd     = (BBBMCommand *) iterator->data;
            command = bbbm_command_get_command(cmd);
            label   = bbbm_command_get_label(cmd);

            if (!bbbm_str_empty(command)) {
                if (bbbm_str_empty(label)) {
                    label = command;
                }
                item = bbbm_command_item_new_for_file(label, command, image->filename);
                g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(bbbm_image_popup_execute_command), NULL);
                gtk_menu_insert(GTK_MENU(popup), item, index);
                index++;
            }
        }
        if (index > 2) {
            /* added at least one command */
            item = gtk_separator_menu_item_new();
            gtk_menu_insert(GTK_MENU(popup), item, index);
        }
        gtk_widget_show_all(popup);
        gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL, event->button, event->time);
    }
    return FALSE;
}

static gboolean bbbm_image_mouse_enter(GtkWidget *widget, GdkEventCrossing *event, BBBMImage *image) {
    /* pop any existing image first */
    gtk_statusbar_pop(GTK_STATUSBAR(image->bbbm->image_bar), image->bbbm->image_cid);
    gtk_statusbar_push(GTK_STATUSBAR(image->bbbm->image_bar), image->bbbm->image_cid,
                       bbbm_image_get_description(image));
    return FALSE;
}

static gboolean bbbm_image_mouse_leave(GtkWidget *widget, GdkEventCrossing *event, BBBMImage *image) {
    /* pop twice to increase the chance the status bar gets actually closed */
    gtk_statusbar_pop(GTK_STATUSBAR(image->bbbm->image_bar), image->bbbm->image_cid);
    gtk_statusbar_pop(GTK_STATUSBAR(image->bbbm->image_bar), image->bbbm->image_cid);
    return FALSE;
}

static void bbbm_image_popup_set(BBBMImage *image) {
    bbbm_util_execute(bbbm_options_get_set_command(image->bbbm->options),
                      bbbm_image_get_filename(image));
}

static void bbbm_image_popup_execute_command(BBBMCommandItem *item) {
    const gchar *command;

    command = bbbm_command_item_get_command(item);
    bbbm_util_execute_cmd(command);
}

static void bbbm_image_popup_move_back(BBBMImage *image) {
    guint index, new_pos;
    gint move;

    index = g_list_index(image->bbbm->images, image);
    move = bbbm_dialogs_move(GTK_WINDOW(image->bbbm->window), "Move back", index);
    if (move == -1) {
        return;
    }
    new_pos = index - move;
    image->bbbm->images = g_list_remove(image->bbbm->images, image);
    image->bbbm->images = g_list_insert(image->bbbm->images, image, new_pos);
    bbbm_reset_images(image->bbbm, new_pos);
    bbbm_set_modified(image->bbbm, TRUE);
}

static void bbbm_image_popup_move_forward(BBBMImage *image) {
    guint index, new_pos, limit;
    gint move;

    index = g_list_index(image->bbbm->images, image);
    limit = g_list_length(image->bbbm->images) - index - 1;
    move = bbbm_dialogs_move(GTK_WINDOW(image->bbbm->window), "Move forward", limit);
    if (move == -1) {
        return;
    }
    new_pos = index + move;
    image->bbbm->images = g_list_remove(image->bbbm->images, image);
    image->bbbm->images = g_list_insert(image->bbbm->images, image, new_pos);
    bbbm_reset_images(image->bbbm, index);
    bbbm_set_modified(image->bbbm, TRUE);
}

static void bbbm_image_popup_edit_description(BBBMImage *image) {
    const gchar *old_description;
    gchar *new_description;

    old_description = bbbm_image_get_description(image);
    new_description = bbbm_dialogs_edit_description(GTK_WINDOW(image->bbbm->window), old_description);
    if (!bbbm_str_equals(new_description, old_description)) {
        /* Normally we would call bbbm_image_set_description.
           However, why do duplicate new_description only to free it? */
        bbbm_image_set_description_ref(image, new_description);
        bbbm_set_modified(image->bbbm, TRUE);
    } else {
        g_free(new_description);
    }
}

static void bbbm_image_popup_insert_images(BBBMImage *image) {
    guint index;
    GList *files;

    index = g_list_index(image->bbbm->images, image);
    files = bbbm_dialogs_get_files(GTK_WINDOW(image->bbbm->window), "Insert images");
    while (files != NULL) {
        gchar *file;

        file = (gchar *) files->data;
        files = g_list_remove(files, file);
        if (bbbm_util_is_image(file) && !bbbm_add_image(image->bbbm, file, NULL, index++)) {
            bbbm_dialogs_error(GTK_WINDOW(image->bbbm->window), "Could not add image '%s'", file);
            /* index got increased, decrease again */
            --index;
        }
        g_free(file);
    }
}

static void bbbm_image_popup_delete(BBBMImage *image) {
    BBBM *bbbm;

    /* cache the image's BBBM object, because once the image has been destroyed it's no longer available */
    bbbm = image->bbbm;
    if (bbbm_dialogs_question(GTK_WINDOW(bbbm->window), "Delete image?", "Delete image '%s'?", bbbm_image_get_description(image))) {
        guint index;

        index = g_list_index(bbbm->images, image);
        bbbm->images = g_list_remove(bbbm->images, image);
        gtk_widget_destroy(GTK_WIDGET(image));
        g_object_unref(image);
        if (index != g_list_length(bbbm->images)) {
            bbbm_reset_images(bbbm, index);
        }
        bbbm_set_modified(bbbm, TRUE);
        if (bbbm->images == NULL) {
            bbbm_update_item_enabled_states(bbbm);
        }
    }
}

static void bbbm_update_item_enabled_states(BBBM *bbbm) {
    GtkWidget *widget;
    gboolean has_filename;
    gboolean has_images;

    has_filename = bbbm->filename != NULL;
    has_images = bbbm->images != NULL;

    widget = gtk_item_factory_get_item(bbbm->factory, "/File/Save");
    gtk_widget_set_sensitive(widget, has_filename);
    widget = gtk_item_factory_get_item(bbbm->factory, "/File/Close");
    gtk_widget_set_sensitive(widget, has_filename || has_images);

    widget = gtk_item_factory_get_item(bbbm->factory, "/Edit/Sort On Filename");
    gtk_widget_set_sensitive(widget, has_images);
    widget = gtk_item_factory_get_item(bbbm->factory, "/Edit/Sort On Description");
    gtk_widget_set_sensitive(widget, has_images);

    widget = gtk_item_factory_get_item(bbbm->factory, "/Tools/Random Background");
    gtk_widget_set_sensitive(widget, has_images);
}

static void bbbm_set_modified(BBBM *bbbm, gboolean modified) {
    if (modified == bbbm->modified) {
        return;
    }
    bbbm->modified = modified;
    if (bbbm->modified && bbbm->filename != NULL) {
        gchar *filename = g_strconcat(bbbm->filename, "*", NULL);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_mod_cid, filename);
        g_free(filename);
    } else if (bbbm->modified) {
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_mod_cid, "Untitled*");
    } else {
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_mod_cid);
    }
}

static gboolean bbbm_can_close(BBBM *bbbm) {
    if (bbbm->modified) {
        return bbbm_dialogs_question(GTK_WINDOW(bbbm->window), "Close collection?", "Collection has been modified. Close anyway?");
    }
    return TRUE;
}

static gboolean bbbm_open_collection(BBBM *bbbm, const gchar *filename) {
    bbbm_close_collection(bbbm);
    if (bbbm_add_collection(bbbm, filename)) {
        bbbm_set_modified(bbbm, FALSE);
        bbbm->filename = g_strdup(filename);
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid, bbbm->filename);
        bbbm_update_item_enabled_states(bbbm);
        return TRUE;
    } else {
        bbbm_dialogs_error(GTK_WINDOW(bbbm->window), "Could not open '%s' properly", filename);
        /* some images might be added, remove them */
        bbbm_close_collection(bbbm);
        return FALSE;
    }
}

static gboolean bbbm_save_collection(BBBM *bbbm, const gchar *filename) {
    GList *iterator;
    FILE *file;

    /* first save, in case any error occurs */
    file = fopen(filename, "w");
    if (file == NULL) {
        return FALSE;
    }
    for (iterator = bbbm->images; iterator != NULL; iterator = iterator->next) {
        fprintf(file, "%s\n%s\n",
                bbbm_image_get_filename(BBBM_IMAGE(iterator->data)),
                bbbm_image_get_description(BBBM_IMAGE(iterator->data)));
    }
    fclose(file);

    bbbm_set_modified(bbbm, FALSE);
    if (!bbbm_str_equals(filename, bbbm->filename)) {
        gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid);
        g_free(bbbm->filename);
        bbbm->filename = bbbm_util_absolute_path(filename);
        gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid, bbbm->filename);
        bbbm_update_item_enabled_states(bbbm);
    }
    return TRUE;
}

static void bbbm_close_collection(BBBM *bbbm) {
    /* remove all images */
    while (bbbm->images != NULL) {
        GtkWidget *image = GTK_WIDGET(bbbm->images->data);
        bbbm->images = g_list_remove(bbbm->images, image);
        gtk_widget_destroy(image);
    }
    /* clear the filename */
    g_free(bbbm->filename);
    bbbm->filename = NULL;

    bbbm_set_modified(bbbm, FALSE);

    /* clear/set the status bars */
    gtk_statusbar_pop(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid);
    gtk_statusbar_push(GTK_STATUSBAR(bbbm->file_bar), bbbm->file_cid, "Untitled");
    gtk_statusbar_pop(GTK_STATUSBAR(bbbm->image_bar), bbbm->image_cid);

    bbbm_update_item_enabled_states(bbbm);
}

static gboolean bbbm_add_image(BBBM *bbbm, const gchar *filename, const gchar *description, gint index) {
    GtkWidget *image;
    guint thumb_width, thumb_height, thumb_column_count;
    guint col, row;

    if (bbbm_str_empty(description)) {
        description = filename;
    }

    thumb_width        = bbbm_options_get_thumb_width(bbbm->options);
    thumb_height       = bbbm_options_get_thumb_height(bbbm->options);
    thumb_column_count = bbbm_options_get_thumb_column_count(bbbm->options);
    image = bbbm_image_new(bbbm, filename, description, thumb_width, thumb_height);
    g_signal_connect(G_OBJECT(image), "button-press-event",   G_CALLBACK(bbbm_image_mouse_press),   image);
    g_signal_connect(G_OBJECT(image), "button-release-event", G_CALLBACK(bbbm_image_mouse_release), image);
    g_signal_connect(G_OBJECT(image), "enter-notify-event",   G_CALLBACK(bbbm_image_mouse_enter),   image);
    g_signal_connect(G_OBJECT(image), "leave-notify-event",   G_CALLBACK(bbbm_image_mouse_leave),   image);
    gtk_widget_show(image);
    g_object_ref(image);
    if (index == -1) {
        guint img_num = g_list_length(bbbm->images);
        col = img_num % thumb_column_count;
        row = img_num / thumb_column_count;
        bbbm->images = g_list_append(bbbm->images, image);
    } else {
        col = index % thumb_column_count;
        row = index / thumb_column_count;
        bbbm->images = g_list_insert(bbbm->images, image, index);
        bbbm_reset_images(bbbm, index + 1);
    }
    bbbm_attach_image(GTK_TABLE(bbbm->table), image, col, row);
    bbbm_set_modified(bbbm, TRUE);
    bbbm_update_item_enabled_states(bbbm);
    return TRUE;
}

static gboolean bbbm_add_collection(BBBM *bbbm, const gchar *filename) {
    gboolean result;
    gchar file_line[PATH_MAX], description_line[PATH_MAX];
    FILE *file;

    file = fopen(filename, "r");
    if (file == NULL) {
        return FALSE;
    }

    result = TRUE;
    while (fgets(file_line, PATH_MAX, file) != NULL) {
        g_strstrip(file_line);
        if (fgets(description_line, PATH_MAX, file) != NULL) {
            g_strstrip(description_line);
        } else {
            strcpy(description_line, file_line);
        }
        if (bbbm_util_is_image(file_line)) {
            result = result & bbbm_add_image(bbbm, file_line, description_line, -1);
        } else {
            result = FALSE;
        }
    }
    fclose(file);
    return result;
}

static gboolean bbbm_add_image_list(BBBM *bbbm, const gchar *filename) {
    gboolean result;
    gchar file_line[PATH_MAX];
    FILE *file;

    file = fopen(filename, "r");
    if (file == NULL) {
        return FALSE;
    }

    result = TRUE;
    while (fgets(file_line, PATH_MAX, file) != NULL) {
        g_strstrip(file_line);
        if (bbbm_util_is_image(file_line)) {
            result = result & bbbm_add_image(bbbm, file_line, file_line, -1);
        } else {
            result = FALSE;
        }
    }
    fclose(file);
    return result;
}

static gboolean bbbm_create_list(BBBM *bbbm, const gchar *filename) {
    GList *iterator;
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL) {
        return FALSE;
    }
    for (iterator = bbbm->images; iterator != NULL; iterator = iterator->next) {
        fprintf(file, "%s\n", bbbm_image_get_filename(BBBM_IMAGE(iterator->data)));
    }
    fclose(file);
    return TRUE;
}

static gboolean bbbm_create_menu(BBBM *bbbm, const gchar *filename) {
    gboolean filename_as_label, filename_as_title;
    GList *iterator;
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL) {
        return FALSE;
    }

    filename_as_label = bbbm_options_get_filename_as_label(bbbm->options);
    filename_as_title = bbbm_options_get_filename_as_title(bbbm->options);

    fprintf(file, "[submenu] (");
    if (bbbm->filename != NULL && (filename_as_label || filename_as_title)) {
        gchar *name;

        name = g_path_get_basename(bbbm->filename);
        if (filename_as_label) {
            bbbm_write_string(file, name);
            fprintf(file, ")");
        } else {
            fprintf(file, "Backgrounds)");
        }
        if (filename_as_title) {
            fprintf(file, " {");
            bbbm_write_string(file, name);
            fprintf(file, "}\n");
        } else {
            fprintf(file, "\n");
        }
        g_free(name);
    } else {
        fprintf(file, "Backgrounds)\n");
    }
    for (iterator = bbbm->images; iterator != NULL; iterator = iterator->next) {
        gchar *cmd;

        cmd = bbbm_util_get_command(bbbm_options_get_set_command(bbbm->options),
                                    bbbm_image_get_filename(BBBM_IMAGE(iterator->data)));
        fprintf(file, "  [exec] (");
        bbbm_write_string(file, bbbm_image_get_description(BBBM_IMAGE(iterator->data)));
        fprintf(file, ") {");
        bbbm_write_string(file, cmd);
        fprintf(file, "}\n");
        g_free(cmd);
    }
    fclose(file);
    return TRUE;
}

static inline void bbbm_write_string(FILE *file, const gchar *string) {
    while (*string != '\0') {
        switch (*string) {
            case '\\': /* fallthrough */
            case '(':  /* fallthrough */
            case ')':  /* fallthrough */
            case '{':  /* fallthrough */
            case '}':
                fputc('\\', file);
                /* fallthrough */
            default:
                fputc(*string++, file);
                break;
        }
    }
}

static inline void bbbm_attach_image(GtkTable *table, GtkWidget *image, guint x, guint y) {
    gtk_table_attach(table, image, x, x + 1, y, y + 1, 0, 0, PADDING, PADDING);
}

static void bbbm_reset_images(BBBM *bbbm, guint index) {
    GList *iterator;
    guint image_count, column_count;
    guint cols, rows;

    image_count = g_list_length(bbbm->images);
    column_count = bbbm_options_get_thumb_column_count(bbbm->options);
    cols = MIN(column_count, image_count);
    rows = image_count / column_count + (image_count % column_count == 0 ? 0 : 1);
    for (iterator = g_list_nth(bbbm->images, index) ; iterator != NULL; iterator = iterator->next, ++index) {
        guint c, r;

        c = index % column_count;
        r = index / column_count;
        iterator->data = g_object_ref(iterator->data);
        gtk_container_remove(GTK_CONTAINER(bbbm->table), GTK_WIDGET(iterator->data));
        bbbm_attach_image(GTK_TABLE(bbbm->table), GTK_WIDGET(iterator->data), c, r);
        g_object_unref(iterator->data);
    }
    gtk_table_resize(GTK_TABLE(bbbm->table), MAX(rows, 1), MAX(cols, 1));
}

static inline void bbbm_resize_thumbs(BBBM *bbbm) {
    GList *iterator;

    for (iterator = bbbm->images; iterator != NULL; iterator = iterator->next) {
        guint thumb_width  = bbbm_options_get_thumb_width(bbbm->options);
        guint thumb_height = bbbm_options_get_thumb_height(bbbm->options);
        bbbm_image_resize(BBBM_IMAGE(iterator->data), thumb_width, thumb_height);
    }
}
