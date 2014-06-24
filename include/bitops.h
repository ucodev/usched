/**
 * @file bitops.h
 * @brief uSched
 *        Bit Operations interface header
 *
 * Date: 24-06-2014
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


#ifndef USCHED_BITOPS_H
#define USCHED_BITOPS_H

#include <stdint.h>

/* Prototypes */
void bit_set(uint32_t *dword, unsigned int n);
void bit_clear(uint32_t *dword, unsigned int n);
void bit_toggle(uint32_t *dword, unsigned int n);
unsigned int bit_test(const uint32_t *dword, unsigned int n);

#endif

