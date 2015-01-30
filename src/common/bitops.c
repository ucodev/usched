/**
 * @file bitops.c
 * @brief uSched
 *        Bit Operations interface
 *
 * Date: 30-01-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
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


#include <stdint.h>

void bit_set(volatile uint32_t *dword, unsigned int n) {
	*dword |= (1 << n);
}

void bit_clear(volatile uint32_t *dword, unsigned int n) {
	*dword &= ~(1 << n);
}

void bit_toggle(volatile uint32_t *dword, unsigned int n) {
	*dword ^= (1 << n);
}

unsigned int bit_test(const volatile uint32_t *dword, unsigned int n) {
	return (*dword & (1 << n));
}

