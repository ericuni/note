/*************************************************************************
	> File Name: demo.cpp
	> Author: Eric Wang
	> Email: wangchaogo1990 at gmail.com
	> Created Time: 2017-07-23 12:29:37
 ************************************************************************/

#include <stdio.h>
#include <glog/logging.h>

int main(int argc, char *argv[]){
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = true; // only log to stderr
	FLAGS_alsologtostderr = true;
    FLAGS_colorlogtostderr = true;
	FLAGS_stderrthreshold = google::INFO;
	FLAGS_minloglevel = 0;
	FLAGS_log_dir = "./";

	LOG(INFO) << "google log info";
	LOG(WARNING) << "google log warning";
	LOG(ERROR) << "google log error";
// 	LOG(FATAL) << "google log fatal"; // fatal will cause abort

	google::ShutdownGoogleLogging();
	return 0;
}

