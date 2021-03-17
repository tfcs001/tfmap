# tfmap

http://tfcs.tpddns.cn

# build and run
## build and install
tfmap# cd map/<br>
tfmap/map# make<br>
tfmap/map# make install

## run
tfmap/map# /usr/bin/tfmap -c /etc/tfmap.conf

## backgroup run
tfmap/map# nohup /usr/bin/tfmap -c /etc/tfmap.conf &


## Configuration instructions
[tfmap]<br>
listen_ip=0.0.0.0     /* local ip   */<br>
listen_port=16667     /* local port */<br>
username=user         /* optional   */<br>
password=user123456   /* optional   */<br>

# Client Config
## windows
please search window10 set proxy

## Linux
### no auth
export http_proxy=http://ip:16667<br>
export https_proxy=http://ip:16667<br>
### auth
export http_proxy=http://username:passwd@ip:16667<br>
export https_proxy=http://username:passwd@ip:16667<br>
