#!/bin/sh

set -e

echo "Generating build information using autoconf"
echo "This may take a while ..."

cat acinclude/* >aclocal.m4
"${AUTOCONF:-autoconf}"

# Run configure for this platform
echo "Now you are ready to run ./configure"
