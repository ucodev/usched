## 1. Summary

  A command-line scheduler with an almost-natural language interpreter.
  It also delivers a powerful programming interface for multiple programming languages.



## 2. Description

  uSched services provide an interface to schedule commands to be executed at a particular time, optionally repeating them over a specificied interval, and optionally stopping them at any other particular time.

  It provides a simple and intuitive structured language that is intepreted via a command line client, but can also be integrated into any programming language through its client libraries and bindings.

  It also operates as a client/server, where requests performed by clients can affect local or remote machines where uSched services are running.



## 3. Portability

  uSched is designed to be compliant with any POSIX operating system. There are some features that may not be enabled by default in the case that some non-portable calls being unavailable for the target operating system, such as chroot(). To disable such calls and features, set the CONFIG_POSIX_STRICT definition to 1 in the include/config.h file.

  uSched client will compile on most Windows versions and the API bindings are available for C#.



## 4. Installation

  Perform the following commands:

      ~# **cd /usr/src**
      ~# **mkdir usched**
      ~# **wget https://github.com/ucodev/usched/archive/master.tar.gz**
      ~# **tar zxvf master.tar.gz -C usched**
      ~# **cd usched**
      ~# **./deploy**

  or

      See INSTALL.txt



## 5. Command-Line Usage Examples

  Run the do_backups.sh script at 23:00 and then run it every 24 hours:

      ~$ **usc run '/usr/local/bin/do_backups.sh' on hour 23 then every 24 hours**


  Dump 'df -h' output into /tmp/disk_stats.log after 10 minutes of running this command, and run it again every 30 minutes:

      ~$ **usc run '/bin/df -h >> /tmp/disk_stats.log' in 10 minutes then every 30 minutes**


  Run the command 'sync' now, repeat every 45 seconds and stop when the time is 12:00:

      ~$ **usc run '/bin/sync' now then every 45 seconds until to time '12:00:00'**


  Show all scheduled entries for the user by running the following command:

      ~$ **usc show all**


  Stop all scheduled entries for the user by running the following command:

      ~$ **usc stop all**



## 6. Library Usage Examples

  See example/*



## 7. Documentation

  Reference Manual in HTML format:

      http://www.usched.org/doc/uSched_Reference_Manual.html

  Reference Manual in PDF format:

      http://www.usched.org/doc/uSched_Reference_Manual.pdf

  Overview manual page:

      usched(7)

  Command-line manual pages:

      usched(1), usa(1), usc(1), usd(1), use(1), usm(1)

  Programmer's manual pages:

      usched_destroy(3), usched_init(3),
      usched_opt_set_remote_hostname(3), usched_opt_set_remote_port(3)
      usched_opt_set_remote_username(3), usched_opt_set_remote_password(3),
      usched_request(3),
      usched_result_free_run(3),
      usched_result_free_show(3),
      usched_result_free_stop(3),
      usched_result_get_run(3),
      usched_result_get_show(3),
      usched_result_get_stop(3),
      usched_usage_error(3), usched_usage_error_str(3)

  Generate Doxygen HTML files (into doc/doxygen/doxyfiles/):

      $ make doxygen

  Brief installation guide:

      INSTALL.txt



## 8. Notes

  The current project revision is on an alpha stage and shall not be used beyond testing purposes.

