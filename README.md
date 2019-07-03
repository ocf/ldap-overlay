## ocf-ldap-overlay
[![Build Status](https://jenkins.ocf.berkeley.edu/buildStatus/icon?job=ldap-overlay/master)](https://jenkins.ocf.berkeley.edu/job/ldap-overlay/)

### Building

Run `make package`

All dependencies can be found in the `build-in-docker` script.

### Installing

Ensure that your schema has the desired virtual attributes

Run `make` and `sudo make install` to install the new module or install the
package from the OCF internal `apt`.

In `/etc/ldap/sladp.conf`, add the following lines:

```
modulepath /usr/lib/ldap
moduleload ocfvirt.la
overlay ocfvirt
```

### Testing

On `dev-firestorm` run `ldapsearch -x -H ldaps://dev-firestorm.ocf.berkeley.edu
uid=<uid>`. Note that ldap loads all modules into memory when it starts, so you
will need to restart ldap if you make changes to your module.
