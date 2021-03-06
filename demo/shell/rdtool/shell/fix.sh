#########################################################################
# File Name: shell/fix.sh
# Author: Eric Wang
# Email: wangchaogo1990 at gmail.com
# Created Time: 2017-11-25 17:59:10
#########################################################################
#!/bin/bash
set -x

cd /home/new_mount_point1/

pstree | grep simid_bs_0 >&2
if [ $? -ne 0 ]
then
	cd simid_bs_0/bin
	./bs_control start
	if [ $? -ne 0 ]
	then
		echo "start bs 0 error on `hostname`" >&2
		exit 1
	fi
	cd -
fi

pstree | grep simid_bs_1 >&2
if [ $? -ne 0 ]
then
	cd simid_bs_1/bin
	./bs_control start
	if [ $? -ne 0 ]
	then
		echo "start bs 1 error on `hostname`" >&2
		exit 1
	fi
	cd -
fi

pstree | grep simid_wbs_0 >&2
if [ $? -ne 0 ]
then
	cd simid_wbs_0/bin
	./wbs_control start
	if [ $? -ne 0 ]
	then
		echo "start wbs 0 error on `hostname`" >&2
		exit 1
	fi
	cd -
fi

pstree | grep simid_wbs_1 >&2
if [ $? -ne 0 ]
then
	cd simid_wbs_1/bin
	./wbs_control start
	if [ $? -ne 0 ]
	then
		echo "start wbs 1 error on `hostname`" >&2
		exit 1
	fi
	cd -
fi

pstree | grep simid_as_0 >&2
if [ $? -ne 0 ]
then
	cd simid_as_0/bin
	./as_control start
	if [ $? -ne 0 ]
	then
		echo "start as 0 error on `hostname`" >&2
		exit 1
	fi
	cd -
fi

pstree | grep simid_as_1 >&2
if [ $? -ne 0 ]
then
	cd simid_as_1/bin
	./as_control start
	if [ $? -ne 0 ]
	then
		echo "start as 1 error on `hostname`" >&2
		exit 1
	fi
	cd -
fi

exit 0

