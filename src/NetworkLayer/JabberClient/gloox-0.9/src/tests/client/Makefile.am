##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

noinst_PROGRAMS = client_test

client_test_SOURCES = client_test.cpp
client_test_LDADD = ../../client.o ../../clientbase.o ../../connectiontcpbase.o ../../connectiontcpclient.o \
			../../disco.o ../../parser.o ../../tag.o ../../stanza.o ../../base64.o ../../jid.o \
			../../md5.o ../../tlsgnutlsclient.o ../../tlsopenssl.o ../../tlsschannel.o \
			../../logsink.o ../../messagesession.o ../../prep.o ../../compressionzlib.o \
			../../dns.o ../../stanzaextensionfactory.o ../../oob.o ../../gpgencrypted.o \
			../../gpgsigned.o ../../vcardupdate.o ../../delayeddelivery.o \
			../../xdelayeddelivery.o ../../rostermanager.o ../../nonsaslauth.o ../../sha.o \
			../../rosteritem.o ../../privatexml.o ../../gloox.o ../../tlsgnutlsbase.o \
			../../tlsdefault.o ../../tlsgnutlsclientanon.o ../../tlsgnutlsserveranon.o \
			../../mutex.o
client_test_CFLAGS = $(CPPFLAGS)
