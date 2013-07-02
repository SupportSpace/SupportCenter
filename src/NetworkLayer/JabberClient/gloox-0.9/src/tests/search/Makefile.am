##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = search_test

search_test_SOURCES = search_test.cpp
search_test_LDADD = ../../stanza.o ../../jid.o ../../dataform.o ../../dataformbase.o \
 		../../dataformreported.o ../../dataformitem.o ../../dataformfield.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o ../../oob.o ../../vcardupdate.o \
		../../delayeddelivery.o ../../xdelayeddelivery.o ../../gpgsigned.o ../../gpgencrypted.o
search_test_CFLAGS = $(CPPFLAGS)
