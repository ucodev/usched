/**
 * @file JNIUsc.java
 * @brief uSched JAVA Library
 *        uSched JAVA Library interface - Client
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

public class JNIUsc {
	public native void nativeInit();
	public native void nativeDestroy();
	public native String nativeTest();
	public native boolean nativeOptSetRemoteHostname(String hostname);
	public native boolean nativeOptSetRemotePort(String port);
	public native boolean nativeOptSetRemoteUsername(String username);
	public native boolean nativeOptSetRemotePassword(String password);
	public native boolean nativeRequest(String request);

	static {
		System.loadLibrary("usc");
	}

	public void init() {
		nativeInit();
	}

	public void destroy() {
		nativeDestroy();
	}

	public void test() {
		String str = nativeTest();
		System.out.println(str);
	}

	public boolean setRemoteHostname(String hostname) {
		return nativeOptSetRemoteHostname(hostname);
	}

	public boolean setRemotePort(String port) {
		return nativeOptSetRemotePort(port);
	}

	public boolean setRemoteUsername(String username) {
		return nativeOptSetRemoteUsername(username);
	}

	public boolean setRemotePassword(String password) {
		return nativeOptSetRemotePassword(password);
	}

	public boolean request(String request) {
		return nativeRequest(request);
	}

	public static void main(String[] args) {
		JNIUsc usc = new JNIUsc();

		usc.init();

		usc.test();

		usc.request("run '/bin/ls -lah /' in 10 seconds then every 5 seconds");

		usc.destroy();
	}
}

