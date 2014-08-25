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

#include <balde.h>
#include <glib.h>
#include "balde-album.h"
#include "image.h"
#include "utils.h"
#include "template.h"


balde_response_t*
ba_view_index(balde_app_t *app, balde_request_t *req)
{
    balde_response_t *rv = balde_make_response("");
    ba_tmpl_header(app, rv);
    ba_tmpl_image_table(app, rv);
    ba_tmpl_footer(app, rv);
    return rv;
}


balde_response_t*
ba_view_image(balde_app_t *app, balde_request_t *req)
{
    const gchar *filename = balde_request_get_view_arg(req, "filename");
    if (filename == NULL)
        return balde_abort(app, 400);
    ba_image_t *img = ba_get_image_from_filename(app, filename);
    if (img == NULL)
        return balde_abort(app, 404);
    balde_response_t *rv = balde_make_response("");
    ba_tmpl_header(app, rv);
    ba_tmpl_image_detail(app, rv, img);
    ba_tmpl_footer(app, rv);
    return rv;
}


balde_response_t*
ba_view_full(balde_app_t *app, balde_request_t *req)
{
    const gchar *filename = balde_request_get_view_arg(req, "filename");
    if (filename == NULL)
        return balde_abort(app, 400);
    ba_image_t *img = ba_get_image_from_filename(app, filename);
    if (img == NULL)
        return balde_abort(app, 404);
    GString *full = ba_get_formatted_full(img->filepath);
    if (full == NULL)
        return balde_abort(app, 404);
    balde_response_t *rv = balde_make_response_len(full->str, full->len);
    g_string_free(full, TRUE);
    if (img->mimetype != NULL)
        balde_response_set_header(rv, "Content-Type", img->mimetype);
    return rv;
}


balde_response_t*
ba_view_thumb(balde_app_t *app, balde_request_t *req)
{
    const gchar *filename = balde_request_get_view_arg(req, "filename");
    if (filename == NULL)
        return balde_abort(app, 400);
    ba_image_t *img = ba_get_image_from_filename(app, filename);
    if (img == NULL)
        return balde_abort(app, 404);
    GString *thumb = ba_get_formatted_thumb(img->filepath);
    if (thumb == NULL)
        return balde_abort(app, 404);
    balde_response_t *rv = balde_make_response_len(thumb->str, thumb->len);
    g_string_free(thumb, TRUE);
    if (img->mimetype != NULL)
        balde_response_set_header(rv, "Content-Type", img->mimetype);
    return rv;
}


balde_response_t*
ba_view_resized(balde_app_t *app, balde_request_t *req)
{
    const gchar *filename = balde_request_get_view_arg(req, "filename");
    if (filename == NULL)
        return balde_abort(app, 400);
    ba_image_t *img = ba_get_image_from_filename(app, filename);
    if (img == NULL)
        return balde_abort(app, 404);
    GString *image = ba_get_formatted_image(img->filepath);
    if (image == NULL)
        return balde_abort(app, 404);
    balde_response_t *rv = balde_make_response_len(image->str, image->len);
    g_string_free(image, TRUE);
    if (img->mimetype != NULL)
        balde_response_set_header(rv, "Content-Type", img->mimetype);
    return rv;
}
