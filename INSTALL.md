
# Service Installation Instructions:


## Server
***
Install the server will require some steps to be completed (please replace the required information...):

- create your /etc/uetherdwarf server dir:

```
mkdir -p /etc/uetherdwarf
```

- Create the execution running script `run.sh`, you can change the port (eg. 465), the up/down scripts, and other values inside this script:
```bash
cat << 'EOF' | install -m '750' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/run.sh'
#!/bin/bash

cd /etc/uetherdwarf

while [ true ]; do
        /usr/sbin/uEtherDwarf --listen true -i vpn0 -f peers.txt -p 465 -k server.key -c server.crt --up up.sh --down down.sh --sys
        sleep 5s
done

EOF
```

- create the `server.crt`:

```bash
cat << 'EOF' | install -m '600' -o 'root'  /dev/stdin '/etc/uetherdwarf/server.crt'
-----BEGIN CERTIFICATE-----
... REPLACE WITH YOUR SERVER SIGNED CERTIFICATE HERE ...
-----END CERTIFICATE-----
EOF
```

- create the `server.key`:

```bash
cat << 'EOF' | install -m '600' -o 'root'  /dev/stdin '/etc/uetherdwarf/server.key'
-----BEGIN PRIVATE KEY-----
... REPLACE WITH YOUR SERVER PRIVATE KEY HERE ...
-----END PRIVATE KEY-----
EOF
```

- Create the peers file:

This is a comma separated (CSV) file.

The first line is for our node, it has to have our IP address, and then the PSK that will be shared with all the clients.

The subsequent lines can include the client peers:

1. First field is the remote **VPN IP** address and VPN ID, every node is identified with the VPN IP address like a username.
2. Second field indicate the **PSK**, this is a shared value and will identify the VPN client. This value only has to be shared with the peer, not with anyone else.
3. Third field is the **"forced MAC address"**, whenever the client sends, the VPN will rewrite the MAC address with this value. This is very useful for *iptables packet filtering*


```bash
cat << 'EOF' | install -m '600' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/peers.txt'
10.99.99.1/24,ULTRASECRET_REPLACEME_PSK_001,
10.99.99.2,ULTRASECRET_REPLACEME_PSK_DFGAAAF,00:00:10:99:99:02
10.99.99.3,ULTRASECRET_REPLACEME_PSK_ASFDXOG,00:00:10:99:99:03
EOF
```

Why a fixed MAC Address by client? 

- We know that the client itself can have some subnets behind it, you can set up some routing rules using up.sh/down.sh scripts, but you also can filter spoofing attempts and/or put some iptables rules there (based on MAC Address).


### Server Optional Setup:

Optional setup can be included/excluded from `run.sh`

- create the `ca.crt` (only if you want to authenticate users SSL keys):
```bash
cat << 'EOF' | install -m '600' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/ca.crt'
-----BEGIN CERTIFICATE-----
... REPLACE WITH YOUR CA CERTIFICATE HERE ...
-----END CERTIFICATE-----
EOF
```
- create the `down.sh`

```bash
cat << 'EOF' | install -m '755' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/down.sh'
#!/bin/bash
# This script will be called when some connection to the client is lost, REMOTEADDR env variable will be filled with peers.txt first field (VPN IP Address from the remote endpoint)
EOF
```

- create the `up.sh`

```bash
cat << 'EOF' | install -m '755' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/up.sh'
#!/bin/bash
# This script will be called when some connection to the client is authenticated, REMOTEADDR env variable will be filled with peers.txt first field (VPN IP Address from the remote endpoint)
EOF
```



### Creating the service:


```bash
cat << 'EOF' | install -m 640 /dev/stdin /etc/systemd/system/uEtherDwarf.service
[Unit]
Description=Unmanarc L2 Virtual Private Network
After=network.target

[Service]
Type=simple
Restart=always
RestartSec=1
ExecStart=/etc/uetherdwarf/run.sh

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now uEtherDwarf.service
```

from this point you will be running this program, you this program to monitor the output/running:
```
journalctl -xef -u uEtherDwarf
``` 



## Client
***


Install the client will require some steps to be completed (please replace the required information...):

- create your /etc/uetherdwarf/clientdir client dir:

```
mkdir -p /etc/uetherdwarf/clientdir
```

- Create the execution running script `run.sh`, you can change the host and port (eg. 465), the scripts, and other values inside this script:
```bash
cat << 'EOF' | install -m '750' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/clientdir/run.sh'
#!/bin/bash

cd /etc/uetherdwarf/clientdir

while [ true ]; do
        /usr/sbin/uEtherDwarf --listen true -i vpn0 -f peers.txt -p 465 -k client.key -c client.crt --up up.sh --down down.sh --sys
        sleep 5s
done

EOF
```

- create the `ca.crt` (to authenticate the server, this is very important):
```bash
cat << 'EOF' | install -m '600' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/clientdir/ca.crt'
-----BEGIN CERTIFICATE-----
... REPLACE WITH YOUR CA CERTIFICATE HERE ...
-----END CERTIFICATE-----
EOF
```

- Create the peers file:

This is a comma separated (CSV) file.

The first line is for our node, it has to have our IP address, and then the PSK that correspond to our node/client.
The second and last line is for the server:

1. First field is the remote server **VPN IP** address.
2. Second field indicate the **PSK** for the server
3. Third field is the **"forced MAC address"**, whenever the client sends, the VPN will rewrite the MAC address with this value. This is very useful for *iptables packet filtering*


```bash
cat << 'EOF' | install -m '600' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/clientdir/peers.txt'
10.99.99.2/24,ULTRASECRET_REPLACEME_PSK_DFGAAAF
10.99.99.1,ULTRASECRET_REPLACEME_PSK_001,00:00:10:99:99:01
EOF
```

### Client Optional Setup:

Optional setup can be included/excluded from `run.sh`

If the client is TLS authenticated, include these files (client crt/key):

- create the `client.crt`:

```bash
cat << 'EOF' | install -m '600' -o 'root'  /dev/stdin '/etc/uetherdwarf/clientdir/client.crt'
-----BEGIN CERTIFICATE-----
... REPLACE WITH YOUR CLIENT SIGNED CERTIFICATE HERE ...
-----END CERTIFICATE-----
EOF
```
- create the `client.key`:

```bash
cat << 'EOF' | install -m '600' -o 'root'  /dev/stdin '/etc/uetherdwarf/clientdir/client.key'
-----BEGIN PRIVATE KEY-----
... REPLACE WITH YOUR CLIENT PRIVATE KEY HERE ...
-----END PRIVATE KEY-----
EOF
```

And if you have something to execute when is connected (eg. routes, iptables rules, etc):

- create the `down.sh`

```bash
cat << 'EOF' | install -m '755' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/clientdir/down.sh'
#!/bin/bash
# This script will be called when some connection to the server is lost, REMOTEADDR env variable will be filled with peers.txt first comma separated field (VPN IP Address from the remote endpoint)
EOF
```

- create the `up.sh`

```bash
cat << 'EOF' | install -m '755' -o 'root' -g 'root' /dev/stdin '/etc/uetherdwarf/clientdir/up.sh'
#!/bin/bash
# This script will be called when some connection to the server is authenticated, REMOTEADDR env variable will be filled with peers.txt first comma separated field (VPN IP Address from the remote endpoint)
EOF
```

### Creating the service:


```bash
cat << 'EOF' | install -m 640 /dev/stdin /etc/systemd/system/uEtherDwarf-Client.service
[Unit]
Description=Unmanarc L2 Virtual Private Network
After=network.target

[Service]
Type=simple
Restart=always
RestartSec=1
ExecStart=/etc/uetherdwarf/clientdir/run.sh

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now uEtherDwarf-Client.service
```

from this point you will be running this program, you this program to monitor the output/running:
```
journalctl -xef -u uEtherDwarf-Client
``` 



