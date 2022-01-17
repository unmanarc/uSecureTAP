# uEtherDwarf 

Unmanarc's L2 Virtual Private Network

Author: Aaron Mizrachi (unmanarc) <aaron@unmanarc.com>   
Main License: GPLv3

***
## Builds

- COPR (Fedora/CentOS/etc):  
[![Copr build status](https://copr.fedorainfracloud.org/coprs/amizrachi/unmanarc/package/uEtherDwarf/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/amizrachi/unmanarc/package/uEtherDwarf/)


Install in Fedora/RHEL8/9:
```bash
dnf copr enable amizrachi/libMantids
dnf copr enable amizrachi/uEtherDwarf

dnf -y install uEtherDwarf
```

Install in RHEL7:
```bash
yum copr enable amizrachi/libMantids
yum copr enable amizrachi/uEtherDwarf

yum -y install uEtherDwarf
```

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
