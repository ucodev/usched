/**
 * @file marshal.h
 * @brief uSched
 *        Serialization / Unserialization interface header
 *
 * Date: 07-07-2014
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


#ifndef USCHED_MARSHAL_H
#define USCHED_MARSHAL_H

/* Prototypes */
int marshal_daemon_init(void);
int marshal_daemon_serialize_pools(void);
int marshal_daemon_unserialize_pools(void);
void marshal_daemon_wipe(void);
void marshal_daemon_destroy(void);

#endif

