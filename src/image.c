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
#include <wand/MagickWand.h>
#include "balde-album.h"
#include "utils.h"


GString*
ba_format_image(GString *image, gsize max_width, gsize max_height)
{
    GString *rv = NULL;
    MagickWand *magick_wand = NewMagickWand();
    if (MagickReadImageBlob(magick_wand, image->str, image->len) == MagickFalse)
        goto point1;

    // fix image orientation. we just rotate, flipping images isn't supported.
    // this basically rotates the image to top-left orientation.
    gdouble degrees = 0;
    switch (MagickGetImageOrientation(magick_wand)) {
        case LeftBottomOrientation:
            degrees += 90;
        case BottomRightOrientation:
            degrees += 90;
        case RightTopOrientation:
            degrees += 90;
    }

    PixelWand *background = NULL;

    // no need to rotate if angle is zero.
    if (degrees > 0) {
        background = NewPixelWand();
        PixelSetColor(background, "#000000");
        if (MagickRotateImage(magick_wand, background, degrees) == MagickFalse)
            goto point2;
        MagickSetImageOrientation(magick_wand, TopLeftOrientation);
    }

    // resize image, respecting maximum width and length.
    gsize width = MagickGetImageWidth(magick_wand);
    gsize height = MagickGetImageHeight(magick_wand);

    // no need to resize if smaller than maximum dimension.
    if (width > max_width || height > max_height) {
        gdouble orig_ratio = (gdouble) width / height;
        gdouble max_ratio = (gdouble) max_width / max_height;
        gsize dest_width, dest_height;
        if (max_ratio < orig_ratio) {
            dest_width = max_width;
            dest_height = dest_width / orig_ratio;
        }
        else {
            dest_height = max_height;
            dest_width = orig_ratio * dest_height;
        }
        if (MagickScaleImage(magick_wand, dest_width, dest_height) == MagickFalse)
            goto point2;
    }

    // Build output string.
    gsize length;
    gchar *str = MagickGetImageBlob(magick_wand, &length);
    rv = g_string_new_len(str, length);
    MagickRelinquishMemory(str);

point2:
    if (background != NULL)
        DestroyPixelWand(background);
    DestroyMagickWand(magick_wand);

point1:
    return rv;
}


GString*
ba_get_formatted_image(const gchar *filepath)
{
    GString *in = ba_open_image(filepath);
    GString *rv = ba_format_image(in, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    g_string_free(in, TRUE);
    return rv;
}


GString*
ba_get_formatted_thumb(const gchar *filepath)
{
    GString *in = ba_open_image(filepath);
    GString *rv = ba_format_image(in, MAX_THUMB_WIDTH, MAX_THUMB_HEIGHT);
    g_string_free(in, TRUE);
    return rv;
}


GString*
ba_get_formatted_full(const gchar *filepath)
{
    GString *in = ba_open_image(filepath);
    GString *rv = ba_format_image(in, -1, -1);
    g_string_free(in, TRUE);
    return rv;
}
