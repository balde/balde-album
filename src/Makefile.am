bin_PROGRAMS = \
	balde-album

balde_album_SOURCES = \
	balde-album.c \
	exif.c \
	image.c \
	loader.c \
	utils.c \
	template.c \
	views.c

balde_album_CFLAGS = \
	$(AM_CFLAGS) \
	$(BALDE_CFLAGS) \
	$(LIBEXIF_CFLAGS) \
	$(MAGICKWAND_CFLAGS)

balde_album_LDFLAGS = \
	$(AM_LDADD) \
	$(BALDE_LIBS) \
	$(LIBEXIF_LIBS) \
	$(MAGICKWAND_LIBS)
