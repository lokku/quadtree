/*
 * Copyright (C) 2011-2012 Lokku ltd. and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * quadtree_portable.h: Portably define things in co-operation with autoconf.
 */

#ifndef QUADTREE_PORTABLE_H
#define QUADTREE_PORTABLE_H

#include "config.h"

/**
 ** Determine how to print a uint64_t
 **/

/* If inttypes.h is available, PRIu64 is defined there. */
#ifdef HAVE_INTTYPES_H
#  ifndef __STDC_FORMAT_MACROS
#     define __STDC_FORMAT_MACROS
#  endif /* !__STDC_FORMAT_MACRS */
#  include <inttypes.h>
#else /* !HAVE_INTTYPES_H */

/* Get the word size */
#  if HAVE_BITS_WORDSIZE_H
#     include <bits/wordsize.h>
#  else /* !HAVE_BITS_WORDSIZE_H */
#     ifndef __WORDSIZE
#        define __WORDSIZE MACHINE_WORDSIZE
#     endif /* __WORDSIZE */
#  endif /* !HAVE_BITS_WORDSIZE_H */

/* No inttypes.h, make an educated guess. */
#  if __WORDSIZE == 64
#     ifndef PRIu64
#        define PRIu64 "lu"  /* Good guess for 64-bit */
#     endif /* PRIu64 */
#  else /* __WORDSIZE != 64 */
#     ifndef PRIu64
#        if defined(HAVE_UNSIGNED_LONG_LONG_INT) && defined(HAVE_PRINTF_LLU)
#           define PRIu64 "llu"
#        else /* !HAVE_PRINTF_LLU */
#           define PRIu64 "lu"
#        endif /* !HAVE_PRINTF_LLU */
#     endif /* !PRIu64 */
#  endif /* __WORDSIZE != 64 */
#endif /* !HAVE_INTTYPES_H */

/**
 ** Portably define bool (taken directly from the autoconf manual)
 **/

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else /* !HAVE_STDBOOL_H */
# ifndef HAVE__BOOL
#  ifdef __cplusplus
      typedef bool _Bool;
#  else /* !__cplusplus */
#   define _Bool signed char
#  endif /* !__cplusplus */
# endif /* HAVE__BOOL */
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif /* !HAVE_STDBOOL_H */

#endif /* QUADTREE_PORTABLE_H */

