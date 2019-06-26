## ocfMail ldap PoC


### Building

Requries `libldap2-dev`, and assumes openldap is in root of the project
Install with `apt-get source openldap`, and follow openldap compilation instructions.

### Installing

Ensure that your schema has the desired virtual attribute (ocfMailVirt)

Run `make` and `sudo make install` to install the new module.

In `/etc/ldap/sladp.conf`, add the following lines:

```
modulepath /usr/lib/ldap
moduleload ocfemail.la
overlay ocfemail
```

### Testing

On `dev-firestorm` run `ldapsearch -x -H ldaps://dev-firestorm.ocf.berkeley.edu uid=<uid>`
