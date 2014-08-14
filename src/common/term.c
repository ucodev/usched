/**
 * @file term.c
 * @brief uSched
 *        Terminal attributes control interface
 *
 * Date: 14-08-2014
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "term.h"

static int _term_flag_mode_input_op(unsigned int flag, unsigned int set) {
	struct termios tp;

	memset(&tp, 0, sizeof(struct termios));

	if (tcgetattr(STDIN_FILENO, &tp) < 0)
		return -1;

	if (set) {
		tp.c_iflag |= flag;
	} else {
		tp.c_iflag &= ~flag;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &tp) < 0)
		return -1;

	return 0;
}

static int _term_flag_mode_output_op(unsigned int flag, unsigned int set) {
	struct termios tp;

	memset(&tp, 0, sizeof(struct termios));

	if (tcgetattr(STDIN_FILENO, &tp) < 0)
		return -1;

	if (set) {
		tp.c_oflag |= flag;
	} else {
		tp.c_oflag &= ~flag;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &tp) < 0)
		return -1;

	return 0;
}

static int _term_flag_mode_control_op(unsigned int flag, unsigned int set) {
	struct termios tp;

	memset(&tp, 0, sizeof(struct termios));

	if (tcgetattr(STDIN_FILENO, &tp) < 0)
		return -1;

	if (set) {
		tp.c_cflag |= flag;
	} else {
		tp.c_cflag &= ~flag;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &tp) < 0)
		return -1;

	return 0;
}

static int _term_flag_mode_local_op(unsigned int flag, unsigned int set) {
	struct termios tp;

	memset(&tp, 0, sizeof(struct termios));

	if (tcgetattr(STDIN_FILENO, &tp) < 0)
		return -1;

	if (set) {
		tp.c_lflag |= flag;
	} else {
		tp.c_lflag &= ~flag;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &tp) < 0)
		return -1;

	return 0;
}

/* Input ops */
int term_input_igncr_set(void) {
	return _term_flag_mode_input_op(IGNCR, 1);
}

int term_input_igncr_unset(void) {
	return _term_flag_mode_input_op(IGNCR, 0);
}

int term_input_crtonl_set(void) {
	return _term_flag_mode_input_op(ICRNL, 1);
}

int term_input_crtonl_unset(void) {
	return _term_flag_mode_input_op(ICRNL, 0);
}

/* Output ops */
int term_output_nocr_set(void) {
	return _term_flag_mode_output_op(ONLRET, 1);
}

int term_output_nocr_unset(void) {
	return _term_flag_mode_output_op(ONLRET, 0);
}

int term_output_crtonl_set(void) {
	return _term_flag_mode_output_op(OCRNL, 1);
}

int term_output_crtonl_unset(void) {
	return _term_flag_mode_output_op(OCRNL, 0);
}

/* Control ops */
int term_ctrl_cread_set(void) {
	return _term_flag_mode_control_op(CREAD, 1);
}

int term_ctrl_cread_unset(void) {
	return _term_flag_mode_control_op(CREAD, 0);
}

/* Local ops */
int term_local_buffer_set(void) {
	return _term_flag_mode_local_op(ICANON, 1);
}

int term_local_buffer_unset(void) {
	return _term_flag_mode_local_op(ICANON, 0);
}

int term_local_echo_set(void) {
	return _term_flag_mode_local_op(ECHO, 1);
}

int term_local_echo_unset(void) {
	return _term_flag_mode_local_op(ECHO, 0);
}

int term_local_signals_set(void) {
	return _term_flag_mode_local_op(ISIG, 1);
}

int term_local_signals_unset(void) {
	return _term_flag_mode_local_op(ISIG, 0);
}

