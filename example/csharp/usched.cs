/**
 * @file usched.cs
 * @brief uSched
 *        uSched C# Interface - Client
 *
 * Date: 25-01-2015
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
using Usched;

namespace UschedExample
{
	class Program
	{
		public static void Main(string[] args)
		{
			Usc req = new Usc();
			UschedEntry[] result;

			req.SetHostname("192.168.1.1");
			req.SetPort("7600");
			req.SetUsername("win32user");
			req.SetPassword("win32password");

			req.Request("show all");

			result = req.ResultShow();
			
			/* Print some results */
			for (int i = 0; i < result.Length; i ++) {
				Console.WriteLine("ID: " + result[i].Id.ToString("X") + ", Command: " + result[i].Command);
			}
		}
	}
}

