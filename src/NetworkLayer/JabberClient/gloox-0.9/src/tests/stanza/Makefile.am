##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -g3 -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = stanza_test

stanza_test_SOURCES = stanza_test.cpp
stanza_test_LDADD = ../../tag.o ../../stanza.o ../../jid.o \
			../../prep.o ../../stanzaextensionfactory.o ../../oob.o ../../vcardupdate.o \
 			../../delayeddelivery.o ../../xdelayeddelivery.o ../../gpgsigned.o \
 			../../gpgencrypted.o ../../gloox.o
stanza_test_CFLAGS = $(CPPFLAGS)
