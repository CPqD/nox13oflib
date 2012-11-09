include ../../../Make.vars 

CONFIGURE_DEPENCIES = $(srcdir)/Makefile.am

EXTRA_DIST =\
	meta.json

pkglib_LTLIBRARIES =		\
	discovery.la \
	link_event.la

discovery_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/nox
discovery_la_SOURCES = discovery.cc
discovery_la_LDFLAGS = -module -export-dynamic

link_event_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/nox
link_event_la_SOURCES = link-event.cc
link_event_la_LDFLAGS = -module -export-dynamic


noinst_HEADERS =		\
    discovery.hh \
	link-event.hh

NOX_RUNTIMEFILES = meta.json	


all-local: nox-all-local
clean-local: nox-clean-local 
install-exec-hook: nox-install-local
