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

#include <math.h>
#include <balde.h>
#include <glib.h>
#include "balde-album.h"
#include "utils.h"


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
    gchar *tmp = g_strdup_printf(
            "    <h2>%s</h2>\n"
            "    <img src=\"%s\">\n"
            "    <p>\n"
            "      <a href=\"%s\">Click here to see in original size.</a>\n"
            "    </p>\n", img->filename, img_link, img_link);
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
