/**
 * @file UschedTest.java
 * @brief uSched JAVA Testing
 *        uSched JAVA Testing interface - Client
 *
 * Date: 12-01-2015
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

class UschedTest {
	public static void main(String[] args) {
		Usched usc = new Usched();

		usc.init();

		usc.test();

		usc.request("run '/bin/ls -lah /' in 10 seconds then every 5 seconds");

		long[] entry = usc.resultGetRun();

		System.out.println("Installed Entry ID: 0x" + Long.toHexString(entry[0]) + "\n");

		usc.resultFreeRun();

		usc.request("show all");

		UschedEntry[] entries = usc.resultGetShow();

		for (int i = 0; i < entries.length; i ++) {
			System.out.println("-----------------------------");
			System.out.println("Entry ID: 0x" + Long.toHexString(entries[i].id));
			System.out.println("Username: " + entries[i].username);
			System.out.println("UID: " + entries[i].uid);
			System.out.println("GID: " + entries[i].gid);
			System.out.println("Trigger: " + entries[i].trigger);
			System.out.println("Step: " + entries[i].step);
			System.out.println("Expire: " + entries[i].expire);
			System.out.println("Command: " + entries[i].cmd);
			System.out.println("-----------------------------\n");
		}

		usc.resultFreeShow();

		usc.destroy();
	}
}
