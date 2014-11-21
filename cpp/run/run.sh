#/bin/sh
#start or stop the im-server

function start() {
	cd login_server
	if [ -e loginserver.conf ]; then
		echo "loginserver.conf exist"
		../daeml ./login_server
	fi

	cd ../route_server
	if [ -e routeserver.conf ]; then
		../daeml ./route_server
	fi

	cd ../msg_server
	if [ -e msgserver.conf ]; then
		../daeml ./msg_server
	fi

	cd ../file_server
	if [ -e fileserver.conf ]; then
		../daeml ./file_server
	fi
	
	cd ../msfs
	if [ -e msfs.conf ]; then
		../daeml ./msfs
	fi
	
	cd ..
}

function stop() {
	cd login_server
	for i in `ls log*_1.txt`; do
		pid=`echo $i|awk -F_ '{print $2}'`  # get pid 
		kill $pid
	done
	mv log*.txt oldlog/

	cd ../route_server
	for i in `ls log*_1.txt`; do
		pid=`echo $i|awk -F_ '{print $2}'`  # get pid 
		kill $pid
	done
	mv log*.txt oldlog/

	cd ../msg_server
	for i in `ls log*_1.txt`; do
		pid=`echo $i|awk -F_ '{print $2}'`  # get pid 
		kill $pid
	done
	mv log*.txt oldlog/

	cd ../file_server
	for i in `ls log*_1.txt`; do
		pid=`echo $i|awk -F_ '{print $2}'`  # get pid 
		kill $pid
	done
	mv log*.txt oldlog/
	
	cd ../msfs
	for i in `ls log*_1.txt`; do
		pid=`echo $i|awk -F_ '{print $2}'`  # get pid 
		kill $pid
	done
	mv log*.txt oldlog/
	
    cd ..
   
}

case $1 in
	start)
		start
		;;
	stop)
		stop
		;;
    restart)
        stop
        start
        ;;
	*)
		echo "Usage: "
		echo "  ./run.sh (start|stop|restart)"
		;;
esac
