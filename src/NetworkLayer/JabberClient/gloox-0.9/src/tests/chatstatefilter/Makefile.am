##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = chatstatefilter_test

chatstatefilter_test_SOURCES = chatstatefilter_test.cpp
chatstatefilter_test_LDADD = ../../tag.o ../../stanza.o ../../stanzaextensionfactory.o ../../gpgsigned.o \
 				../../jid.o ../../prep.o ../../gloox.o ../../gpgencrypted.o ../../oob.o \
 				 ../../vcardupdate.o ../../delayeddelivery.o ../../xdelayeddelivery.o
chatstatefilter_test_CPPFLAGS = $(CPPFLAGS)
