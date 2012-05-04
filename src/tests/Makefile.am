# Copyright (C) 2011-2012 Lokku ltd. and contributors
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

##
## CFLAGS
##

# Don't change this! Adding anything but warning flags may break CFLAGS
# customization for people not using gcc.
AM_CFLAGS = -Wall -W -I$(top_srcdir)/src/


##
## Test Programs
##

check_PROGRAMS = checkqt benchmark same-bucket stress-test
test_headers  = ../quadtree.h ../quadtree_portable.h ../quadtree_private.h


## Tests to run by default in 'make check'
TESTS = checkqt same-bucket

# Checkqt (name can't clash with target 'check')
checkqt_SOURCES = check.c $(test_headers)
checkqt_LDADD = ../libquadtree.la

# Lots of items in the same bucket
same_bucket_SOURCES = same-bucket.c $(test_headers)
same_bucket_LDADD = ../libquadtree.la


## Tests not run by default (with reasons)

# Benchmark binary (used to compare to perl performance, not a test)
benchmark_SOURCES = benchmark.c $(test_headers)
benchmark_LDADD = ../libquadtree.la

# Stress test (requires >4GB of free RAM, so not enabled by default)
stress_test_SOURCES = stress-test.c $(test_headers)
stress_test_LDADD = ../libquadtree.la

##
## Help 
##

help:
	@cat $(top_srcdir)/docs/make-help.txt
