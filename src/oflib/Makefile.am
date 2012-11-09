include ../Make.vars

lib_LTLIBRARIES = liboflib.la

AM_CPPFLAGS += $(OPENFLOW_CPPFLAGS)
AM_CPPFLAGS += -I$(top_srcdir)/src/nox

liboflib_la_SOURCES = \
	ofl.h \
	ofl-actions.c \
	ofl-actions.h \
	ofl-actions-pack.c \
	ofl-actions-print.c \
	ofl-actions-unpack.c \
	ofl-messages.c \
	ofl-messages.h \
	ofl-messages-pack.c \
	ofl-messages-print.c \
	ofl-messages-unpack.c \
	oxm-match.c \
	oxm-match.h \
	ofl-print.c \
	ofl-print.h \
	ofl-structs.c \
	ofl-structs.h \
	ofl-structs-match.c \
	ofl-structs-pack.c \
	ofl-structs-print.c \
	ofl-structs-unpack.c \
	ofl-utils.h

liboflib_la_LIBADD = ../libopenflow/libopenflow.la

AM_CPPFLAGS += -xc++ -DOFL_LOG_VLOG_CC

 
