# tfmap

http://tfcs.tpddns.cn

# build and install
tfmap# cd map/
tfmap/map# make
tfmap/map# make install

# run
tfmap/map# /usr/bin/tfmap -c /etc/tfmap.conf

***************************************************************
*                                                             *
*                         tfmap v1.0.0                        *
*            Compiled at 23:52:06, on  Mar 17 2021            *
*                                                             *
*                                                             *
*                                                             *
*                Author:     tfcs                              *
*                                                             *
*                  Time:   2019-01 ~~ 2021-03                 *
*                  (c) All Rights Reserved.                   *
***************************************************************
2021/03/17-23:53:41 INFO main: log level:info
2021/03/17-23:53:41 INFO main: log output:stdout
log level=6, output=0
2021/03/17-23:53:41 INFO main: tfmap listen_ip:0.0.0.0
2021/03/17-23:53:41 INFO main: tfmap listen_port:16667
2021/03/17-23:53:41 INFO main: tfmap username=user, password=user123456
2021/03/17-23:53:41 NOTICE main: ulimit hard rlim_cur=10000, soft rlim_max=10000
2021/03/17-23:53:41 INFO main: create map thread success
2021/03/17-23:53:41 INFO listen_for_client: [map]listen port 16667 success
2021/03/17-23:53:41 INFO start: create timer thread success

# backgroup run
tfmap/map# nohup /usr/bin/tfmap -c /etc/tfmap.conf &


# Configuration instructions
/tfmap/map# cat /etc/tfmap.conf
##########################################################
[module]
######################(on/off)############
tfmap=on
[log]
##########level(debug/info/notice/warn/error/crit/alert/emerg)############
level=info
##########ouput(stdout/syslog)############
ouput=stdout
[tfmap]
listen_ip=0.0.0.0     /* local ip   */
listen_port=16667     /* local port */
username=user         /* optional   */
password=user123456   /* optional   */