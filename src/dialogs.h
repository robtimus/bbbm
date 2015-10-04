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

#ifndef __BBBM_DIALOGS_H_
#define __BBBM_DIALOGS_H_

#include "bbbm.h"

/* Shows the Options dialog for the given BBBM application. */
void bbbm_options_dialog(BBBM *bbbm);

/* Shows the About dialog for the given BBBM application. */
void bbbm_about_dialog(BBBM *bbbm);

/* Shows the Move dialogs for the given BBBM image at the given index.
   Forward indicates whether it is a Move forward or Move back dialog. */
void bbbm_move_dialog(BBBMImage *image, gint index, gboolean forward);

/* Shows the Edit description dialog for the given BBBM image. */
void bbbm_edit_description_dialog(BBBMImage *image);

/* Shows an error dialog with the given message. */
void bbbm_error_message_dialog(BBBM *bbbm, const gchar *message);

#endif /* __BBBM_DIALOG_H_ */
