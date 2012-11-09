include ../Make.vars

lib_LTLIBRARIES = libopenflow.la

AM_CPPFLAGS += $(OPENFLOW_CPPFLAGS)
AM_CPPFLAGS += -I$(top_srcdir)/src/nox

libopenflow_la_SOURCES = \
	compiler.h \
	dirs.c \
	dirs.h \
	dynamic-string.c \
	dynamic-string.h \
	fatal-signal.c \
	fatal-signal.h \
	fault.c \
	fault.h \
	hash.c \
	hash.h \
	hmap.c \
	hmap.h \
	ipv6_util.c \
	ipv6_util.h \
	list.c \
	list.h \
	ofpbuf.c \
	ofpbuf.h \
	packets.h \
	random.c \
	random.h \
	sat-math.h \
	tag.c \
	tag.h \
	timeval.c \
	timeval.h \
	type-props.h \
	util.c \
	util.h \
	vlog-modules.def \
	vlog.c \
	vlog.h 

AM_CPPFLAGS += -xc++ -DOFL_LOG_VLOG_CC


