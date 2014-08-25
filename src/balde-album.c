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


#include <glib.h>
#include <balde.h>
#include "balde-album.h"
#include "exif.h"
#include "loader.h"
#include "utils.h"
#include "template.h"
#include "views.h"


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
