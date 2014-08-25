/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_LOADER_H
#define _BALDE_ALBUM_LOADER_H

#include <glib.h>

ba_image_t* ba_load_image_file(const gchar *filepath);
GSList* ba_load_images_directory(const gchar *dirpath);

#endif /* _BALDE_ALBUM_LOADER_H */
