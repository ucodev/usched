#!/usr/bin/env python
#
# @file request_basic.py
# @brief uSched
#        uSched Basic Library Request Example
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


from usched import *

# Example
usched_init()
usched_request("run 'ls -lah /' in 10 seconds then every 5 seconds")
usched_result_free_run()
usched_destroy()

