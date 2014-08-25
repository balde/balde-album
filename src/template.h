/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_TEMPLATE_H
#define _BALDE_ALBUM_TEMPLATE_H

#include <balde.h>
#include "balde-album.h"

void ba_tmpl_header(balde_app_t *app, balde_response_t *resp);
void ba_tmpl_footer(balde_app_t *app, balde_response_t *resp);
void ba_tmpl_image_table_cell(balde_app_t *app, balde_response_t *resp, ba_image_t* img);
void ba_tmpl_image_table(balde_app_t *app, balde_response_t *resp);
void ba_tmpl_image_detail(balde_app_t *app, balde_response_t *resp, ba_image_t *img);

#endif /* _BALDE_ALBUM_TEMPLATE_H */
