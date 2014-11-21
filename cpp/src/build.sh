#!/bin/bash

build() {
	echo "#ifndef __VERSION_H__" > base/version.h
	echo "#define __VERSION_H__" >> base/version.h
	echo "#define VERSION \"$1\"" >> base/version.h
	echo "#endif" >> base/version.h
 
	cd base
	make
	cd ../login_server
	make
	cd ../route_server
	make
	cd ../msg_server
	make
	cd ../file_server
	make
    cd ../msfs
    make
	cd ../tools
	make
    cd ../

    mkdir -p ../run/login_server
    mkdir -p ../run/route_server
    mkdir -p ../run/msg_server
    mkdir -p ../run/file_server
    mkdir -p ../run/msfs
	#copy executables to run/ dir


    cp login_server/login_server ../run/login_server/
    cp route_server/route_server ../run/route_server/
    cp msg_server/msg_server ../run/msg_server/
    cp file_server/file_server ../run/file_server/
    cp msfs/msfs ../run/msfs/
    cp tools/daeml ../run/

    build_version=im-server-$1
	build_name=$build_version.tar.gz
	if [ -e "$build_name" ]; then 
		rm $build_name
	fi

    mkdir -p ../$build_version
    mkdir -p ../$build_version/login_server
    mkdir -p ../$build_version/route_server
    mkdir -p ../$build_version/msg_server
    mkdir -p ../$build_version/file_server
    mkdir -p ../$build_version/msfs
    cp login_server/login_server ../$build_version/login_server/
    cp login_server/loginserver.conf ../$build_version/login_server/
    cp route_server/route_server ../$build_version/route_server/
    cp route_server/routeserver.conf ../$build_version/route_server/
    cp msg_server/msg_server ../$build_version/msg_server/
    cp msg_server/msgserver.conf ../$build_version/msg_server/
    cp file_server/file_server ../$build_version/file_server/
    cp file_server/fileserver.conf ../$build_version/file_server/
    cp msfs/msfs ../$build_version/msfs/
    cp msfs/msfs.conf.example ../$build_version/msfs/msfs.conf
    cp tools/daeml ../$build_version/
    cp ../run/restart.sh ../$build_version/
    cp ../run/run.sh    ../$build_version/

    cd ../
	tar zcvf $build_name $build_version/daeml $build_version/restart.sh $build_version/run.sh \
                         $build_version/login_server/login_server $build_version/login_server/loginserver.conf \
                         $build_version/route_server/route_server $build_version/route_server/routeserver.conf \
                         $build_version/msg_server/msg_server $build_version/msg_server/msgserver.conf\
                         $build_version/file_server/file_server $build_version/file_server/fileserver.conf\
                         $build_version/msfs/msfs $build_version/msfs/msfs.conf
    rm -rf $build_version
}

clean() {
	cd base
	make clean
	cd ../login_server
	make clean
	cd ../route_server
	make clean
	cd ../msg_server
	make clean
	cd ../file_server
	make clean
    cd ../msfs
    make clean
    
}

print_help() {
	echo "Usage: "
	echo "  $0 clean --- clean all build"
	echo "  $0 version version_str --- build a version"
}

case $1 in
	clean)
		echo "clean all build..."
		clean
		;;
	version)
		if [ $# != 2 ]; then 
			echo $#
			print_help
			exit
		fi

		echo $2
		echo "build..."
		build $2
		;;
	*)
		print_help
		;;
esac
