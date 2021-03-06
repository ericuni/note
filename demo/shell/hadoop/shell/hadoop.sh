#!/bin/bash
set -x

source ./conf/common.conf || exit 1
source ./conf/func.sh || exit 1

$local_hadoop dfs -test -d $top_dir
if [ $? -ne 0 ]
then
	$local_hadoop dfs -mkdir $top_dir
fi

files=""
for file in `ls bin/* conf/* shell/*`
do
	files="$files -file $file"
done 1>/dev/null 2>&1
echo "files=$files"

$local_hadoop dfs -rmr $output
$local_hadoop streaming \
	-input $input \
	-output $output \
	-mapper "bash mapper.sh 0" \
	-reducer "bash reducer.sh 0" \
	-partitioner "org.apache.hadoop.mapred.lib.KeyFieldBasedPartitioner" \
	-jobconf mapred.map.tasks="$mapper_num" \
	-jobconf mapred.job.map.capacity="$mapper_capacity" \
	-jobconf mapred.map.over.capacity.allowed="false" \
	-jobconf mapred.map.tasks.speculative.execution="true" \
	-jobconf mapred.map.max.attempts="3" \
	-jobconf mapred.max.map.failures.percent="5" \
	-jobconf mapred.reduce.slowstart.completed.maps=$slowstart \
	-jobconf mapred.reduce.tasks="$reducer_num" \
	-jobconf mapred.job.reduce.capacity="$reducer_capacity" \
	-jobconf mapred.reduce.over.capacity.allowed="false" \
	-jobconf mapred.reduce.tasks.speculative.execution="true" \
	-jobconf mapred.reduce.max.attempts="3" \
	-jobconf mapred.max.reduce.failures.percent="5" \
	-jobconf stream.memory.limit="800" \
	-jobconf mapred.job.name="${owner}_${job_name}" \
	-jobconf mapred.job.priority="$priority" \
	$files

CHK_RET FATAL "$job_name error"

exit 0

