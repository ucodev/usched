/**
 * @file usched.cs
 * @brief uSched
 *        uSched C# Interface - Client
 *
 * Date: 18-03-2015
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

using System;
using System.Runtime.InteropServices;

namespace UschedAPI
{
	public struct UschedEntry {
		public UInt64 Id;
		public String Username;
		public UInt32 UID;
		public UInt32 GID;
		public UInt32 Trigger;
		public UInt32 Step;
		public UInt32 Expire;
		public String Command;
	}
	
	public class Usched {
		[StructLayout(LayoutKind.Sequential, Pack = 4)]
		private unsafe struct usched_entry {
			public ulong id;		/* 8 bytes */
			public uint flags;		/* 4 bytes */
			public uint uid;		/* 4 bytes */
			public uint gid;		/* 4 bytes */
			public uint trigger;		/* 4 bytes */
			public uint step;		/* 4 bytes */
			public uint expire;		/* 4 bytes */
			public uint psize;		/* 4 bytes */
			
			public fixed byte username[32];
			public fixed byte session[272];
			
			public void *payload;
			
			public uint subj_size;		/* 4 bytes */
			public void *subj;

			/* Resered union */
			public fixed byte reserved[32];

			/* Crypto structure */
			public fixed byte context[480];
			public fixed byte agreed_key[32];
			public ulong nonce;		/* 8 bytes */

			public uint create_time;

			public fixed byte signature[32];
		}

		[DllImport("libusc.dll", EntryPoint = "usched_init", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern int usched_init();

		[DllImport("libusc.dll", EntryPoint = "usched_request", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern int usched_request(String req);		

		[DllImport("libusc.dll", EntryPoint = "usched_opt_set_remote_hostname", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern int usched_opt_set_remote_hostname(String hostname);

		[DllImport("libusc.dll", EntryPoint = "usched_opt_set_remote_port", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern int usched_opt_set_remote_port(String port);

		[DllImport("libusc.dll", EntryPoint = "usched_opt_set_remote_username", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern int usched_opt_set_remote_username(String username);

		[DllImport("libusc.dll", EntryPoint = "usched_opt_set_remote_password", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern int usched_opt_set_remote_password(String password);

		[DllImport("libusc.dll", EntryPoint = "usched_result_get_run", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_result_get_run(ulong **entry_list, uint *nmemb);

		[DllImport("libusc.dll", EntryPoint = "usched_result_get_stop", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_result_get_stop(ulong **entry_list, uint *nmemb);

		[DllImport("libusc.dll", EntryPoint = "usched_result_get_show", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_result_get_show(usched_entry **entry_list, uint *nmemb);

		[DllImport("libusc.dll", EntryPoint = "usched_result_free_run", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_result_free_run();

		[DllImport("libusc.dll", EntryPoint = "usched_result_free_stop", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_result_free_stop();

		[DllImport("libusc.dll", EntryPoint = "usched_result_free_show", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_result_free_show();

		[DllImport("libusc.dll", EntryPoint = "usched_usage_error_str", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern string usched_usage_error_str(int error);

		[DllImport("libusc.dll", EntryPoint = "usched_destroy", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static unsafe extern void usched_destroy();
		
		public int Request(String request) {
			return usched_request(request);
		}
		
		public void SetHostname(String host) {
			usched_opt_set_remote_hostname(host);
		}
		
		public void SetPort(String port) {
			usched_opt_set_remote_port(port);
		}
		
		public void SetUsername(String username) {
			usched_opt_set_remote_username(username);
		}
		
		public void SetPassword(String password) {
			usched_opt_set_remote_password(password);
		}
		
		public UInt64[] ResultRun() {
			UInt64[] entryList;

			unsafe {
				ulong *entry_list;
				uint nmemb;
	
				usched_result_get_run(&entry_list, &nmemb);
				
				entryList = new UInt64[nmemb];
				
				for (int i = 0; i < nmemb; i ++) {
					entryList[i] = entry_list[i];
				}
				
				usched_result_free_run();
			}
			
			return entryList;
		}

		public UInt64[] ResultStop() {
			UInt64[] entryList;

			unsafe {
				ulong *entry_list;
				uint nmemb;
	
				usched_result_get_stop(&entry_list, &nmemb);
				
				entryList = new UInt64[nmemb];
				
				for (int i = 0; i < nmemb; i ++) {
					entryList[i] = entry_list[i];
				}
				
				usched_result_free_stop();
			}
			
			return entryList;
		}
		
		public UschedEntry[] ResultShow() {
			UschedEntry[] entryList;
			
			unsafe {
				usched_entry *entry_list;
				uint nmemb;
				
				usched_result_get_show(&entry_list, &nmemb);
				
				entryList = new UschedEntry[nmemb];

				for (int i = 0; i < nmemb; i ++) {
					entryList[i].Id = entry_list[i].id;
					entryList[i].Username = new string((sbyte *) entry_list[i].username);
					entryList[i].UID = entry_list[i].uid;
					entryList[i].GID = entry_list[i].gid;
					entryList[i].Trigger = entry_list[i].trigger;
					entryList[i].Step = entry_list[i].step;
					entryList[i].Expire = entry_list[i].expire;
					entryList[i].Command = new string((sbyte *) entry_list[i].subj);
				}
				
				usched_result_free_show();
			}
			
			return entryList;
		}

		public String UsageError(int error) {
			return usched_usage_error_str(error);
		}

		public Usched() {
			usched_init();
		}
		
		~Usched() {
			usched_destroy();
		}
	}
}

