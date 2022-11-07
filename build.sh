#!/bin/sh

set -e
cd "$(dirname "$0")"

aclocal
autoconf
autoheader
automake --add-missing
CXXFLAGS="-O2 -std=gnu++11 -g -Wall -pedantic" ./configure "$@"
make -j4
