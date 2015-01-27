<?php

/**
 * @file usched.php
 * @brief uSched PHP Library
 *        uSched PHP Library interface - Client
 *
 * Date: 27-01-2015
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


class Usched {
	public function SetHostname($hostname) {
		usc_opt_set_remote_hostname($hostname);
	}

	public function SetPort($port) {
		usc_opt_set_remote_port($port);
	}

	public function SetUsername($username) {
		usc_opt_set_remote_username($username);
	}

	public function SetPassword($password) {
		usc_opt_set_remote_password($password);
	}

	public function Request($request) {
		return usc_request($request);
	}

	public function ResultRun() {
		$entry_list = usc_result_get_run();

		usc_result_free_run();

		return $entry_list;
	}

	public function ResultStop() {
		$entry_list = usc_result_get_stop();

		usc_result_free_stop();

		return $entry_list;
	}

	public function ResultShow() {
		$entry_list = usc_result_get_show();

		usc_result_free_show();

		return $entry_list;
	}

	public function UsageError($error) {
		return usc_usage_error_str($error);
	}
}

