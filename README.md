## Summary 

A free and open source command-line scheduler with an almost-natural language interpreter, delivering remote management and an intuitive programming interface for multiple programming languages.



## Description

uSched services provide an interface to schedule commands to be executed at a particular time, optionally repeating them over a specified interval, and optionally stopping them at any other particular time.

It provides a simple and intuitive natural language that is intepreted via a command line client, but can also be integrated into any programming language through its client libraries and bindings.

It also operates as a **client/server**, where requests performed by clients can affect local or remote machines where uSched services are running.



## Portability

uSched is designed to be compliant with any POSIX-oriented operating system. It was successfully tested under **Linux**, **FreeBSD**, **OpenBSD**, **SunOS**, **Minix**, **Darwin** and [others](https://github.com/ucodev/usched/wiki#supported-operating-systems-tested), and multiple architectures (**x86/64**, **ARM**, **MIPS**, **SPARC** and [more](https://github.com/ucodev/usched/wiki#supported-architectures-tested)).

There are some features that may not be enabled by default in the case of some non-portable calls being unavailable on the target operating system, such as chroot(). To disable such calls and features, set the CONFIG_POSIX_STRICT definition to 1 in the [include/config.h](https://github.com/ucodev/usched/blob/master/include/config.h) file.

uSched client will compile on most Windows versions and the API bindings are available for C#.



## Installation

Perform the following commands:

      ~# cd /usr/src
      ~# mkdir usched
      ~# wget https://github.com/ucodev/usched/archive/v0.1-beta-06.tar.gz
      ~# tar zxvf usched-v0.1-beta-06.tar.gz -C usched
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


Perform a request on a remote machine running uSched:

      ~$ usc -H remote.example.com -U username run '/usr/local/bin/do_something.sh' on time 11:00:00
      Password:
      ~$


Show all scheduled entries for the user by running the following command:

      ~$ usc show all


Stop all scheduled entries for the user by running the following command:

      ~$ usc stop all



## Documentation

Useful documentation links:

  * [GitHub uSched Wiki](https://github.com/ucodev/usched/wiki) - The official uSched project wiki
  * [uSched FAQ](https://github.com/ucodev/usched/wiki/FAQ) - Frequently Asked Questions
  * [Getting Started](https://github.com/ucodev/usched/wiki/Getting-Started) - A starters guide to uSched
  * [Getting Help](https://github.com/ucodev/usched/wiki#getting-help) - How to get help installing, configuring and using the project

Reference Manual:

  * [HTML](http://doc.usched.org/uSched_Reference_Manual.html) format
  * [PDF](http://doc.usched.org/uSched_Reference_Manual.pdf) format

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
          
          /* Optional remote parameters. Not required for localhost requests */
          usched_opt_set_remote_hostname("remote.usched.example.com");
          usched_opt_set_remote_port("7600");
          usched_opt_set_remote_username("username");
          usched_opt_set_remote_password("password");
          
          /* Perform the request */
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
                  
                  /* Optional remote parameters. Not required for localhost requests */
                  usc.SetHostname("remote.usched.example.com");
                  usc.SetPort("7600");
                  usc.SetUsername("username");
                  usc.SetPassword("password");
                  
                  /* Perform the request */
                  usc.Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds");
              }
          }
      }

### Java

      class UschedExample {
          public static void main(String[] args) {
              Usched usc = new Usched();
              
              /* Optional remote parameters. Not required for localhost requests */
              usc.SetHostname("remote.usched.example.com");
              usc.SetPort("7600");
              usc.SetUsername("username");
              usc.SetPassword("password");
              
              /* Perform the request */
              usc.Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds");
          }
      }

### PHP

      include("usched.php");
      
      $usc = new Usched();
      
      /* Optional remote parameters. Not required for localhost requests */
      $usc->SetHostname("remote.usched.example.com");
      $usc->SetPort("7600");
      $usc->SetUsername("username");
      $usc->SetPassword("password");
      
      /* Perform the request */
      $usc->Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds");

### Python

      from usched import *
      
      usc = Usched()
      
      /* Optional remote parameters. Not required for localhost requests */
      usc.SetHostname("remote.usched.example.com");
      usc.SetPort("7600");
      usc.SetUsername("username");
      usc.SetPassword("password");
      
      /* Perform the request */
      usc.Request("run 'df -h >> /tmp/disk.txt' now then every 30 seconds")



## License

uSched is licensed under the [GNU General Public License version 3](https://www.gnu.org/copyleft/gpl.html).



## Contributing

The best way to do it is through GitHub. Clone it, fork it, change it and improved it. Then check the [project wiki](https://github.com/ucodev/usched/wiki) for details regarding the submission of contributions.



## Notes

* Tree v0.1 of the project is on a **beta stage** and shall not be used beyond testing purposes.
* Tree v0.2 is **under development** and may not be fully functional.

