## $NetBSD: Makefile.am,v 1.10 2010/11/29 06:21:40 agc Exp $

AM_CFLAGS		= $(WARNCFLAGS) $(BOTAN_INCLUDES) $(JSON_CFLAGS)

AM_LDFLAGS              = $(BOTAN_LDFLAGS)

lib_LTLIBRARIES		= librnp.la

librnp_la_CPPFLAGS	= -I$(top_srcdir)/include

librnp_la_SOURCES	= \
	bn.c \
	bufgap.c \
	compress.c \
	create.c \
	crypto.c \
	dsa.c \
	elgamal.c \
	hash.c \
	key_store.c \
	key_store_pgp.c \
	key_store_ssh.c \
	misc.c \
	packet-parse.c \
	packet-print.c \
	packet-show.c \
	packet-key.c \
	pem.c \
	reader.c \
	rnp.c \
	rsa.c \
	s2k.c \
	signature.c \
	symmetric.c \
	validate.c \
	writer.c

librnp_la_LIBADD	= $(BOTAN_LIBS) $(JSON_LIBS)

dist_man_MANS		= librnp.3
