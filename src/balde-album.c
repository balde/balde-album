/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
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


#include <stdio.h>
#include <math.h>
#include <glib.h>
#include <gio/gio.h>
#include <libexif/exif-data.h>
#include <jpeglib.h>
#include <balde.h>


/**
 * Constants
 *
 * Defining a few constants to avoid magic numbers.
 */

#define BUFFER_SIZE 1024
#define IMAGES_PER_ROW 4
#define MAX_FILENAME_LENGTH 32
#define MAX_IMAGE_WIDTH 900


/**
 * Structure and functions declarations.
 *
 * This is a single file app, then I'll just drop any declarations here,
 * instead of creating a header file :P
 */

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

void ba_dump_exif_entry(ExifEntry *entry, GSList **metadata);
ba_image_t* ba_load_image_file(const gchar *filepath);
gint ba_sort_images_by_filename(ba_image_t *a, ba_image_t *b);
GSList* ba_load_images_directory(const gchar *dirpath);
void ba_free_image_metadata(ba_image_metadata_t *meta);
void ba_free_image(ba_image_t *img);
balde_app_t* ba_app_init(void);
void ba_app_free(balde_app_t *app);
ba_image_t* ba_get_image_from_filename(balde_app_t *app, const gchar *filename);
gint64 ba_get_image_width(ba_image_t *img);
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

void
ba_dump_exif_entry(ExifEntry *entry, GSList **metadata)
{
    if (entry == NULL)
        return;
    ba_image_metadata_t *m = g_new(ba_image_metadata_t, 1);
    m->name = g_strdup(exif_tag_get_name_in_ifd(entry->tag,
        exif_content_get_ifd(entry->parent)));
    m->title = g_strdup(exif_tag_get_title_in_ifd(entry->tag,
        exif_content_get_ifd(entry->parent)));
    gchar buf[BUFFER_SIZE];
    exif_entry_get_value(entry, buf, BUFFER_SIZE);
    m->value = g_strdup(buf);
    *metadata = g_slist_append(*metadata, m);
}


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
    ExifData *ed = exif_data_new_from_file(filepath);
    ba_image_t *img = g_new(ba_image_t, 1);
    img->filepath = g_strdup(filepath);
    img->filename = g_path_get_basename(filepath);
    img->image = g_string_new_len(contents, length);
    g_free(contents);
    img->thumb = NULL;
    img->metadata = NULL;
    if (ed != NULL) {
        if (ed->data != NULL && ed->size > 0)
            img->thumb = g_string_new_len(ed->data, ed->size);
        exif_content_foreach_entry(ed->ifd[EXIF_IFD_0],
            (ExifContentForeachEntryFunc) ba_dump_exif_entry, &(img->metadata));
        exif_content_foreach_entry(ed->ifd[EXIF_IFD_EXIF],
            (ExifContentForeachEntryFunc) ba_dump_exif_entry, &(img->metadata));
        exif_data_unref(ed);
    }
    return img;
}


gint
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
