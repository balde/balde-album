/**
 * balde-album: Yet another web gallery.
 * Copyright (C) 2014 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the LGPL-2 License.
 * See the file COPYING.
 */

#ifndef _BALDE_ALBUM_EXIF_H
#define _BALDE_ALBUM_EXIF_H

#include <glib.h>
#include <libexif/exif-data.h>

void ba_dump_exif_entry(ExifEntry *entry, GSList **metadata);
GSList* ba_dump_exif(GString *in);

#endif /* _BALDE_ALBUM_EXIF_H */
