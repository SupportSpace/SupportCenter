@echo off

set bin=FTestRelay.exe
set server=192.168.0.66
set port=5904
rem set passwd=<some password>
set test=0
set pool=500
set peers=10000
set extport=7799
set user=TestUser


%bin% -a=%server% -p=%port% -t=%test% -l=%pool% -e=%peers% -x=%extport% -u=%user%
