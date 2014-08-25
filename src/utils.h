/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_UTILS_H
#define _BALDE_ALBUM_UTILS_H

#include <balde.h>
#include <glib.h>
#include "balde-album.h"

void ba_free_image_metadata(ba_image_metadata_t *meta);
void ba_free_image(ba_image_t *img);
ba_image_t* ba_get_image_from_filename(balde_app_t *app, const gchar *filename);
gchar* ba_shorten_filename(const gchar *filename);

#endif /* _BALDE_ALBUM_UTILS_H */
