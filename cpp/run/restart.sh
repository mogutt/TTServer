#/bin/sh
#start or stop the im-server

function restart() {
	cd $1
	if [ ! -e *.conf ]
	then 
		echo "no config file"
		return
	fi

	if [ -e log*_1.txt ]; then
		for i in `ls log*_1.txt`; do
			pid=`echo $i|awk -F_ '{print $2}'`  # get pid 
			kill -SIGHUP $pid
		done
		mkdir -p oldlog/
		mv log*.txt oldlog/
	fi
	../daeml ./$1
}

case $1 in
	login_server)
		restart $1
		;;
	msg_server)
		restart $1
		;;
	route_server)
		restart $1
		;;
	file_server)
		restart $1
		;;
	msfs)
		restart $1
		;;
	*)
		echo "Usage: "
		echo "  ./restart.sh (login_server|msg_server|route_server|file_server|msfs)"
		;;
esac
