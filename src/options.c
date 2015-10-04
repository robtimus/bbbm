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
#include "options.h"
#include "util.h"

#if HAVE_STRTOLD == 1
    #define BBBM_STRTOD(nptr, endptr)  strtold(nptr, endptr)
#else
    #define BBBM_STRTOD(nptr, endptr)  strtod(nptr, endptr)
#endif

#define BBBM_OPTIONS_GET_STRING(string)          string != NULL ? g_strdup(string->str) : NULL

#define BBBM_OPTIONS_DEFAULT_SET_COMMAND         "bsetbg"
#define BBBM_OPTIONS_DEFAULT_THUMB_WIDTH         128
#define BBBM_OPTIONS_DEFAULT_THUMB_HEIGHT        96
#define BBBM_OPTIONS_DEFAULT_THUMB_COLUMN_COUNT  4
#define BBBM_OPTIONS_DEFAULT_FILENAME_AS_LABEL   FALSE
#define BBBM_OPTIONS_DEFAULT_FILENAME_AS_TITLE   FALSE
#define BBBM_OPTIONS_DEFAULT_COMMAND_COUNT       10

#define BBBM_OPTIONS_READ_BUFFER_SIZE            8192

typedef struct {
    BBBMOptions *options;
    /* the stack of text data for the current element stack */
    GSList *text_stack;
    /* lists for commands and command labels */
    GSList *command_list;
    GSList *command_label_list;
    /* the command and command label for the current bbbm/commands/command element; reset each time */
    gchar *command;
    gchar *command_label;
    /* flags for found elements */
    gboolean found_thumbs;             /* bbbm/thumbs */
    gboolean found_thumb_size;         /* bbbm/thumbs/size */
    gboolean found_thumb_column_count; /* bbbm/thumbs/column-count */
    gboolean found_menu;               /* bbbm/menu */
    gboolean found_filename_as_label;  /* bbbm/menu/filename-as-label */
    gboolean found_filename_as_title;  /* bbbm/menu/filename-as-title */
    gboolean found_commands;           /* bbbm/commands */
    gboolean found_set_command;        /* bbbm/commands/set-command */
    /* the following two flags are reset for each bbbm/commands/command element */
    gboolean found_command;            /* bbbm/commands/command/command */
    gboolean found_command_label;      /* bbbm/commands/command/label */
} BBBMOptionsParseData;

/* general utility functions */
static inline void bbbm_options_close_file(FILE *file, const gchar *filename);
static inline void bbbm_options_write_element(FILE *file, const gchar *text, const gchar *value);

/* parser callback functions */
static void bbbm_options_parse_start_element(GMarkupParseContext *context,
                                             const gchar *element_name,
                                             const gchar **attribute_names, const gchar **attribute_values,
                                             gpointer user_data,
                                             GError **error);
static void bbbm_options_parse_end_element(GMarkupParseContext *context,
                                           const gchar *element_name,
                                           gpointer user_data,
                                           GError **error);
static void bbbm_options_parse_text(GMarkupParseContext *context,
                                    const gchar *text, gsize text_len,
                                    gpointer user_data,
                                    GError **error);

/* parser utility functions */
static gboolean bbbm_options_parse_check_empty_content(const gchar *element_name, GString *content,
                                                       GError **error);
static gboolean bbbm_options_parse_text_to_int(const gchar *element_name, GString *content,
                                               guint *result,
                                               GError **error);
static gboolean bbbm_options_parse_text_to_boolean(const gchar *element_name, GString *content,
                                                   gboolean *result,
                                                   GError **error);

static gboolean bbbm_options_parse_check_attributes(const gchar *element_name, const gchar **names, GError **error);
static gboolean bbbm_options_parse_get_attribute_size(const gchar *element_name,
                                                      const gchar **names, const gchar **values,
                                                      guint *width, guint *height,
                                                      GError **error);
static gboolean bbbm_options_parse_attribute_to_int(const gchar *element_name, const gchar* name, const gchar *value,
                                                    guint *result, GError **error);

/* parser error reporting functions */
static inline void bbbm_options_parse_invalid_element(GError **error, const gchar *element_name, const gchar *expected);
static inline void bbbm_options_parse_incomplete_element(GError **error, const gchar *element_name, const gchar *expected);
static inline void bbbm_options_parse_no_simple_element(GError **error, const gchar *element_name);
static inline void bbbm_options_parse_invalid_attribute(GError **error, const gchar *element_name, const gchar *name);
static inline void bbbm_options_parse_missing_attribute(GError **error, const gchar *element_name, const gchar *name);

BBBMOptions *bbbm_options_new() {
    BBBMOptions *options;

    options = g_malloc(sizeof(BBBMOptions));
    options->set_command           = g_strdup(BBBM_OPTIONS_DEFAULT_SET_COMMAND);
    options->thumb_width           = BBBM_OPTIONS_DEFAULT_THUMB_WIDTH;
    options->thumb_height          = BBBM_OPTIONS_DEFAULT_THUMB_HEIGHT;
    options->thumb_column_count    = BBBM_OPTIONS_DEFAULT_THUMB_COLUMN_COUNT;
    options->filename_as_label     = BBBM_OPTIONS_DEFAULT_FILENAME_AS_LABEL;
    options->filename_as_title     = BBBM_OPTIONS_DEFAULT_FILENAME_AS_TITLE;
    options->current_command_count = BBBM_OPTIONS_DEFAULT_COMMAND_COUNT;
    options->new_command_count     = options->current_command_count;
    options->commands              = g_malloc(options->current_command_count * sizeof(gchar *));
    options->command_labels        = g_malloc(options->current_command_count * sizeof(gchar *));
    /* clear the commands and command labels */
    memset(options->commands,       0, options->current_command_count * sizeof(gchar *));
    memset(options->command_labels, 0, options->current_command_count * sizeof(gchar *));

    return options;
}

BBBMOptions *bbbm_options_read_from_file(const gchar *filename) {
    FILE *file;
    gchar contents[BBBM_OPTIONS_READ_BUFFER_SIZE];
    gint len;
    GSList *command_iterator, *command_label_iterator;
    int i;

    GMarkupParser parser;
    GMarkupParseContext *context;
    GError *error = NULL;

    BBBMOptions *options;
    BBBMOptionsParseData parse_data;

    g_return_val_if_fail(!bbbm_str_empty(filename), NULL);

    file = fopen(filename, "r");
    if (file == NULL) {
        g_critical("could not read from '%s': %s", filename, g_strerror(errno));
        return NULL;
    }

    parser.start_element = bbbm_options_parse_start_element;
    parser.end_element   = bbbm_options_parse_end_element;
    parser.text          = bbbm_options_parse_text;
    parser.passthrough   = NULL; /* no passthrough handler */
    parser.error         = NULL; /* no error handler */

    options = g_malloc(sizeof(BBBMOptions));
    options->set_command = NULL;

    parse_data.options                  = options;
    parse_data.text_stack               = NULL; /* empty stack */
    parse_data.command_list             = NULL; /* empty command list */
    parse_data.command_label_list       = NULL; /* empty command label list */
    parse_data.command                  = NULL; /* no command */;
    parse_data.command_label            = NULL; /* no command label */
    parse_data.found_thumbs             = FALSE; /* bbbm/thumbs */
    parse_data.found_thumb_size         = FALSE;;/* bbbm/thumbs/size */
    parse_data.found_thumb_column_count = FALSE; /* bbbm/thumbs/column-count */
    parse_data.found_menu               = FALSE; /* bbbm/menu */
    parse_data.found_filename_as_label  = FALSE; /* bbbm/menu/filename-as-label */
    parse_data.found_filename_as_title  = FALSE; /* bbbm/menu/filename-as-title */
    parse_data.found_commands           = FALSE; /* bbbm/commands */
    parse_data.found_set_command        = FALSE; /* bbbm/commands/set-command */
    parse_data.found_command            = FALSE; /* bbbm/commands/command/command */
    parse_data.found_command_label      = FALSE; /* bbbm/commands/command/label */

    context = g_markup_parse_context_new(&parser, G_MARKUP_PREFIX_ERROR_POSITION, &parse_data, NULL);
    while ((len = fread(contents, sizeof(gchar), BBBM_OPTIONS_READ_BUFFER_SIZE, file)) != 0) {

        if (!g_markup_parse_context_parse(context, contents, len, &error)) {
            g_critical("error reading '%s': %s", filename, error->message);
            g_error_free(error);
            g_markup_parse_context_free(context);
            g_free(options);
            bbbm_options_close_file(file, filename);
            return NULL;
        }
    }
    if (!g_markup_parse_context_end_parse(context, &error)) {
        g_critical("error reading '%s': %s", filename, error->message);
        g_error_free(error);
        g_markup_parse_context_free(context);
        g_free(options);
        bbbm_options_close_file(file, filename);
        return NULL;
    }
    g_markup_parse_context_free(context);
    bbbm_options_close_file(file, filename);

    options->current_command_count = g_slist_length(parse_data.command_list);
    options->new_command_count     = options->current_command_count;
    options->commands              = g_malloc(options->current_command_count * sizeof(gchar *));
    options->command_labels        = g_malloc(options->current_command_count * sizeof(gchar *));
    /* clear the commands and command labels */
    memset(options->commands,       0, options->current_command_count * sizeof(gchar *));
    memset(options->command_labels, 0, options->current_command_count * sizeof(gchar *));

    command_iterator       = parse_data.command_list;
    command_label_iterator = parse_data.command_label_list;
    for (i = 0; i < options->current_command_count; i++) {
        options->commands[i]       = (gchar *) command_iterator->data;
        options->command_labels[i] = (gchar *) command_label_iterator->data;

        command_iterator       = g_slist_next(command_iterator);
        command_label_iterator = g_slist_next(command_label_iterator);
    }
    g_slist_free(parse_data.command_list);
    g_slist_free(parse_data.command_label_list);

    if (!parse_data.found_thumb_size) {
        g_info("option thumb size missing. Using default value %dx%d",
               BBBM_OPTIONS_DEFAULT_THUMB_WIDTH, BBBM_OPTIONS_DEFAULT_THUMB_HEIGHT);
        options->thumb_width  = BBBM_OPTIONS_DEFAULT_THUMB_WIDTH;
        options->thumb_height = BBBM_OPTIONS_DEFAULT_THUMB_HEIGHT;
    } else {
        if (options->thumb_width <= 0) {
            g_warning("thumb width %d <= 0. Using value 1",
                      options->thumb_width);
            options->thumb_width = 1;
        } else if (options->thumb_width > BBBM_OPTIONS_MAX_THUMB_WIDTH) {
            g_warning("thumb width %d > %d. Using value %d",
                      options->thumb_width, BBBM_OPTIONS_MAX_THUMB_WIDTH, BBBM_OPTIONS_MAX_THUMB_WIDTH);
            options->thumb_width = BBBM_OPTIONS_MAX_THUMB_WIDTH;
        }
        if (options->thumb_height <= 0) {
            g_warning("thumb height %d <= 0. Using value 1",
                      options->thumb_height);
            options->thumb_height = 1;
        } else if (options->thumb_height > BBBM_OPTIONS_MAX_THUMB_HEIGHT) {
            g_warning("thumb height %d > %d. Using value %d",
                      options->thumb_height, BBBM_OPTIONS_MAX_THUMB_HEIGHT, BBBM_OPTIONS_MAX_THUMB_HEIGHT);
            options->thumb_height = BBBM_OPTIONS_MAX_THUMB_HEIGHT;
        }
    }
    if (!parse_data.found_thumb_column_count) {
        g_info("thumb column count missing. Using default value %d",
                  BBBM_OPTIONS_DEFAULT_THUMB_COLUMN_COUNT);
        options->thumb_column_count = BBBM_OPTIONS_DEFAULT_THUMB_COLUMN_COUNT;
    } else if (options->thumb_column_count <= 0) {
        g_warning("thumb column count %d <= 0. Using value 1",
                  options->thumb_column_count);
        options->thumb_column_count = 1;
    } else if (options->thumb_column_count > BBBM_OPTIONS_MAX_THUMB_COLUMN_COUNT) {
        g_warning("thumb column count %d > %d. Using value %d",
                  options->thumb_column_count, BBBM_OPTIONS_MAX_THUMB_COLUMN_COUNT, BBBM_OPTIONS_MAX_THUMB_COLUMN_COUNT);
        options->thumb_column_count = BBBM_OPTIONS_MAX_THUMB_COLUMN_COUNT;
    }
    if (!parse_data.found_filename_as_label) {
        g_info("filename-as-label missing. Using default value %s",
               BBBM_OPTIONS_DEFAULT_FILENAME_AS_LABEL ? "true" : "false");
        options->filename_as_label = BBBM_OPTIONS_DEFAULT_FILENAME_AS_LABEL;
    }
    if (!parse_data.found_filename_as_title) {
        g_info("filename-as-title missing. Using default value %s",
               BBBM_OPTIONS_DEFAULT_FILENAME_AS_TITLE ? "true" : "false");
        options->filename_as_title = BBBM_OPTIONS_DEFAULT_FILENAME_AS_TITLE;
    }

    return options;
}

void bbbm_options_write_to_file(BBBMOptions *options, const gchar *filename) {
    int i;
    FILE *file;

    g_return_if_fail(options != NULL);
    g_return_if_fail(!bbbm_str_empty(filename));

    file = fopen(filename, "w");
    if (file == NULL) {
        g_critical("could not write to '%s': %s", filename, g_strerror(errno));
        return;
    }

    fprintf(file, "<!--\n");
    fprintf(file, "  bbbm configuration (version "VERSION")\n");
    fprintf(file, "-->\n");
    fprintf(file, "<?xml version=\"1.0\"?>\n");
    fprintf(file, "<bbbm>\n");
    fprintf(file, "  <thumbs>\n");
    fprintf(file, "    <size width=\"%d\" height=\"%d\" />\n", options->thumb_width, options->thumb_height);
    fprintf(file, "    <column-count>%d</column-count>\n", options->thumb_column_count);
    fprintf(file, "  </thumbs>\n");
    fprintf(file, "  <menu>\n");
    fprintf(file, "    <filename-as-label>%s</filename-as-label>\n", options->filename_as_label ? "true" : "false");
    fprintf(file, "    <filename-as-title>%s</filename-as-title>\n", options->filename_as_title ? "true" : "false");
    fprintf(file, "  </menu>\n");

    fprintf(file, "  <commands>\n");
    if (!bbbm_str_empty(options->set_command)) {
        fprintf(file, "    <set-command>%s</set-command>\n", options->set_command);
    } else {
        fprintf(file, "    <set-command />\n");
    }
    for (i = 0; i < options->new_command_count; i++) {
        if (i < options->current_command_count
            && (!bbbm_str_empty(options->commands[i]) || !bbbm_str_empty(options->command_labels[i]))) {

            fprintf(file, "    <command>\n");
            bbbm_options_write_element(file, "      <command>%s</command>\n", options->commands[i]);
            bbbm_options_write_element(file, "      <label>%s</label>\n", options->command_labels[i]);
            fprintf(file, "    </command>\n");
        } else {
            fprintf(file, "    <command />\n");
        }
    }
    fprintf(file, "  </commands>\n");

    fprintf(file, "</bbbm>\n");
    bbbm_options_close_file(file, filename);
}

const gchar *bbbm_options_get_set_command(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, NULL);
    return options->set_command;
}

gboolean bbbm_options_set_set_command(BBBMOptions *options, const gchar *set_command) {
    g_return_val_if_fail(options != NULL, FALSE);
    if (!bbbm_str_equals(set_command, options->set_command)) {
        g_free(options->set_command);
        options->set_command = g_strdup(set_command);
        return TRUE;
    }
    return FALSE;
}

const guint bbbm_options_get_thumb_width(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, 0);
    return options->thumb_width;
}

const guint bbbm_options_get_thumb_height(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, 0);
    return options->thumb_height;
}

gboolean bbbm_options_set_thumb_size(BBBMOptions *options, const guint width, const guint height) {
    g_return_val_if_fail(options != NULL, FALSE);
    if (width != options->thumb_width || height != options->thumb_height) {
        options->thumb_width = width;
        options->thumb_height = height;
        return TRUE;
    }
    return FALSE;
}

const guint bbbm_options_get_thumb_column_count(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, 0);
    return options->thumb_column_count;
}

gboolean bbbm_options_set_thumb_column_count(BBBMOptions *options, const guint column_count) {
    g_return_val_if_fail(options != NULL, FALSE);
    if (column_count != options->thumb_column_count) {
        options->thumb_column_count = column_count;
        return TRUE;
    }
    return FALSE;
}

const gboolean bbbm_options_get_filename_as_label(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, FALSE);
    return options->filename_as_label;
}

gboolean bbbm_options_set_filename_as_label(BBBMOptions *options, const gboolean filename_as_label) {
    g_return_val_if_fail(options != NULL, FALSE);
    if (filename_as_label != options->filename_as_label) {
        options->filename_as_label = filename_as_label;
        return TRUE;
    }
    return FALSE;
}

const gboolean bbbm_options_get_filename_as_title(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, FALSE);
    return options->filename_as_title;
}

gboolean bbbm_options_set_filename_as_title(BBBMOptions *options, const gboolean filename_as_title) {
    g_return_val_if_fail(options != NULL, FALSE);
    if (filename_as_title != options->filename_as_title) {
        options->filename_as_title = filename_as_title;
        return TRUE;
    }
    return FALSE;
}

const guint bbbm_options_get_current_command_count(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, 0);
    return options->current_command_count;
}

const guint bbbm_options_get_new_command_count(BBBMOptions *options) {
    g_return_val_if_fail(options != NULL, 0);
    return options->new_command_count;
}

gboolean bbbm_options_set_command_count(BBBMOptions *options, const guint command_count) {
    g_return_val_if_fail(options != NULL, FALSE);
    if (command_count != options->new_command_count) {
        options->new_command_count = command_count;
        return TRUE;
    }
    return FALSE;
}

const gchar *bbbm_options_get_command(BBBMOptions *options, guint index) {
    g_return_val_if_fail(options != NULL, NULL);
    g_return_val_if_fail(0 <= index && index < options->current_command_count, NULL);
    return options->commands[index];
}

gboolean bbbm_options_set_command(BBBMOptions *options, guint index, const gchar *command) {
    g_return_val_if_fail(options != NULL, FALSE);
    g_return_val_if_fail(0 <= index && index < options->current_command_count, FALSE);
    if (!bbbm_str_equals(command, options->commands[index])) {

        g_free(options->commands[index]);
        options->commands[index] = g_strdup(command);
        return TRUE;
    }
    return FALSE;
}

const gchar *bbbm_options_get_command_label(BBBMOptions *options, guint index) {
    g_return_val_if_fail(options != NULL, NULL);
    g_return_val_if_fail(0 <= index && index < options->current_command_count, NULL);
    return options->command_labels[index];
}

gboolean bbbm_options_set_command_label(BBBMOptions *options, guint index, const gchar *label) {
    g_return_val_if_fail(options != NULL, FALSE);
    g_return_val_if_fail(0 <= index && index < options->current_command_count, FALSE);
    if (!bbbm_str_equals(label, options->command_labels[index])) {

        g_free(options->command_labels[index]);
        options->command_labels[index] = g_strdup(label);
        return TRUE;
    }
    return FALSE;
}

void bbbm_options_destroy(BBBMOptions *options) {
    int i;

    g_return_if_fail(options != NULL);
    g_free(options->set_command);
    for (i = 0; i < options->current_command_count; i++) {
        g_free(options->commands[i]);
        g_free(options->command_labels[i]);
    }
    g_free(options->commands);
    g_free(options->command_labels);
    g_free(options);
}

static inline void bbbm_options_close_file(FILE *file, const gchar *filename) {
    if (fclose(file) != 0) {
        g_warning("could not close file '%s': %s", filename, g_strerror(errno));
    }
}

static inline void bbbm_options_write_element(FILE *file, const gchar *text, const gchar *value) {
    if (!bbbm_str_empty(value)) {
        gchar *escaped = g_markup_escape_text(value, -1);
        fprintf(file, text, escaped);
        g_free(escaped);
    }
}

static void bbbm_options_parse_start_element(GMarkupParseContext *context,
                                             const gchar *element_name,
                                             const gchar **attribute_names, const gchar **attribute_values,
                                             gpointer user_data,
                                             GError **error) {

    const GSList *element_stack;
    guint depth;
    const gchar *parent_element_name;
    BBBMOptionsParseData *parse_data;

    element_stack = g_markup_parse_context_get_element_stack(context);
    depth = g_slist_length((GSList *) element_stack);

    parse_data = (BBBMOptionsParseData *) user_data;

    g_debug("found start element '%s'", element_name);

    switch (depth) {
        case 1:
            /* allowed: bbbm */
            if (!bbbm_str_equals("bbbm", element_name)) {
                g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                            "cannot find declaration of element '%s'", element_name);
            }
            bbbm_options_parse_check_attributes(element_name, attribute_names, error);
            break;
        case 2:
            /* allowed: thumbs, menu, commands */
            if (!parse_data->found_thumbs) {
                /* didn't find thumbs yet, so element_name must be thumbs, menu or commands */
                if (bbbm_str_equals("thumbs", element_name)) {
                    bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                    parse_data->found_thumbs = TRUE;
                } else if (bbbm_str_equals("menu", element_name)) {
                    bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                    parse_data->found_menu = TRUE;
                } else if (bbbm_str_equals("commands", element_name)) {
                    bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                    parse_data->found_commands = TRUE;
                } else {
                    bbbm_options_parse_invalid_element(error, element_name, "thumbs, menu, commands");
                }
            } else if (!parse_data->found_menu) {
                /* found thumbs but not menu, so element_name must be menu or commands */
                if (bbbm_str_equals("menu", element_name)) {
                    bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                    parse_data->found_menu = TRUE;
                } else if (bbbm_str_equals("commands", element_name)) {
                    bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                    parse_data->found_commands = TRUE;
                } else {
                    bbbm_options_parse_invalid_element(error, element_name, "menu, commands");
                }
            } else if (!parse_data->found_commands) {
                /* found thumbs and menu but not commands, so element_name must be commands */
                if (bbbm_str_equals("commands", element_name)) {
                    bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                    parse_data->found_commands = TRUE;
                } else {
                    bbbm_options_parse_invalid_element(error, element_name, "commands");
                }
            } else {
                /* found thumbs, menu and commands; no other element names allowed */
                bbbm_options_parse_invalid_element(error, element_name, NULL);
            }
            break;
        case 3:
            parent_element_name = (const gchar *) g_slist_next(element_stack)->data;
            if (bbbm_str_equals("thumbs", parent_element_name)) {
                /* allowed: size, column-count */
                if (!parse_data->found_thumb_size) {
                    /* didn't find size yet, so element_name must be size or column-count */
                    if (bbbm_str_equals("size", element_name)) {
                        bbbm_options_parse_get_attribute_size(element_name, attribute_names, attribute_values,
                                                              &(parse_data->options->thumb_width),
                                                              &(parse_data->options->thumb_height),
                                                              error);
                        g_debug("found thumb size %dx%d",
                                parse_data->options->thumb_width, parse_data->options->thumb_height);
                        parse_data->found_thumb_size = TRUE;
                    } else if (bbbm_str_equals("column-count", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_thumb_column_count = TRUE;
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "size, column-count");
                    }
                } else if (!parse_data->found_thumb_column_count) {
                    /* found size but not column-count, so element_name must be column-count */
                    if (bbbm_str_equals("column-count", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_thumb_column_count = TRUE;
                        /* content is handled in text + end_element handling */
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "column-count");
                    }
                } else {
                    /* found size and column-count; no other element names allowed */
                    bbbm_options_parse_invalid_element(error, element_name, NULL);
                }
            } else if (bbbm_str_equals("menu", parent_element_name)) {
                /* allowed: filename-as-label, filename-as-title */
                if (!parse_data->found_filename_as_label) {
                    /* didn't find filename-as-label yet, so element_name must be filename-as-label or filename-as-title */
                    if (bbbm_str_equals("filename-as-label", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_filename_as_label = TRUE;
                        /* content is handled in text + end_element handling */
                    } else if (bbbm_str_equals("filename-as-title", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_filename_as_title = TRUE;
                        /* content is handled in text + end_element handling */
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "filename-as-label, filename-as-title");
                    }
                } else if (!parse_data->found_filename_as_title) {
                    /* found filename-as-label but not filename-as-title, so element_name must be filename-as-title */
                    if (bbbm_str_equals("filename-as-title", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_filename_as_title = TRUE;
                        /* content is handled in text + end_element handling */
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "filename-as-title");
                    }
                } else {
                    /* found filename-as-label and filename-as-title; no other element names allowed */
                    bbbm_options_parse_invalid_element(error, element_name, NULL);
                }
            } else if (bbbm_str_equals("commands", parent_element_name)) {
                /* allowed: set-command, command* */
                if (!parse_data->found_set_command) {
                    /* didn't find set-command yet, so element_name must be set-command */
                    if (bbbm_str_equals("set-command", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_set_command = TRUE;
                        /* content is handled in text + end_element handling */
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "set-command");
                    }
                } else {
                    /* found set-command, so element_name must be command */
                    if (bbbm_str_equals("command", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        /* reset found_command and found_command_label */
                        parse_data->found_command = FALSE;
                        parse_data->found_command_label = FALSE;
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "command");
                    }
                }
            } else {
                /* should not occur; any other depth=2 elements should have produced an error */
                bbbm_options_parse_invalid_element(error, element_name, NULL);
            }
            break;
        case 4:
            parent_element_name = (const gchar *) g_slist_next(element_stack)->data;
            if (bbbm_str_equals("command", parent_element_name)) {
                /* allowed: command, label; both are optional though */
                if (!parse_data->found_command) {
                    /* didn;t find command yet, so element_name must be command or label */
                    if (bbbm_str_equals("command", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_command = TRUE;
                        /* content is handled in text + end_element handling */
                    } else if (bbbm_str_equals("label", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_command_label = TRUE;
                        /* content is handled in text + end_element handling */
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "command, label");
                    }
                } else if (!parse_data->found_command_label) {
                    /* found command, so element_name must be label */
                    if (bbbm_str_equals("label", element_name)) {
                        bbbm_options_parse_check_attributes(element_name, attribute_names, error);
                        parse_data->found_command_label = TRUE;
                        /* content is handled in text + end_element handling */
                    } else {
                        bbbm_options_parse_invalid_element(error, element_name, "label");
                    }
                } else {
                    /* found command and label; no other elements allowed */
                    bbbm_options_parse_invalid_element(error, element_name, NULL);
                }
            } else {
                /* the only depth=3 element that can have child elements is command */
                bbbm_options_parse_no_simple_element(error, element_name);
            }
            break;
        default:
            /* a depth greater than 4 is not allowed */
            bbbm_options_parse_no_simple_element(error, element_name);
    }

    /* push a NULL as first element */
    parse_data->text_stack = g_slist_prepend(parse_data->text_stack, NULL);
}

static void bbbm_options_parse_end_element(GMarkupParseContext *context,
                                           const gchar *element_name,
                                           gpointer user_data,
                                           GError **error) {

    const GSList *element_stack;
    guint depth;
    const gchar *parent_element_name;
    BBBMOptionsParseData *parse_data;
    GString *text;

    element_stack = g_markup_parse_context_get_element_stack(context);
    depth = g_slist_length((GSList *) element_stack);

    parse_data = (BBBMOptionsParseData *) user_data;

    /* get then pop the first element */
    text = (GString *) parse_data->text_stack->data;
    if (text != NULL) {
        g_strstrip(text->str);
        text->len = strlen(text->str);
    }

    g_debug("found end element '%s' with text '%s'", element_name, text ? text->str : "");

    parse_data->text_stack = g_slist_delete_link(parse_data->text_stack, parse_data->text_stack);

    switch (depth) {
        case 1:
            /* allowed: bbbm */
            /* for bbbm, commands must have been found */
            if (bbbm_str_equals("bbbm", element_name) && !parse_data->found_commands) {
                bbbm_options_parse_incomplete_element(error, element_name, "thumbs, menu, commands");
            }
            break;
        case 2:
            /* allowed: thumbs, menu, commands */
            /* for commands, set-command must have been found */
            if (bbbm_str_equals("thumbs", element_name) || bbbm_str_equals("menu", element_name)) {
                bbbm_options_parse_check_empty_content(element_name, text, error);
            } else if (bbbm_str_equals("commands", element_name)) {
                if (!parse_data->found_set_command) {
                    bbbm_options_parse_incomplete_element(error, element_name, "set-command");
                } else {
                    bbbm_options_parse_check_empty_content(element_name, text, error);
                }
            }
            break;
        case 3:
            parent_element_name = (const gchar *) g_slist_next(element_stack)->data;
            if (bbbm_str_equals("thumbs", parent_element_name)) {
                /* allowed: size, column-count */
                if (bbbm_str_equals("size", element_name)) {
                    bbbm_options_parse_check_empty_content(element_name, text, error);
                } else if (bbbm_str_equals("column-count", element_name)) {
                    bbbm_options_parse_text_to_int(element_name, text,
                                                   &(parse_data->options->thumb_column_count),
                                                   error);
                    g_debug("found thumb column count %d",
                            parse_data->options->thumb_column_count);
                }
            } else if (bbbm_str_equals("menu", parent_element_name)) {
                /* allowed: filename-as-label, filename-as-title */
                if (bbbm_str_equals("filename-as-label", element_name)) {
                    bbbm_options_parse_text_to_boolean(element_name, text,
                                                       &(parse_data->options->filename_as_label),
                                                       error);
                    g_debug("found filename-as-label %s",
                            parse_data->options->filename_as_label ? "true" : "false");
                } else if (bbbm_str_equals("filename-as-title", element_name)) {
                    bbbm_options_parse_text_to_boolean(element_name, text,
                                                       &(parse_data->options->filename_as_title),
                                                       error);
                    g_debug("found filename-as-title %s",
                            parse_data->options->filename_as_title ? "true" : "false");
                }
            } else if (bbbm_str_equals("commands", parent_element_name)) {
                /* allowed: set-command, command* */
                if (bbbm_str_equals("set-command", element_name)) {
                    parse_data->options->set_command = BBBM_OPTIONS_GET_STRING(text);
                    g_debug("found set command '%s'",
                            parse_data->options->set_command);
                } else if (bbbm_str_equals("command", element_name)) {
                    parse_data->command_list       = g_slist_append(parse_data->command_list,       parse_data->command);
                    parse_data->command_label_list = g_slist_append(parse_data->command_label_list, parse_data->command_label);
                    g_debug("found command '%s' with label '%s'",
                            parse_data->command, parse_data->command_label);
                    parse_data->command = NULL;
                    parse_data->command_label = NULL;
                }
            }
            break;
        case 4:
            parent_element_name = (const gchar *) g_slist_next(element_stack)->data;
            if (bbbm_str_equals("command", parent_element_name)) {
                /* allowed: command, label */
                if (bbbm_str_equals("command", element_name)) {
                    parse_data->command = BBBM_OPTIONS_GET_STRING(text);
                } else if (bbbm_str_equals("label", element_name)) {
                    parse_data->command_label = BBBM_OPTIONS_GET_STRING(text);
                }
            }
            break;
    }

    if (text != NULL) {
        g_string_free(text, TRUE);
    }
}

static void bbbm_options_parse_text(GMarkupParseContext *context,
                                    const gchar *text, gsize text_len,
                                    gpointer user_data,
                                    GError **error) {

    BBBMOptionsParseData *parse_data;
    GString *current_text;

    parse_data = (BBBMOptionsParseData *) user_data;

    current_text = (GString *) parse_data->text_stack->data;

    if (current_text == NULL) {
        current_text = g_string_sized_new(1024);
    }
    current_text = g_string_append_len(current_text, text, text_len);

    parse_data->text_stack->data = current_text;
}

static gboolean bbbm_options_parse_check_empty_content(const gchar *element_name, GString *content,
                                                       GError **error) {
    if (content != NULL && content->len > 0) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                    "Element '%s' cannot have character [children], because the type's content type is element only.",
                    element_name);
        return FALSE;
    }
    return TRUE;
}

static gboolean bbbm_options_parse_text_to_int(const gchar *element_name, GString *content,
                                               guint *result,
                                               GError **error) {
    if (content != NULL) {
        gchar *end;
        guint i;

        i = (guint) BBBM_STRTOD(content->str, &end);
        if (*end == '\0' && end != content->str) {
            *result = i;
            return TRUE;
        }
    }
    g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                "The value '%s' of element '%s' is not a valid value for 'integer'",
                content ? content->str : "", element_name);
    return FALSE;
}

static gboolean bbbm_options_parse_text_to_boolean(const gchar *element_name, GString *content,
                                                   gboolean *result,
                                                   GError **error) {
    if (content != NULL) {
        if (bbbm_str_equals("true", content->str)) {
            *result = TRUE;
            return TRUE;
        }
        if (bbbm_str_equals("false", content->str)) {
            *result = FALSE;
            return TRUE;
        }
    }
    g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                "The value '%s' of element '%s' is not a valid value for 'boolean'",
                content ? content->str : "", element_name);
    return FALSE;
}

static gboolean bbbm_options_parse_check_attributes(const gchar *element_name, const gchar **names, GError **error) {
    if (names[0] != NULL) {
        bbbm_options_parse_invalid_attribute(error, element_name, names[0]);
        return FALSE;
    }
    return TRUE;
}

static gboolean bbbm_options_parse_get_attribute_size(const gchar *element_name,
                                                      const gchar **names, const gchar **values,
                                                      guint *width, guint *height,
                                                      GError **error) {
    int i;
    gboolean found_width = FALSE;
    gboolean found_height = FALSE;

    for (i = 0; names[i] != NULL; i++) {
        if (bbbm_str_equals("width", names[i])) {
            found_width = TRUE;
            if (!bbbm_options_parse_attribute_to_int(element_name, names[i], values[i], width, error)) {
                return FALSE;
            }
        } else if (bbbm_str_equals("height", names[i])) {
            found_height = TRUE;
            if (!bbbm_options_parse_attribute_to_int(element_name, names[i], values[i], height, error)) {
                return FALSE;
            }
        } else {
            bbbm_options_parse_invalid_attribute(error, element_name, names[i]);
            return FALSE;
        }
    }
    if (!found_width) {
        bbbm_options_parse_missing_attribute(error, element_name, "width");
        return FALSE;
    }
    if (!found_height) {
        bbbm_options_parse_missing_attribute(error, element_name, "height");
        return FALSE;
    }
    return TRUE;
}

static gboolean bbbm_options_parse_attribute_to_int(const gchar *element_name, const gchar* name, const gchar *value,
                                                    guint *result, GError **error) {
    gchar *end;
    guint i;

    i = (guint) BBBM_STRTOD(value, &end);
    if (*end != '\0' || end == value) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                    "The value '%s' of attribute '%s' on element '%s' is not valid with respect to its type, 'integer'.",
                    value, name, element_name);
        return FALSE;
    }
    *result = i;
    return TRUE;
}

static inline void bbbm_options_parse_invalid_element(GError **error, const gchar *element_name, const gchar *expected) {
    if (expected != NULL) {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                    "Invalid content was found starting with element '%s'. One of '{%s}' is expected.",
                    element_name, expected);
    } else {
        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                    "Invalid content was found starting with element '%s'. No child element is expected at this point.",
                    element_name);
    }
}

static inline void bbbm_options_parse_incomplete_element(GError **error, const gchar *element_name, const gchar *expected) {
    g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                "The content of element '%s' is not complete. One of '{%s}' is expected",
                element_name, expected);
}

static inline void bbbm_options_parse_no_simple_element(GError **error, const gchar *element_name) {
    g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                "Element '%s' is a simple type, so it must have no element information item [children].",
                element_name);
}

static inline void bbbm_options_parse_invalid_attribute(GError **error, const gchar *element_name, const gchar *name) {
    g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
                "Attribute '%s' is not allowed to appear in element '%s'",
                name, element_name);
}

static inline void bbbm_options_parse_missing_attribute(GError **error, const gchar *element_name, const gchar *name) {
    g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                "Attribute '%s' must appear on element '%s'",
                name, element_name);
}
