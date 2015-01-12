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
	private native void nativeInit();
	private native void nativeDestroy();
	private native String nativeTest();
	private native boolean nativeOptSetRemoteHostname(String hostname);
	private native boolean nativeOptSetRemotePort(String port);
	private native boolean nativeOptSetRemoteUsername(String username);
	private native boolean nativeOptSetRemotePassword(String password);
	private native boolean nativeRequest(String request);
	private native long[] nativeResultGetRun();
	private native long[] nativeResultGetStop();
	private native long[] nativeResultGetShow();
	private native void nativeResultFreeRun();
	private native void nativeResultFreeStop();
	private native void nativeResultFreeShow();
	private native String nativeUsageErrorStr(int error);

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

	public long[] resultGetRun() {
		return nativeResultGetRun();
	}

	public long[] resultGetStop() {
		return nativeResultGetStop();
	}

	public long[] resultGetShow() {
		return nativeResultGetShow();
	}

	public void resultFreeRun() {
		nativeResultFreeRun();
	}

	public void resultFreeStop() {
		nativeResultFreeStop();
	}

	public void resultFreeShow() {
		nativeResultFreeShow();
	}

	public String usageErrorStr(int error) {
		return nativeUsageErrorStr(error);
	}

	public static void main(String[] args) {
		JNIUsc usc = new JNIUsc();

		usc.init();

		usc.test();

		usc.request("run '/bin/ls -lah /' in 10 seconds then every 5 seconds");

		usc.destroy();
	}
}

