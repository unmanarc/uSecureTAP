# uEtherDwarf 

Unmanarc's L2 Virtual Private Network

Author: Aaron Mizrachi (unmanarc) <aaron@unmanarc.com>   
Main License: GPLv3

***
## Project Description

Simple TLS Client/Server L2 VPN that comes with:

- PSK secure authentication
- Encryption/Unencrypted traffic Support
- No special signature in the VPN traffic
- Client MAC address security enforcing
- UP/DOWN Scripts

### Use Cases:

You can use program for:

- Simplified VPN configurations

***
## Building/Installing uEtherDwarf

### Building

[Building Instructions Here](BUILD.md)

### Install

[Install Instructions Here](INSTALL.md)

## Compatibility

This program was tested in:

* Fedora Linux 34
* Ubuntu 20.04 LTS (Server)
* CentOS/RHEL 7/8

### Overall Pre-requisites:

* libMantids (in static mode)
* OpenSSL (in static mode)
* cmake3
* C++11 Compatible Compiler (like GCC >=5)

### Running Instructions:

Here are the instructions to use this program:

Starting the service:

```
systemctl start uEtherDwarf
```

Stopping the service:

```
systemctl stop uEtherDwarf
```



Program help:

```
# Unmanarc's L2 Virtual Private Network (uEtherDwarf) v1.0.1
# Author:  Aarón Mizrachi (aaron@unmanarc.com)
# License: GPLv3
# 

Help:
-----

Authentication:
---------------
-f --peersfile <value> : Formatted multi line file (IP:PSK:MAC, first line is for myself) (default: )

Other Options:
--------------
-v --verbose <value> : Set verbosity level. (default: 0)
-h --help            : Show information usage. (default: false)
-s --sys             : Journalctl Log Mode (don't print colors or dates) (default: false)
-x --uid     <value> : Drop Privileged to UID (default: 0)
-g --gid     <value> : Drop Privileged to GID (default: 0)

Scripts:
--------
-u --up   <value> : Up Script (executed when connection is up) (default: )
-w --down <value> : Down Script (executed when connection is down) (default: )

Service Options:
----------------
-d --daemon         : Run as daemon. (default: false)

TAP Interface:
--------------
-i --interface <value> : Interface Name (default: utap0)

TLS Options:
------------
-9 --notls            : Disable the use of TLS (default: false)
-4 --ipv4             : Use only IPv4 (default: true)
-y --cafile   <value> : X.509 Certificate Authority Path (default: )
-k --keyfile  <value> : X.509 Private Key Path (For listening mode) (default: )
-c --certfile <value> : X.509 Certificate Path (For listening mode) (default: )
-l --listen           : Use in listen mode (default: false)
-p --port     <value> : Port (default: 443)
-a --addr     <value> : Address (default: *)
-t --threads  <value> : Max Concurrent Connections (Threads) (default: 1024)
```
