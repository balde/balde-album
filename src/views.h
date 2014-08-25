/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_VIEWS_H
#define _BALDE_ALBUM_VIEWS_H

#include <balde.h>

balde_response_t* ba_view_index(balde_app_t *app, balde_request_t *req);
balde_response_t* ba_view_image(balde_app_t *app, balde_request_t *req);
balde_response_t* ba_view_full(balde_app_t *app, balde_request_t *req);
balde_response_t* ba_view_thumb(balde_app_t *app, balde_request_t *req);

#endif /* _BALDE_ALBUM_VIEWS_H */
