include ../Make.vars

lib_LTLIBRARIES = liboflib_exp.la

AM_CPPFLAGS += $(OPENFLOW_CPPFLAGS)
AM_CPPFLAGS += -I$(top_srcdir)/src/nox

liboflib_exp_la_SOURCES = \
	ofl-exp.c \
	ofl-exp.h \
	ofl-exp-nicira.c \
	ofl-exp-nicira.h \
	ofl-exp-openflow.c \
	ofl-exp-openflow.h

AM_CPPFLAGS += -xc++ -DOFL_LOG_VLOG_CC
