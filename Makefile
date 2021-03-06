LDAP_INC = -I$(LDAP_SRC)/include -I$(LDAP_SRC)/include -I$(LDAP_SRC)/servers/slapd
LDAP_LIB = $(LDAP_SRC)/libraries/libldap_r/libldap_r.la \
	$(LDAP_SRC)/libraries/liblber/liblber.la

LIBTOOL = libtool
CC = gcc
OPT = -O2 -Wall -Wno-discarded-qualifiers -Wno-format-extra-args -pedantic -c
DEFS =
INCS = $(LDAP_INC)
LIBS = $(LDAP_LIB)

PROGRAMS = ocfvirt.la
LTVER = 0:0:0

prefix=/usr
moduledir = $(prefix)/lib/ldap

.SUFFIXES: .c .o .lo

.c.lo: ocfvirt.c
	$(LIBTOOL) --mode=compile $(CC) $(OPT) $(DEFS) $(INCS) -c $<

all: $(PROGRAMS)

ocfvirt.la: ocfvirt.lo
	$(LIBTOOL) --mode=link $(CC) $(OPT) -version-info $(LTVER) \
	-rpath $(moduledir) -module -o $@ $? $(LIBS)

clean:
	rm -rf *.o *.lo *.la .libs debian/*.debhelper debian/*.log dist dist_* venv

install: ocfvirt.la
	mkdir -p $(DESTDIR)$(moduledir)
	for p in $(PROGRAMS) ; do \
		$(LIBTOOL) --mode=install cp $$p $(DESTDIR)$(moduledir) ; \
	done

dist:
	mkdir -p "$@"

.PHONY: test
test: venv
	venv/bin/pre-commit install
	venv/bin/pre-commit run --all-files

venv: Makefile
	vendor/venv-update \
		venv= $@ -ppython3 \
		install= 'pre-commit>=1.0.0'

.PHONY: package_%
package_%: dist
	docker run -e "DIST_UID=$(shell id -u)" -e "DIST_GID=$(shell id -g)" -v $(CURDIR):/mnt:rw "docker.ocf.berkeley.edu/theocf/debian:$*" /mnt/build-in-docker "$*"

.PHONY: package
package: package_stretch package_buster

.PHONY: autoversion
autoversion:
	date +%Y.%m.%d.%H.%M-git`git rev-list -n1 HEAD | cut -b1-8` > .version
	rm -f debian/changelog
	DEBFULLNAME="Open Computing Facility" DEBEMAIL="help@ocf.berkeley.edu" VISUAL=true \
		dch -v `sed s/-/+/g .version` -D stable --no-force-save-on-release \
		--create --package "ocf-ldap-overlay" "Package for Debian."

.PHONY: builddeb
builddeb: autoversion
	dpkg-buildpackage -us -uc
