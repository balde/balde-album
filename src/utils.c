/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <math.h>
#include <balde.h>
#include <glib.h>
#include <jpeglib.h>
#include "balde-album.h"
#include "utils.h"


void
ba_free_image_metadata(ba_image_metadata_t *meta)
{
    g_free(meta->name);
    g_free(meta->title);
    g_free(meta->value);
    g_free(meta);
}


void
ba_free_image(ba_image_t *img)
{
    g_free(img->filepath);
    g_free(img->filename);
    g_free(img->mimetype);
    if (img->image != NULL)
        g_string_free(img->image, TRUE);
    if (img->thumb != NULL)
        g_string_free(img->thumb, TRUE);
    g_slist_free_full(img->metadata, (GDestroyNotify) ba_free_image_metadata);
    g_free(img);
}


ba_image_t*
ba_get_image_from_filename(balde_app_t *app, const gchar *filename)
{
    ba_user_data_t *ud = app->user_data;
    for (GSList *tmp = ud->images; tmp != NULL; tmp = g_slist_next(tmp)) {
        ba_image_t *img = tmp->data;
        if (g_strcmp0(img->filename, filename) == 0)
            return img;
    }
    return NULL;
}


gint64
ba_get_image_width(ba_image_t *img)
{
    // let's try exif first, as it should be cheapest.
    if (img->metadata != NULL) {
        for (GSList *tmp = img->metadata; tmp != NULL; tmp = g_slist_next(tmp)) {
            ba_image_metadata_t *meta = tmp->data;
            if (g_strcmp0(meta->name, "PixelXDimension") == 0) {
                return g_ascii_strtoll(meta->value, NULL, 10);
            }
        }
    }

    // ok, no exif for us, let's try jpeg decompression
    if (g_strcmp0(img->mimetype, "image/jpeg") == 0) {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, img->image->str, img->image->len);
        gint rc = jpeg_read_header(&cinfo, TRUE);
        if (rc == 1) {
            jpeg_start_decompress(&cinfo);
            gint64 width = cinfo.output_width;
            jpeg_destroy_decompress(&cinfo);
            return width;
        }
    }

    // no jpeg, no exif... png maybe?! hmm... no!
    return -1;
}


gchar*
ba_shorten_filename(const gchar *filename)
{
    if (g_utf8_strlen(filename, -1) <= MAX_FILENAME_LENGTH)
        return g_strdup(filename);
    // 3 is the length of '...'
    guint start = ceilf((gfloat) (MAX_FILENAME_LENGTH - 3) / 2);
    guint end = MAX_FILENAME_LENGTH - start - 3;
    GString *rv = g_string_new("");  // laaaaaaaazy :P
    for (guint i = 0; i < start; i++) {
        g_string_append_c(rv, filename[i]);
    }
    g_string_append(rv, "...");
    guint index = g_utf8_strlen(filename, -1) - end;
    for (guint j = index; j < (index + end); j++) {
        g_string_append_c(rv, filename[j]);
    }
    return g_string_free(rv, FALSE);
}
