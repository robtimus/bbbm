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

#ifndef __BBBM_COMPAT_H_
#define __BBBM_COMPAT_H_

#include <glib.h>

/* if not defined (because glib is too old), define g_info here */
#ifndef g_info
#define g_info(...)   g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

/* if not defined (because glib is too old), define g_debug here */
#ifndef g_debug
#define g_debug(...)  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#endif

/* autoconf doesn't recognize glib/gtk+ constants and functions; do it manually based on version numbers */

/* g_markup_parse_context_get_element_stack is available since glib 2.16 */
/* although not explicitly documented, so are G_MARKUP_PREFIX_ERROR_POSITION, G_MARKUP_ERROR_MISSING_ATTRIBUTE */
#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 16
#define HAVE_G_MARKUP_PARSE_CONTEXT_GET_ELEMENT_STACK  0
#define HAVE_G_MARKUP_PREFIX_ERROR_POSITION            0
#define HAVE_G_MARKUP_ERROR_MISSING_ATTRIBUTE          0
#else
#define HAVE_G_MARKUP_PARSE_CONTEXT_GET_ELEMENT_STACK  1
#define HAVE_G_MARKUP_PREFIX_ERROR_POSITION            1
#define HAVE_G_MARKUP_ERROR_MISSING_ATTRIBUTE          1
#endif

/* G_MARKUP_TREAT_CDATA_AS_TEXT is available since glib 2.12 */
#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 12
#define HAVE_G_MARKUP_TREAT_CDATA_AS_TEXT  0
#else
#define HAVE_G_MARKUP_TREAT_CDATA_AS_TEXT  1
#endif

/* gtk_button_set_image is available since gtk+ 2.6 */
#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 6
#define HAVE_GTK_BUTTON_SET_IMAGE  0
#else
#define HAVE_GTK_BUTTON_SET_IMAGE  1
#endif

/* gtk_menu_item's label property is available since gtk+ 2.16 */
#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 16
#define HAVE_GTK_MENU_ITEM_LABEL  0
#else
#define HAVE_GTK_MENU_ITEM_LABEL  1
#endif

#endif /* __BBBM_COMPAT_H_ */
