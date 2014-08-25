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
    balde_app_add_url_rule(app, "resized", "/resized/<filename>", BALDE_HTTP_GET,
        ba_view_resized);

    balde_app_run(app);
    ba_app_free(app);

    return 0;
}
