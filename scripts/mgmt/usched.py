#!/usr/bin/env python
#
# @file usched
# @brief uSched
#        uSched flush/start/stop script - Python implementation
#
# Date: 14-05-2015
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

import sys
import os
import signal
import subprocess
import time

# Config
CONFIG_USCHED_ADMIN_BIN = "@_SYSSBINDIR_@/usa"
CONFIG_USCHED_DAEMON_BIN = "@_SYSSBINDIR_@/usd"
CONFIG_USCHED_EXEC_BIN = "@_SYSSBINDIR_@/use"
CONFIG_USCHED_IPC_BIN = "@_SYSSBINDIR_@/usi"
CONFIG_USCHED_STAT_BIN = "@_SYSSBINDIR_@/uss"
CONFIG_USCHED_MONITOR_BIN = "@_SYSSBINDIR_@/usm"
CONFIG_USCHED_PREINIT_BIN = "@_SYSSBINDIR_@/usched_preinit"
CONFIG_USCHED_DAEMON_PID_FILE = "@_SYSRUNDIR_@/usched_usd.pid"
CONFIG_USCHED_EXEC_PID_FILE = "@_SYSRUNDIR_@/usched_use.pid"
CONFIG_USCHED_IPC_PID_FILE = "@_SYSRUNDIR_@/usched_usi.pid"
CONFIG_USCHED_STAT_PID_FILE = "@_SYSRUNDIR_@/usched_uss.pid"

# Operations
USCHED_OP_FLUSH = "flush"
USCHED_OP_RELOAD = "reload"
USCHED_OP_START = "start"
USCHED_OP_STOP = "stop"
USCHED_OP_FORCE_STOP = "force_stop"

# Options
BE_QUIET = 0

# Globals
EXIT_SUCCESS = 0
EXIT_FAILURE = 1

def print_info(msg):
	if not BE_QUIET:
		sys.stdout.write(msg)

def usage_print():
	print_info("Usage: %s %s|%s|%s|%s|%s [quiet]\n" % (sys.argv[0], USCHED_OP_FLUSH, USCHED_OP_RELOAD, USCHED_OP_START, USCHED_OP_STOP, USCHED_OP_FORCE_STOP))
	sys.exit(EXIT_FAILURE)

def usage_check():
	global BE_QUIET

	if len(sys.argv) < 2:
		usage_print()

	if len(sys.argv) == 3:
		if sys.argv[2] != "quiet":
			usage_print()
		else:
			BE_QUIET = 1

def perms_check():
	if os.getuid() != 0:
		print_info("You need to be root to perform that operation.\n")
		sys.exit(EXIT_FAILURE)

def op_check():
	if sys.argv[1] != USCHED_OP_FLUSH and sys.argv[1] != USCHED_OP_RELOAD and sys.argv[1] != USCHED_OP_START and sys.argv[1] != USCHED_OP_STOP and sys.argv[1] != USCHED_OP_FORCE_STOP:
		print_info("Invalid operation: " + sys.argv[1] + "\n")
		sys.exit(EXIT_FAILURE)

def op_process():
	if sys.argv[1] == USCHED_OP_FLUSH:
		return usched_flush()
	elif sys.argv[1] == USCHED_OP_RELOAD:
		return usched_reload()
	elif sys.argv[1] == USCHED_OP_START:
		return usched_start()
	elif sys.argv[1] == USCHED_OP_STOP:
		return usched_stop()
	elif sys.argv[1] == USCHED_OP_FORCE_STOP:
		return usched_force_stop()

	return False

def usched_flush():
	print_info("Flush operation status: ")

	# Signal usd
	if signal_pid_file(CONFIG_USCHED_DAEMON_PID_FILE, signal.SIGUSR1) == False:
		return False

	return True

def usched_reload():
	print_info("Reload operation status: ")

	# Signal usd
	if signal_pid_file(CONFIG_USCHED_DAEMON_PID_FILE, signal.SIGHUP) == False:
		return False

	return True

def usched_start():
	print_info("Start operation status: ")

	# Run pre initialization routines
	try:
		status = subprocess.call([CONFIG_USCHED_PREINIT_BIN])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Pre initialization routines failed.\n")
		return False

	# Commit uSched Core configuration changes
	try:
		status = subprocess.call([CONFIG_USCHED_ADMIN_BIN, "commit", "core"])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to commit uSched Core configuration.\n")
		return False

	# Commit uSched Exec configuration changes
	try:
		status = subprocess.call([CONFIG_USCHED_ADMIN_BIN, "commit", "exec"])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to commit uSched IPC configuration.\n")
		return False

	# Commit uSched IPC configuration changes
	try:
		status = subprocess.call([CONFIG_USCHED_ADMIN_BIN, "commit", "ipc"])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to commit uSched IPC configuration.\n")
		return False

	# Commit uSched Stat configuration changes
	try:
		status = subprocess.call([CONFIG_USCHED_ADMIN_BIN, "commit", "stat"])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to commit uSched Stat configuration.\n")
		return False

	# Start uSched IPC module
	try:
		status = subprocess.call([CONFIG_USCHED_MONITOR_BIN, "-p", CONFIG_USCHED_IPC_PID_FILE, "-r", "-S", CONFIG_USCHED_IPC_BIN])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to start uSched IPC Module\n")
		return False

	time.sleep(1)

	# Start uSched Status and Statistics module
	try:
		status = subprocess.call([CONFIG_USCHED_MONITOR_BIN, "-p", CONFIG_USCHED_STAT_PID_FILE, "-r", "-S", CONFIG_USCHED_STAT_BIN])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to start uSched Status and Statistics Module\n")
		return False

	time.sleep(1)

	# Start uSched Executer
	try:
		status = subprocess.call([CONFIG_USCHED_MONITOR_BIN, "-p", CONFIG_USCHED_EXEC_PID_FILE, "-r", "-S", CONFIG_USCHED_EXEC_BIN])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to start uSched Executer Module\n")
		return False

	time.sleep(1)

	# Wait for 'use' to start
	while not os.path.isfile(CONFIG_USCHED_EXEC_PID_FILE):
		time.sleep(1)

	# Start uSched Daemon
	try:
		status = subprocess.call([CONFIG_USCHED_MONITOR_BIN, "-p", CONFIG_USCHED_DAEMON_PID_FILE, "-r", "-S", CONFIG_USCHED_DAEMON_BIN])
	except OSError:
		status = 127

	if status != EXIT_SUCCESS:
		print_info("Failed: Unable to start uSched Daemon Module\n")
		return False

	time.sleep(1)

	# Wait for 'usd' to start
	while not os.path.isfile(CONFIG_USCHED_DAEMON_PID_FILE):
		time.sleep(1)

	return True

def usched_stop():
	print_info("Stop operation status: ")

	# Terminate usd
	if signal_pid_file(CONFIG_USCHED_DAEMON_PID_FILE, signal.SIGTERM) == False:
		return False

	# Wait for 'usd' to terminate
	while os.path.isfile(CONFIG_USCHED_DAEMON_PID_FILE):
		time.sleep(1)

	# Terminate use
	if signal_pid_file(CONFIG_USCHED_EXEC_PID_FILE, signal.SIGTERM) == False:
		return False

	# Wait for 'use' to terminate
	while os.path.isfile(CONFIG_USCHED_EXEC_PID_FILE):
		time.sleep(1)

	# Terminate uss
	if signal_pid_file(CONFIG_USCHED_STAT_PID_FILE, signal.SIGTERM) == False:
		return False

	# Wait for 'uss' to terminate
	while os.path.isfile(CONFIG_USCHED_STAT_PID_FILE):
		time.sleep(1)

	# Terminate usi
	if signal_pid_file(CONFIG_USCHED_IPC_PID_FILE, signal.SIGTERM) == False:
		return False

	# Wait for 'usi' to terminate
	while os.path.isfile(CONFIG_USCHED_IPC_PID_FILE):
		time.sleep(1)

	return True

def usched_force_stop():
	print_info("Force stop operation status: ")

	try:
		# Terminate usd
		signal_pid_file(CONFIG_USCHED_DAEMON_PID_FILE, signal.SIGKILL, True)

		# Terminate use
		signal_pid_file(CONFIG_USCHED_EXEC_PID_FILE, signal.SIGKILL, True)

		# Terminate uss
		signal_pid_file(CONFIG_USCHED_STAT_PID_FILE, signal.SIGKILL, True)

		# Terminate usi
		signal_pid_file(CONFIG_USCHED_IPC_PID_FILE, signal.SIGKILL, True)

		# Remove the daemon pid file
		os.unlink(CONFIG_USCHED_DAEMON_PID_FILE)

		# Remove the exec pid file
		os.unlink(CONFIG_USCHED_EXEC_PID_FILE)

		# Remove the stat pid file
		os.unlink(CONFIG_USCHED_STAT_PID_FILE)

		# Remove the ipc pid file
		os.unlink(CONFIG_USCHED_IPC_PID_FILE)
	except:
		pass

	return True

def signal_pid_file(pid_file, sig, quiet = False):
	try:
		with open(pid_file, "r") as fp:
			usd_pid = fp.readline().strip()

			try:
				os.kill(int(usd_pid), sig)
			except OSError, ex:
				if not quiet:
					print_info("Failed: %s\n" % ex[1])
				return False
	except IOError, ex:
		if not quiet:
			print_info("Failed: %s: '%s'\n" % (ex[1], pid_file))
		return False

	return True

# Main
if __name__ == "__main__":
	usage_check()

	perms_check()

	op_check()

	if op_process() == False:
		print_info("Failed.\n")
		sys.exit(EXIT_FAILURE)

	print_info("Success.\n")

	sys.exit(EXIT_SUCCESS)

