include ../../../Make.vars 

pkglib_LTLIBRARIES =		\
	messenger_core.la \
	messenger.la \
	jsonmessenger.la

messenger_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/nox -I $(top_srcdir)/src/nox/coreapps/
messenger_la_SOURCES = messenger.cc messenger.hh
messenger_la_LDFLAGS = -module -export-dynamic

messenger_core_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/nox -I $(top_srcdir)/src/nox/coreapps/
messenger_core_la_SOURCES = messenger_core.cc messenger_core.hh
messenger_core_la_LDFLAGS = -module -export-dynamic

jsonmessenger_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/nox -I $(top_srcdir)/src/nox/coreapps/
jsonmessenger_la_SOURCES = jsonmessenger.hh jsonmessenger.cc
jsonmessenger_la_LDFLAGS = -module -export-dynamic

NOX_RUNTIMEFILES = meta.json \
	serverkey.pem \
	cacert.pem \
	servercert.pem \
	messenger.py

EXTRA_DIST = \
	serverkey.pem \
	cacert.pem \
	servercert.pem \
	meta.json \
	messenger.py

all-local:  nox-all-local
clean-local: nox-clean-local
install-exec-hook: nox-install-local
