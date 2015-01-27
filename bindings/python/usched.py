#!/usr/bin/env python
#
# @file usched.py
# @brief uSched
#        uSched Python Library Module
#
# Date: 27-01-2015
# 
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
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
from ctypes.util import find_library

# Globals
LIBRARY_USC_NAME = "usc"

# Raw uSched Entry type
class UschedEntryRaw(Structure):
	_pack_   = 4
	_fields_ = [
		("id",		c_uint64),
		("flags",	c_uint32),
		("uid",		c_uint32),
		("gid",		c_uint32),
		("trigger",	c_uint32),
		("step",	c_uint32),
		("expire",	c_uint32),
		("psize",	c_uint32),
		("username",	c_char * 32),
		("session",	c_char * 272),
		("payload",	c_char_p),
		("subj_size",	c_uint32),
		("subj",	c_char_p),
		("reserved",	c_char * 32),
		("context",	c_char * 480),
		("agreed_key",	c_char * 32),
		("nonce",	c_uint64)
	]

# High level class
class Usched:
	def __init__(self):
		# Load uSched Client Library
		cdll.LoadLibrary(find_library(LIBRARY_USC_NAME))
		self._libusc = CDLL(find_library(LIBRARY_USC_NAME))

		# C to Python bindings
		self._usched_init = self._libusc.usched_init

		self._usched_request = self._libusc.usched_request
		self._usched_request.argtypes = [ c_char_p ]

		self._usched_opt_set_remote_hostname = self._libusc.usched_opt_set_remote_hostname
		self._usched_opt_set_remote_hostname.argtypes = [ c_char_p ]

		self._usched_opt_set_remote_port = self._libusc.usched_opt_set_remote_port
		self._usched_opt_set_remote_port.argtypes = [ c_char_p ]

		self._usched_opt_set_remote_username = self._libusc.usched_opt_set_remote_username
		self._usched_opt_set_remote_username.argtypes = [ c_char_p ]

		self._usched_opt_set_remote_password = self._libusc.usched_opt_set_remote_password
		self._usched_opt_set_remote_password.argtypes = [ c_char_p ]

		self._usched_result_get_run = self._libusc.usched_result_get_run
		self._usched_result_get_run.argtypes = [ POINTER(POINTER(c_uint64)), POINTER(c_uint32) ]

		self._usched_result_get_stop = self._libusc.usched_result_get_stop
		self._usched_result_get_stop.argtypes = [ POINTER(POINTER(c_uint64)), POINTER(c_uint32) ]

		self._usched_result_get_show = self._libusc.usched_result_get_show
		self._usched_result_get_show.argtypes = [ POINTER(POINTER(UschedEntryRaw)), POINTER(c_uint32) ]

		self._usched_result_free_run = self._libusc.usched_result_free_run

		self._usched_result_free_stop = self._libusc.usched_result_free_stop

		self._usched_result_free_show = self._libusc.usched_result_free_show

		self._usched_usage_error = self._libusc.usched_usage_error

		self._usched_usage_error_str = self._libusc.usched_usage_error_str
		self._usched_usage_error_str.argtypes = [ c_char_p ]

		self._usched_destroy = self._libusc.usched_destroy

		# Initialize uSched client engine
		self._usched_init()

	def __enter__(self):
		return self

	def __exit__(self, type, value, traceback):
		self._usched_destroy()

	def SetHostname(self, hostname):
		return self._usched_opt_set_remote_hostname(c_char_p(hostname))

	def SetPort(self, port):
		return self._usched_opt_set_remote_port(c_char_p(port))

	def SetUsername(self, username):
		return self._usched_opt_set_remote_username(c_char_p(username))

	def SetPassword(self, password):
		return self._usched_opt_set_remote_password(c_char_p(password))

	def Request(self, request):
		return self._usched_request(c_char_p(request))

	def ResultRun(self):
		entry_list = []
		nmemb = c_uint32(0)
		ids = POINTER(c_uint64)()

		self._usched_result_get_run(byref(ids), byref(nmemb))

		i = 0
		while i < nmemb.value:
			entry_list.append(ids[i])
			i += 1

		self._usched_result_free_run()

		return entry_list

	def ResultStop(self):
		entry_list = []
		nmemb = c_uint32(0)
		ids = POINTER(c_uint64)()

		self._usched_result_get_stop(byref(ids), byref(nmemb))

		i = 0
		while i < nmemb.value:
			entry_list.append(ids[i])
			i += 1

		self._usched_result_free_stop()

		return False

	def ResultShow(self):
		entry_list = []
		nmemb = c_uint32(0)
		entries = POINTER(UschedEntryRaw)()

		self._usched_result_get_show(byref(entries), byref(nmemb))

		i = 0
		while i < nmemb.value:
			entry = {}

			entry["id"] = entries[i].id
			entry["username"] = entries[i].username
			entry["uid"] = entries[i].uid
			entry["gid"] = entries[i].gid
			entry["trigger"] = entries[i].trigger
			entry["step"] = entries[i].step
			entry["expire"] = entries[i].expire
			entry["command"] = entries[i].subj

			entry_list.append(entry)

			i += 1

		self._usched_result_free_show()

		return entry_list

	def UsageError(self, error):
		return self._usched_usage_error_str(error)

