/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

/**
 * TGV 9713 - PARIS GARE LYON -> BARCELONA SANTS - August 12, 2014 - 10:07
 *
 * It's going to be a long journey...
 */


/**
 * Read me!!!!11!
 *
 *
 * How to build this thing?!
 *
 * $ gcc -std=c99 -o balde-album balde-album.c $(pkg-config --libs --cflags \
 *       glib-2.0 gio-2.0 libexif balde) -lm -ljpeg
 *
 * It depends on balde, glib, gio, libexif and libjpeg/libjpeg-turbo
 *
 *
 * Where are the tests?!
 *
 * Seriously? :P
 *
 *
 * How to run this shit?!
 *
 * Take a look at balde documentation. A sample apache snippet, with
 * mod_fastcgid would look like this:
 *
 * <VirtualHost *:80>
 *     ServerName localhost
 *     ScriptAlias /album/ /path/to/balde-album/
 *     FcgidInitialEnv TITLE "My shiny new image gallery"
 *     FcgidInitialEnv IMAGES_DIRECTORY "/var/www/pics"
 *     <Directory /path/to>
 *         Order allow,deny
 *         Allow from all
 *         Options ExecCGI
 *         SetHandler fcgid-script
 *     </Directory>
 * </VirtualHost>
 *
 * The images directory and the images itself must be readable by the
 * webserver, obviously.
 *
 * Just open your browser and point to /album/, and voila. (if you did
 * everything correctly).
 *
 *
 * WARNINGS:
 *
 * - This thing just supports thumbnails embedded in the original image's
 *   EXIF metadata, no runtime generation is done.
 * - The image width detection just works with images that provides the
 *   relevant EXIF metadata, or JPEG images.
 * - To add new images you need to reload the FastCGI processes manually, no
 *   fancy reloaders at all.
 * - This is just a quick hack that comes without guarantees!
 *
 */


#include <math.h>
#include <glib.h>
#include <balde.h>
#include "balde-album.h"
#include "exif.h"
#include "loader.h"
#include "utils.h"


balde_app_t* ba_app_init(void);
void ba_app_free(balde_app_t *app);
void ba_tmpl_header(balde_app_t *app, balde_response_t *resp);
void ba_tmpl_footer(balde_app_t *app, balde_response_t *resp);
void ba_tmpl_image_table_cell(balde_app_t *app, balde_response_t *resp, ba_image_t* img);
void ba_tmpl_image_table(balde_app_t *app, balde_response_t *resp);
void ba_tmpl_image_detail(balde_app_t *app, balde_response_t *resp, ba_image_t *img);
balde_response_t* ba_view_index(balde_app_t *app, balde_request_t *req);
balde_response_t* ba_view_image(balde_app_t *app, balde_request_t *req);
balde_response_t* ba_view_full(balde_app_t *app, balde_request_t *req);
balde_response_t* ba_view_thumb(balde_app_t *app, balde_request_t *req);


/**
 * Miscelaneous functions
 */


balde_app_t*
ba_app_init(void)
{
    balde_app_t *app = balde_app_init();
    app->user_data = NULL;
    const gchar *dirpath = g_getenv("IMAGES_DIRECTORY");
    if (dirpath == NULL) {
        balde_abort_set_error_with_description(app, 500,
            "IMAGES_DIRECTORY environment variable must be set");
        goto point1;
    }
    balde_app_set_config(app, "IMAGES_DIRECTORY", dirpath);
    balde_app_set_config(app, "TITLE", g_getenv("TITLE"));
    app->user_data = g_new(ba_user_data_t, 1);
    ba_user_data_t *ud = app->user_data;
    ud->images = ba_load_images_directory(dirpath);
point1:
    return app;
}


void
ba_app_free(balde_app_t *app)
{
    ba_user_data_t *ud = app->user_data;
    if (ud != NULL)
        g_slist_free_full(ud->images, (GDestroyNotify) ba_free_image);
    g_free(ud);
    balde_app_free(app);
}


/**
 * Template functions
 *
 * Not using balde templates and static files support here.
 * Come on, this is a single file app!
 */

void
ba_tmpl_header(balde_app_t *app, balde_response_t *resp)
{
    const gchar* title = balde_app_get_config(app, "TITLE");
    gchar *dtitle = g_strdup(title != NULL ? title : "Untitled gallery");
    gchar *tmp = g_strdup_printf(
        "<!DOCTYPE html>\n"
        "<html>\n"
        "  <head>\n"
        "    <title>%s</title>\n"
        "    <style type=\"text/css\">\n"
        "      body { text-align: center; }\n"
        "      table { margin: 20px auto; }\n"
        "      #grid td { padding: 10px; }\n"
        "      th { padding-right: 20px; }\n"
        "    </style>\n"
        "  </head>\n"
        "  <body>\n"
        "    <h1>%s</h1>\n", dtitle, dtitle);
    g_free(dtitle);
    balde_response_append_body(resp, tmp);
    g_free(tmp);
}


void
ba_tmpl_footer(balde_app_t *app, balde_response_t *resp)
{
    balde_response_append_body(resp,
        "  </body>\n"
        "</html>\n");
}


void
ba_tmpl_image_table_cell(balde_app_t *app, balde_response_t *resp, ba_image_t* img)
{
    balde_response_append_body(resp, "        <td>\n");
    gchar *img_link = balde_app_url_for(app, "image", FALSE, img->filename);
    gchar *tmp;
    if (img->thumb != NULL) {
        gchar *thumb_link = balde_app_url_for(app, "thumb", FALSE, img->filename);
        tmp = g_strdup_printf(
            "          <a href=\"%s\">\n"
            "            <img src=\"%s\">\n"
            "          </a>\n"
            "          <br>\n", img_link, thumb_link);
        g_free(thumb_link);
        balde_response_append_body(resp, tmp);
        g_free(tmp);
    }
    gchar *tmp_filename = ba_shorten_filename(img->filename);
    tmp = g_strdup_printf(
        "          <a href=\"%s\">\n"
        "            %s\n"
        "          </a>\n"
        "        </td>\n", img_link, tmp_filename);
    g_free(tmp_filename);
    balde_response_append_body(resp, tmp);
    g_free(img_link);
    g_free(tmp);
}


void
ba_tmpl_image_table(balde_app_t *app, balde_response_t *resp)
{
    ba_user_data_t *ud = app->user_data;
    guint num_rows = ceilf((gfloat) g_slist_length(ud->images) / IMAGES_PER_ROW);
    GSList *tmp = ud->images;
    balde_response_append_body(resp,
        "    <table id=\"grid\" border=\"1\" cellspacing=\"0\">\n");
    for (guint i = 0; i < num_rows; i++) {
        balde_response_append_body(resp, "      <tr>\n");
        for (guint j = 0; j < IMAGES_PER_ROW; j++) {
            if (tmp != NULL) {
                ba_tmpl_image_table_cell(app, resp, tmp->data);
                tmp = g_slist_next(tmp);
            }
            else
                balde_response_append_body(resp, "        <td>&nbsp;</td>\n");
        }
        balde_response_append_body(resp, "      </tr>\n");
    }
    balde_response_append_body(resp, "    </table>\n");
}


void
ba_tmpl_image_detail(balde_app_t *app, balde_response_t *resp, ba_image_t *img)
{
    gchar *img_link = balde_app_url_for(app, "full", FALSE, img->filename);
    gchar *tmp;
    tmp = g_strdup_printf("    <h2>%s</h2>\n", img->filename);
    balde_response_append_body(resp, tmp);
    g_free(tmp);
    // if we know the image width, and it is too big, we should resize it.
    gint64 width = ba_get_image_width(img);
    if (width > MAX_IMAGE_WIDTH)
        tmp = g_strdup_printf(
            "    <img src=\"%s\" width=\"%d\">\n"
            "    <p>\n"
            "      <a href=\"%s\">Click here to see in original size.</a>\n"
            "    </p>\n", img_link, MAX_IMAGE_WIDTH, img_link);
    else
        tmp = g_strdup_printf("    <img src=\"%s\">\n", img_link);
    g_free(img_link);
    balde_response_append_body(resp, tmp);
    g_free(tmp);
    if (img->metadata == NULL)
        return;
    balde_response_append_body(resp,
        "    <hr>\n"
        "    <h3>EXIF metadata</h3>\n"
        "    <table>\n");
    for (GSList *tmp = img->metadata; tmp != NULL; tmp = g_slist_next(tmp)) {
        ba_image_metadata_t *meta = tmp->data;
        gchar *tmp_row = g_strdup_printf(
            "      <tr>\n"
            "        <th>%s</th>\n"
            "        <td>%s</td>\n"
            "      </tr>\n", meta->title, meta->value);
        balde_response_append_body(resp, tmp_row);
        g_free(tmp_row);
    }
    balde_response_append_body(resp,
        "    </table>\n");
}


/**
 * Views! \o/
 */

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
    if (img == NULL || img->image == NULL)
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
    if (img == NULL || img->image == NULL)
        return balde_abort(app, 404);
    balde_response_t *rv = balde_make_response_len(img->image->str, img->image->len);
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
    if (img == NULL || img->thumb == NULL)
        return balde_abort(app, 404);
    balde_response_t *rv = balde_make_response_len(img->thumb->str, img->thumb->len);
    if (img->mimetype != NULL)
        balde_response_set_header(rv, "Content-Type", img->mimetype);
    return rv;
}


/**
 * Main function
 */

int
main(int argc, char **argv)
{
    balde_app_t *app = ba_app_init();

    // views declaration
    balde_app_add_url_rule(app, "index", "/", BALDE_HTTP_GET, ba_view_index);
    balde_app_add_url_rule(app, "image", "/image/<filename>", BALDE_HTTP_GET,
        ba_view_image);
    balde_app_add_url_rule(app, "full", "/full/<filename>", BALDE_HTTP_GET,
        ba_view_full);
    balde_app_add_url_rule(app, "thumb", "/thumb/<filename>", BALDE_HTTP_GET,
        ba_view_thumb);

    balde_app_run(app);
    ba_app_free(app);

    return 0;
}
