##
## Process this file with automake to produce Makefile.in
##

SUBDIRS = . tests

AM_CPPFLAGS = -Wall -pedantic -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith \
                -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual -ansi

lib_LTLIBRARIES = libgloox.la

libgloox_la_SOURCES = jid.cpp parser.cpp connection.cpp clientbase.cpp tag.cpp stanza.cpp logsink.cpp \
                        dns.cpp prep.cpp base64.cpp client.cpp component.cpp \
                        disco.cpp adhoc.cpp privatexml.cpp registration.cpp \
                        nonsaslauth.cpp rosteritem.cpp rostermanager.cpp privacyitem.cpp \
                        bookmarkstorage.cpp annotations.cpp privacymanager.cpp lastactivity.cpp \
                        flexoff.cpp dataform.cpp dataformfield.cpp dataformbase.cpp messagesession.cpp \
                        messageeventfilter.cpp chatstatefilter.cpp main.cpp compression.cpp \
                        inbandbytestream.cpp inbandbytestreammanager.cpp messagefilter.cpp vcard.cpp \
                        vcardmanager.cpp

libgloox_la_LDFLAGS = -version-info 4:0:0 -no-undefined -no-allow-shlib-undefined
libgloox_la_LIBADD =
libgloox_la_CFLAGS = $(CPPFLAGS) $(IKSEMEL_CFLAGS)
libgloox_la_LIBS = -liksemel

libglooxincludedir = $(includedir)/gloox
libglooxinclude_HEADERS = adhoccommandprovider.h      privacymanager.h        tag.h \
                            adhoc.h                   iqhandler.h             privatexml.h \
                            annotations.h             client.h                privatexmlhandler.h \
                            annotationshandler.h      component.h             registration.h \
                            bookmarkhandler.h         registrationhandler.h   jid.h \
                            bookmarkstorage.h         messagehandler.h        rosteritem.h \
                            clientbase.h              rosterlistener.h        taghandler.h \
                            connectionlistener.h      rostermanager.h         gloox.h\
                            disco.h                   presencehandler.h       subscriptionhandler.h \
                            discohandler.h            privacyitem.h           stanza.h\
                            disconodehandler.h        privacylisthandler.h    loghandler.h \
                            lastactivity.h            lastactivityhandler.h   flexoff.h \
                            flexoffhandler.h          dataform.h              dataformfield.h \
                            dataformitem.h            dataformbase.h          dataformreported.h \
                            macros.h                  logsink.h               messagesession.h \
                            messageeventhandler.h     messagesessionhandler.h chatstatehandler.h \
                            chatstatefilter.h         messageeventfilter.h    inbandbytestream.h \
                            inbandbytestreamhandler.h inbandbytestreamdatahandler.h \
                            inbandbytestreammanager.h messagefilter.h         vcard.h \
                            vcardmanager.h            vcardhandler.h

noinst_HEADERS = prep.h parser.h connection.h dns.h nonsaslauth.h base64.h compression.h
