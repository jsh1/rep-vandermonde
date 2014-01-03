#!/bin/sh

if [ -f configure.in ]; then
  if grep "A[MC]_CONFIG_HEADER" configure.in >/dev/null; then
    if [ ! -f config.h.in ]; then
      echo "Running autoheader"
      autoheader || exit 1
    fi
  fi

  echo "Running libtoolize"
  libtoolize || exit 1

  echo "Running aclocal $ACLOCAL_FLAGS"
  aclocal $ACLOCAL_FLAGS || exit 1

  echo "Running autoconf $AUTOCONF_FLAGS"
  autoconf $AUTOCONF_FLAGS || exit 1
fi

./configure "$@" && ( echo ; echo "Now type \`make' to compile." )

