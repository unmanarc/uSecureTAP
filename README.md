# uEtherDwarf 

Unmanarc's L2 Virtual Private Network

Author: Aaron Mizrachi (unmanarc) <aaron@unmanarc.com>   
Main License: GPLv3

***
## Builds

- COPR (Fedora/CentOS/etc):  
[![Copr build status](https://copr.fedorainfracloud.org/coprs/amizrachi/unmanarc/package/uEtherDwarf/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/amizrachi/unmanarc/package/uEtherDwarf/)


### Simple installation guide for Fedora/RHEL:

To activate our repo's and download/install the software:

In RHEL7:
```bash
# Install EPEL Repo + COPR
yum -y install epel-release
yum -y install yum-plugin-copr

# Install unmanarc's copr
yum copr enable amizrachi/unmanarc -y
yum -y install uEtherDwarf
```

In RHEL8:
```bash
# Install EPEL Repo
dnf -y install 'dnf-command(config-manager)'
dnf config-manager --set-enabled powertools
dnf -y install epel-release

# Install unmanarc's copr
dnf copr enable amizrachi/unmanarc -y
dnf -y install uEtherDwarf
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

- Starting the service:

```
systemctl start uEtherDwarf
```

- Stopping the service:

```
systemctl stop uEtherDwarf
```

- Installing the service:

```
systemctl enable uEtherDwarf
```

- Don't forget to open the firewall
