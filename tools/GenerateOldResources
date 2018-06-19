#!/usr/bin/perl -w

# This generates the old resources header to stdout from stdin.
#
# Moved from CMakeKists.cmake to here in order to avoid any crazy syntax
# problems with cmake.

perl -ne 'print "#include $1 $2\n" if /^MAKE_RESOURCE\((\w+), (\d+)\);/;'
