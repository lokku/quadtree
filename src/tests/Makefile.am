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
AM_CFLAGS = -Wall -W


##
## Test Programs
##

check_PROGRAMS = checkqt benchmark

# Checkqt (name can't clash with target 'check')
checkqt_SOURCES = check.c ../quadtree.h
checkqt_LDADD = ../libquadtree.la

# Benchmark binary
benchmark_SOURCES = benchmark.c ../quadtree.h
benchmark_LDADD = ../libquadtree.la

# Tests to actually run
TESTS = checkqt

##
## Help 
##

help:
	@cat $(top_srcdir)/docs/make-help.txt
