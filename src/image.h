/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_IMAGE_H
#define _BALDE_ALBUM_IMAGE_H

#include <glib.h>

GString* ba_format_image(GString *image, gsize max_width, gsize max_height);
GString* ba_get_formatted_image(const gchar *filepath);
GString* ba_get_formatted_thumb(const gchar *filepath);
GString* ba_get_formatted_full(const gchar *filepath);

#endif /* _BALDE_ALBUM_IMAGE_H */
