## ocf-ldap-overlay
[![Build Status](https://jenkins.ocf.berkeley.edu/buildStatus/icon?job=ldap-overlay/master)](https://jenkins.ocf.berkeley.edu/job/ldap-overlay/)

### Setup

Get dependencies and a version of the ldap source in the CWD
```
apt-get -y source slapd
apt-get -y build-dep slapd
```

Debian will pull down three files. Untar the version containing `orig` and
compile it.

### Installing

Ensure that your schema has the desired virtual attributes

Run `LDAP_SRC=./openldap-$version make` and `sudo make install` to install the
new module or install the package from the OCF internal `apt`.

In `/etc/ldap/slapd.conf`, add the following lines:

```
modulepath /usr/lib/ldap
moduleload ocfvirt.la
overlay ocfvirt
```

### Debugging

In this example, we'll set a breakpoint in `ocfvirt_response`.  In the
`Makefile` change `CC = gcc -g` so the shared library is compiled with debug
symbols.

```
$ LDAP_SRC=./openldap-2.4.47+dfsg make && sudo make install
# systemctl restart slapd
# gdb -p "$(pgrep slapd)"
# (gdb) b ocfvirt.c:103
```

In another session trigger the breakpoint with:
```
ldapsearch -x -H ldaps://dev-firestorm.ocf.berkeley.edu uid=keur
```

### Packaging

Run `make package`

All dependencies can be found in the `build-in-docker` script.
