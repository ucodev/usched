/**
 * @file mm.c
 * @brief uSched
 *        Memory Management interface
 *
 * Date: 12-08-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
 *
 * This file is part of usched.
 *
 * usched is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usched is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usched.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdlib.h>

#include "config.h"

#if CONFIG_USE_LIBFSMA == 1
 #include <fsma/fsma.h>
#endif

void *mm_alloc(size_t size) {
	return
#if CONFIG_USE_LIBFSMA == 1
	fsma_malloc(size);
#else
	malloc(size);
#endif
}

void mm_free(void *ptr) {
#if CONFIG_USE_LIBFSMA == 1
	fsma_free(ptr);
#else
	free(ptr);
#endif
}

void *mm_realloc(void *ptr, size_t size) {
	return
#if CONFIG_USE_LIBFSMA == 1
	fsma_realloc(ptr, size);
#else
	realloc(ptr, size);
#endif
}

void *mm_calloc(size_t nmemb, size_t size) {
	return
#if CONFIG_USE_LIBFSMA == 1
	fsma_calloc(nmemb, size);
#else
	calloc(nmemb, size);
#endif
}

