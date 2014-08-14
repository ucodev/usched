/**
 * @file term.h
 * @brief uSched
 *        Terminal attributes control interface header
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


#ifndef USCHED_TERM_H
#define USCHED_TERM_H

/* Prototypes */
int term_input_igncr_set(void);
int term_input_igncr_unset(void);
int term_input_crtonl_set(void);
int term_input_crtonl_unset(void);
int term_output_nocr_set(void);
int term_output_nocr_unset(void);
int term_output_crtonl_set(void);
int term_output_crtonl_unset(void);
int term_ctrl_cread_set(void);
int term_ctrl_cread_unset(void);
int term_local_buffer_set(void);
int term_local_buffer_unset(void);
int term_local_echo_set(void);
int term_local_echo_unset(void);
int term_local_signals_set(void);
int term_local_signals_unset(void);

#endif

