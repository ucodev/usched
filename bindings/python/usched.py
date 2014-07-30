#!/usr/bin/env python
#
# @file usched.py
# @brief uSched
#        uSched Python Library Module
#
# Date: 30-07-2014
# 
# Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
#
# This file is part of usched.
#
# usched is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# usched is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with usched.  If not, see <http://www.gnu.org/licenses/>.
#


from ctypes import *

# Load uSched Client Library
cdll.LoadLibrary("libusc.so")
libusc = CDLL("libusc.so")

# C to Python bindings
usched_init = libusc.usched_init
usched_request = libusc.usched_request
usched_result_get_run = libusc.usched_result_get_run
usched_result_get_stop = libusc.usched_result_get_stop
usched_result_get_show = libusc.usched_result_get_show
usched_result_free_run = libusc.usched_result_free_run
usched_result_free_stop = libusc.usched_result_free_stop
usched_result_free_show = libusc.usched_result_free_show
usched_usage_error = libusc.usched_usage_error
usched_usage_error_str = libusc.usched_usage_error_str
usched_destroy = libusc.usched_destroy

# TODO
# ......

# Example (TO BE REMOVED)
usched_init()
usched_request("run 'ls -lah /' in 10 seconds then every 5 seconds")
usched_result_free_run()
usched_destroy()

