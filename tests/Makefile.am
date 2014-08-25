check_PROGRAMS = \
	check_foo

TESTS = \
	$(check_PROGRAMS)

check_foo_SOURCES = \
	check_foo.c

AM_CFLAGS = \
	$(all_includes) \
	$(BALDE_CFLAGS) \
	$(LIBEXIF_CFLAGS) \
	$(MAGICKWAND_CFLAGS)

AM_LDFLAGS = \
	$(BALDE_LIBS) \
	$(LIBEXIF_LIBS) \
	$(MAGICKWAND_LIBS)

clean-local:
	-rm -rf valgrind-*.xml

if USE_VALGRIND
valgrind: clean
	$(MAKE) check TESTS_ENVIRONMENT=" \
		G_SLICE=always-malloc \
		G_DEBUG=gc-friendly \
		$(LIBTOOL) \
			--mode=execute \
			$(VALGRIND) \
				--tool=memcheck \
				--xml=yes \
				--xml-file=valgrind-%p.xml \
				--leak-check=full \
				--leak-resolution=high \
				--num-callers=20 \
				--show-possibly-lost=no"
endif
