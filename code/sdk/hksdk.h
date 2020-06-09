#ifndef __HKSDK_H
#define __HKSDK_H

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <thread>

#include "HCNetSDK.h"
#include "PlayM4.h"

namespace hksdk
{
	typedef struct tagLOGIN_INFO
	{
		tagLOGIN_INFO() {
			port = 8000;
		}
		std::string ip;
		std::string user;
		std::string pwd;
		int port;
	}LOGIN_INFO;

	typedef struct {
		std::string file_name;
		std::string start_time;
		std::string end_time;
		std::string file_size; // MB
		std::string nvr_ip;
		std::string ip;
	}FILE_RECORD_INFO;

	void sdk_init();
	void sdk_cleanup();
};

#endif // !__HKSDK_H


