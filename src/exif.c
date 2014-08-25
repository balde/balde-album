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
#include <libexif/exif-data.h>
#include "balde-album.h"
#include "exif.h"


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


GSList*
ba_dump_exif(GString *in)
{
    ExifData *exif = exif_data_new_from_data(in->str, in->len);
    if (exif == NULL)
        return NULL;
    GSList *exif_l = NULL;
    exif_content_foreach_entry(exif->ifd[EXIF_IFD_0],
        (ExifContentForeachEntryFunc) ba_dump_exif_entry, &exif_l);
    exif_content_foreach_entry(exif->ifd[EXIF_IFD_EXIF],
        (ExifContentForeachEntryFunc) ba_dump_exif_entry, &exif_l);
    exif_data_unref(exif);
    return exif_l;
}
