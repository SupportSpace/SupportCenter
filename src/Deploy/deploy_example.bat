echo build start time: %date% %time%
nant -t:net-2.0 doall
rem nant -t:net-2.0 doall -D:version=0 -D:deploy_url="http://213.8.114.131:18080/jwchat/archer/" -D:tag="sprint2_00_00" -D:configuration=dynamic_debug
rem nant -t:net-2.0 doall -D:version=14 -D:deploy_url="http://max/recent" -D:deploy_dir="\\max\wwwroot\recent"
echo build finished time: %date% %time%
