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

#include <glib.h>
#include "command.h"
#include "util.h"

BBBMCommand *bbbm_command_new(const gchar *command, const gchar *label) {
    BBBMCommand *cmd;

    cmd = g_malloc(sizeof(BBBMCommand));
    cmd->id = 12345;
    cmd->command = g_strdup(command);
    cmd->label   = g_strdup(label);

    return cmd;
}

const gchar *bbbm_command_get_command(BBBMCommand *cmd) {
    g_return_val_if_fail(cmd != NULL, NULL);
    return cmd->command;
}

gboolean bbbm_command_set_command(BBBMCommand *cmd, const gchar *command) {
    g_return_val_if_fail(cmd != NULL, FALSE);
    if (!bbbm_str_equals(command, cmd->command)) {
        g_free(cmd->command);
        cmd->command = g_strdup(command);
        return TRUE;
    }
    return FALSE;
}

const gchar *bbbm_command_get_label(BBBMCommand *cmd) {
    g_return_val_if_fail(cmd != NULL, NULL);
    return cmd->label;
}

gboolean bbbm_command_set_label(BBBMCommand *cmd, const gchar *label) {
    g_return_val_if_fail(cmd != NULL, FALSE);
    if (!bbbm_str_equals(label, cmd->label)) {
        g_free(cmd->label);
        cmd->label = g_strdup(label);
        return TRUE;
    }
    return FALSE;
}

void bbbm_command_destroy(BBBMCommand *cmd) {
    g_return_if_fail(cmd != NULL);
    g_free(cmd->command);
    g_free(cmd->label);
    g_free(cmd);
}
