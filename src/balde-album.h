/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_H
#define _BALDE_ALBUM_H

#include <glib.h>

#define BUFFER_SIZE 1024
#define IMAGES_PER_ROW 4
#define MAX_FILENAME_LENGTH 32
#define MAX_IMAGE_WIDTH 900

typedef struct {
    gchar *name;
    gchar *title;
    gchar *value;
} ba_image_metadata_t;

typedef struct {
    gchar *filepath;  // nul-terminated string
    gchar *filename;  // nul-terminated string
    gchar *mimetype;  // nul-terminated string
    GString *image;   // binary string
    GString *thumb;   // binary string
    GSList *metadata;
} ba_image_t;

typedef struct {
    GSList *images;
    // TODO: expand me!
} ba_user_data_t;

#endif /* _BALDE_ALBUM_H */
