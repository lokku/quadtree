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

SUBDIRS = src docs
ACLOCAL_AMFLAGS = -I build/m4

EXTRA_DIST = README.maintainer

dist-local:
	@echo "----------------------------------------------------------"
	@echo "Make sure to run 'make distcheck', and confirm that there"
	@echo "are no errrors, before distributing this tarball."
	@echo "   >> Refer to README.maintainer for more information <<"
	@echo "----------------------------------------------------------"

help:
	@cat docs/make-help.txt

# Clear up test files
check-local:
	@rm -f _check.dat _benchmark.dat 2>/dev/null
