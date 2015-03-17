This is a plugin for gentoo and pidgin users to update your status message with the package emerge is currently processing.

# Prerequisites #
  * having Gentoo installed
  * pidgin/libpurple
  * having a user in the _portage_ group, or at least, access to the emerge log file (usually /var/log/emerge.log)

# Compilation #
as non-root
```
# make
```

# Installation #
still as non-root
```
# make install
```

# Usage #
  1. load plugin
  1. change settings (refresh rate, emerge log file) and reload plugin if settings changed
  1. click your status -> Saved statuses...
> > select GENTOO then close

> If all goes well, your status should update to  "GENTOO building: pkgcategory/pkgname-version"
