/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_H
#define _BALDE_ALBUM_H

#include <balde.h>
#include <glib.h>

#define BUFFER_SIZE 1024
#define IMAGES_PER_ROW 4
#define MAX_FILENAME_LENGTH 32
#define MAX_IMAGE_WIDTH 900
#define MAX_IMAGE_HEIGHT 700
#define MAX_THUMB_WIDTH 200
#define MAX_THUMB_HEIGHT 150

typedef struct {
    gchar *name;
    gchar *title;
    gchar *value;
} ba_image_metadata_t;

typedef struct {
    gchar *filepath;
    gchar *filename;
    gchar *mimetype;
    GSList *metadata;
} ba_image_t;

typedef struct {
    GSList *images;
    // TODO: expand me!
} ba_user_data_t;

balde_app_t* ba_app_init(void);
void ba_app_free(balde_app_t *app);

#endif /* _BALDE_ALBUM_H */
