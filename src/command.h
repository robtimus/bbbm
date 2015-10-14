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

#ifndef __BBBM_COMMAND_H_
#define __BBBM_COMMAND_H_

#include <glib.h>

typedef struct _BBBMCommand BBBMCommand;

struct _BBBMCommand {
    gchar *command;
    gchar *label;
};

/* Creates a new BBBMCommand object with the given command and label.
   The returned object must be freed with bbbm_command_destroy when no longer needed */
BBBMCommand *bbbm_command_new(const gchar *command, const gchar *label);

const gchar *bbbm_command_get_command(BBBMCommand *cmd);
gboolean bbbm_command_set_command(BBBMCommand *cmd, const gchar *command);

const gchar *bbbm_command_get_label(BBBMCommand *cmd);
gboolean bbbm_command_set_label(BBBMCommand *cmd, const gchar *label);

void bbbm_command_destroy(BBBMCommand *cmd);

#endif /* __BBBM_COMMAND_H_ */
