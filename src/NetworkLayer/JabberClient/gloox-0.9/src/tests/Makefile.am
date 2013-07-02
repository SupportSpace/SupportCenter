##
## Process this file with automake to produce Makefile.in
##

SUBDIRS = base64 chatstatefilter client clientbase dataform dataformfield delayeddelivery gpgencrypted \
	  gpgsigned jid md5 messageeventfilter oob parser prep search sha simanager stanza \
	  stanzaextensionfactory tag tlsgnutls vcardupdate xdelayeddelivery xpath zlib

check: test

test: ${SUBDIRS}
	@for i in $?; do \
	  ${MAKE} -C $$i all; \
	done;
	@for i in $?; do \
	  $$i/$${i}_test;    \
	done;

leaktest: leakcheck

leakcheck: ${SUBDIRS}
	@for i in $?; do \
	  ${MAKE} -C $$i all; \
	done;
	@for i in $?; do \
	  valgrind -q --leak-check=full --show-reachable=yes $$i/$${i}_test;    \
	done;
