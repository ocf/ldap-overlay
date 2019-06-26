LDAP_SRC = openldap-2.4.44+dfsg
LDAP_BUILD = $(LDAP_SRC)
LDAP_INC = -I$(LDAP_BUILD)/include -I$(LDAP_SRC)/include -I$(LDAP_SRC)/servers/slapd
LDAP_LIB = $(LDAP_BUILD)/libraries/libldap_r/libldap_r.la \
	$(LDAP_BUILD)/libraries/liblber/liblber.la

LIBTOOL = /usr/bin/libtool
CC = gcc
OPT = -O2 -Wall -Wno-discarded-qualifiers -Wno-format-extra-args -pedantic -c
DEFS =
INCS = $(LDAP_INC)
LIBS = $(LDAP_LIB)

PROGRAMS = ocfemail.la
LTVER = 0:0:0

prefix=/usr/local
exec_prefix=$(prefix)
ldap_subdir=/openldap

libdir=$(exec_prefix)/lib
libexecdir=$(exec_prefix)/libexec
#moduledir = $(libexecdir)$(ldap_subdir)
DESTDIR=/usr/lib
moduledir = /usr/lib/ldap

.SUFFIXES: .c .o .lo

.c.lo: ocfemail.c
	$(LIBTOOL) --mode=compile $(CC) $(OPT) $(DEFS) $(INCS) -c $<

all: $(PROGRAMS)

ocfemail.la: ocfemail.lo
	$(LIBTOOL) --mode=link $(CC) $(OPT) -version-info $(LTVER) \
	-rpath $(moduledir) -module -o $@ $? $(LIBS)

clean:
	rm -rf *.o *.lo *.la .libs

install: ocfemail.la
	#mkdir -p $(DESTDIR)$(moduledir)
	mkdir -p /usr/lib/ldap
	for p in $(PROGRAMS) ; do \
		$(LIBTOOL) --mode=install cp $$p /usr/lib/ldap ; \
	done
