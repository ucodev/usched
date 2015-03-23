## Summary 

A command-line scheduler with an almost-natural language interpreter.
It also delivers a powerful programming interface for multiple programming languages.



## Description

uSched services provide an interface to schedule commands to be executed at a particular time, optionally repeating them over a specificied interval, and optionally stopping them at any other particular time.

It provides a simple and intuitive structured language that is intepreted via a command line client, but can also be integrated into any programming language through its client libraries and bindings.

It also operates as a client/server, where requests performed by clients can affect local or remote machines where uSched services are running.



## Portability

uSched is designed to be compliant with any POSIX operating system. There are some features that may not be enabled by default in the case that some non-portable calls being unavailable for the target operating system, such as chroot(). To disable such calls and features, set the CONFIG_POSIX_STRICT definition to 1 in the [include/config.h](https://github.com/ucodev/usched/blob/master/include/config.h) file.

uSched client will compile on most Windows versions and the API bindings are available for C#.



## Installation

Perform the following commands:

      ~# cd /usr/src
      ~# mkdir usched
      ~# wget https://github.com/ucodev/usched/archive/master.tar.gz
      ~# tar zxvf master.tar.gz -C usched
      ~# cd usched
      ~# ./deploy

or see [INSTALL.txt](https://github.com/ucodev/usched/blob/master/doc/text/INSTALL.txt)



## Command-Line Usage Examples

Run the do_backups.sh script at 23:00 and then run it every 24 hours:

      ~$ usc run '/usr/local/bin/do_backups.sh' on hour 23 then every 24 hours


Dump 'df -h' output into /tmp/disk_stats.log after 10 minutes of running this command, and run it again every 30 minutes:

      ~$ usc run '/bin/df -h >> /tmp/disk_stats.log' in 10 minutes then every 30 minutes


Run the command 'sync' now, repeat every 45 seconds and stop when the time is 12:00:

      ~$ usc run '/bin/sync' now then every 45 seconds until to time '12:00:00'


Send a some sort of notification in 30 minutes:

      ~$ usc run '/usr/local/bin/notify.sh something' in 30 minutes


Capture network traffic on interface eth0 between 8:00 and 18:00, every day:

      ~# usc run 'tcpdump -i eth0 -w /tmp/traffic_`date +%Y%m%d`.dump' on hour 8 every 1 day
      ~# usc run 'killall tcpdump' on hour 18 every 1 day


Show all scheduled entries for the user by running the following command:

      ~$ usc show all


Stop all scheduled entries for the user by running the following command:

      ~$ usc stop all



## Documentation

Reference Manual in HTML format:

  * [http://doc.usched.org/uSched_Reference_Manual.html](http://doc.usched.org/uSched_Reference_Manual.html)

Reference Manual in PDF format:

  * [http://doc.usched.org/uSched_Reference_Manual.pdf](http://doc.usched.org/uSched_Reference_Manual.pdf)

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

      ~$ make doxygen

Brief installation guide:

  * [INSTALL.txt](https://github.com/ucodev/usched/blob/master/doc/text/INSTALL.txt)



## Library Usage Examples

See [example/](https://github.com/ucodev/usched/tree/master/example) directory for library usage examples in C, C#, Java, PHP, Python and more yet to come. The following sections illustrate some code snippets for some of the supported programming languages.

### C

      #include <usched/lib.h>
      
      int main(void) {
          usched_init();
          
          usched_request("run \'df -h >> /tmp/disk.txt\' now then every 30 seconds");
          
          usched_result_free_run();
          
          usched_destroy();
          
          return 0;
      }

### Csharp

      using UschedAPI;

      namespace UschedExample {
          class Program {
              public static void Main(string[] args) {
                  Usched usc = new Usched();
                  
                  usc.Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds");
              }
          }
      }

### Java

      class UschedExample {
          public static void main(String[] args) {
              Usched usc = new Usched();
              
              usc.Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds");
          }
      }

### PHP

      include("usched.php");
      
      $usc = new Usched();
      
      $usc->Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds");

### Python

      from usched import *
      
      usc = Usched()
      
      usc.Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds")



## Notes

The current project revision is on an **beta stage** and shall not be used beyond testing purposes.

