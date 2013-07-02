##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = xdelayeddelivery_test

xdelayeddelivery_test_SOURCES = xdelayeddelivery_test.cpp
xdelayeddelivery_test_LDADD = ../../xdelayeddelivery.o ../../tag.o \
		../../jid.o ../../prep.o ../../gloox.o
xdelayeddelivery_test_CFLAGS = $(CPPFLAGS)
