## Shadowsocks Bootstrap

> A simple program to make the original shadowsocks support SIP003 plugins.

### Quick Start

As we all know, the [original shadowsocks](https://github.com/shadowsocks/shadowsocks) is written in python, this version does not support [SIP003](https://shadowsocks.org/en/wiki/Plugin.html) plugin, and [shadowsocks-libev](https://github.com/shadowsocks/shadowsocks-libev) or [shadowsocks-rust](https://github.com/shadowsocks/shadowsocks-rust) which support plugin lack some encryption methods (although they are insecure), so making the original shadowsocks support SIP003 plugin will be able to get better compatibility.

The SIP003 plugin runs as a subroutine. In shadowsocks-bootstrap, both the shadowsocks program and the plugin will be executed as subroutines to assign ports and manage them.

You can use shadowsocks-bootstrap just like shadowsocks, the only difference is the extra `--plugin` and `--plugin-opts` options. In addition, it will use `sslocal` or `ssserver` as shadowsocks program by default, if necessary you should specify a custom filename with `--shadowsocks` option.

```
A simple program to make the original shadowsocks support SIP003 plugins.

-s <server_host>             Host name or IP address of your remote server.
-p <server_port>             Port number of your remote server.
-b <local_address>           Local address to bind.
-l <local_port>              Port number of your local server.
-c <config_file>             Path to JSON config file.
-k <password>                Password of your remote server.
-m <method>                  Encrypt method.
-t <timeout>                 Socket timeout in seconds.
--fast-open                  Enable TCP fast open (with Linux kernel 3.7+).
--plugin <name>              Enable SIP003 plugin.
--plugin-opts <options>      Set SIP003 plugin options.
--shadowsocks <shadowsocks>  Set shadowsocks local or server program.
--no-udp                     Do not use UDP proxy.
-h, --help                   Print this message.
```

You can also run via a JSON config file:

```
{
    "server": "...",
    "server_port": 8388,
    "local_address": "127.0.0.1",
    "local_port": 1080,
    "password": "...",
    "timeout": 300,
    "method": "aes-256-cfb",
    "fast_open": false,
    "no_udp": false,
    "plugin": "...",
    "plugin_opts": "...",
    "shadowsocks": "...",
    "extra_opts": "..."
}
```

Example:

```bash
shell> ss-bootstrap-server -s 0.0.0.0 -p 12345 -k dnomd343 -m aes-256-ctr --shadowsocks ss-python-server --plugin obfs-server --plugin-opts "obfs=http"
shell> ss-bootstrap-local -s 127.0.0.1 -p 12345 -b 0.0.0.0 -l 1080 -k dnomd343 -m aes-256-ctr --shadowsocks ss-python-local --plugin obfs-local --plugin-opts "obfs=http;obfs-host=www.bing.com"
```

### Compile

You need to install `gcc` , `make` and `cmake` at first, and also need the `glib2.0` development environment.

<details>

<summary>Examples</summary>

```bash
# Alpine
apk add build-base make cmake git glib-dev
git clone https://github.com/dnomd343/shadowsocks-bootstrap.git
cd shadowsocks-bootstrap/
mkdir build && cd build/
cmake .. && make
mv ../bin/* /usr/bin/
```

```bash
# Ubuntu
sudo apt update
sudo apt install build-essential cmake git libglib2.0-dev
git clone https://github.com/dnomd343/shadowsocks-bootstrap.git
cd shadowsocks-bootstrap/
mkdir build && cd build/
cmake .. && make
sudo mv ../bin/* /usr/local/bin/
```

```bash
# CentOS
sudo yum update
sudo yum groupinstall "Development Tools"
sudo yum install cmake libgnomeui-devel
git clone https://github.com/dnomd343/shadowsocks-bootstrap.git
cd shadowsocks-bootstrap/
mkdir build && cd build/
cmake .. && make
sudo mv ../bin/* /usr/local/bin/
```

</details>

### License

MIT ©2022 [@dnomd343](https://github.com/dnomd343)
