#!/bin/sh

mkdir -p m4

aclocal -I m4 \
&& autoheader \
&& automake --add-missing --copy \
&& autoconf \
&& ./configure CFLAGS='-O0' $*

