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

#include <glib.h>
#include <gio/gio.h>
#include "balde-album.h"
#include "exif.h"
#include "loader.h"


ba_image_t*
ba_load_image_file(const gchar *filepath)
{
    g_return_val_if_fail(filepath != NULL, NULL);
    gchar *contents;
    gsize length;
    GError *tmp_error = NULL;
    if (!g_file_get_contents(filepath, &contents, &length, &tmp_error)) {
        g_printerr("%s\n", tmp_error->message);
        g_error_free(tmp_error);
        return NULL;
    }
    ba_image_t *img = g_new(ba_image_t, 1);
    img->filepath = g_strdup(filepath);
    img->filename = g_path_get_basename(filepath);
    img->image = g_string_new_len(contents, length);
    g_free(contents);
    img->thumb = NULL;
    img->metadata = ba_dump_exif(img->image);
    return img;
}


static gint
ba_sort_images_by_filename(ba_image_t *a, ba_image_t *b)
{
    return g_strcmp0(a->filename, b->filename);
}


GSList*
ba_load_images_directory(const gchar *dirpath)
{
    if (dirpath == NULL)
        return NULL;
    GError *tmp_error = NULL;
    GDir *dir = g_dir_open(dirpath, 0, &tmp_error);
    if (tmp_error != NULL) {
        // buh! nothing to do if we can't open the dir :P
        g_printerr("%s\n", tmp_error->message);
        g_error_free(tmp_error);
        return NULL;
    }
    GSList *rv = NULL;
    const gchar *tmp;
    while ((tmp = g_dir_read_name(dir)) != NULL) {
        gchar *filepath = g_build_filename(dirpath, tmp, NULL);
        // we just need to load files!
        if (!g_file_test(filepath, G_FILE_TEST_IS_REGULAR))
            goto clean1;
        // even better... we just need to load images!
        gchar *mime = g_content_type_guess(filepath, NULL, 0, NULL);
        if (!g_str_has_prefix(mime, "image/"))
            goto clean2;
        ba_image_t *img = ba_load_image_file(filepath);
        if (img == NULL)
            goto clean2;
        img->mimetype = g_strdup(mime);
        rv = g_slist_insert_sorted(rv, img,
            (GCompareFunc) ba_sort_images_by_filename);
clean2:
        g_free(mime);
clean1:
        g_free(filepath);
    }
    g_dir_close(dir);
    return rv;
}
