#ifndef _HK_OBJECT_FACTORY_H
#define _HK_OBJECT_FACTORY_H
#include "hkipc.h"
#include "hknvr.h"
#include "hkvideoplayback.h"
#include "hkvideodownload.h"

namespace hksdk
{
	class hkobjectfactory
	{
	public:
		std::shared_ptr<hknvr> login_nvr(const std::string &ip, const std::string &user, const std::string &pwd);
		std::shared_ptr<hknvr> get_nvr(const std::string &ip);

		std::shared_ptr<hksdk::hkipc> create_ipc(std::shared_ptr<hknvr> nvr, const std::string &ip, const std::string &user, const std::string &pwd);
		std::shared_ptr<hksdk::hkvideoplayback> create_playback(std::shared_ptr<hknvr> nvr, const std::string& file);
		std::shared_ptr<hksdk::hkvideoplayback> create_playback(std::shared_ptr<hknvr> nvr, const std::string &ipc_ip, const std::string &startTime, const std::string &endTime);
		std::shared_ptr<hksdk::hkvideodownload> create_download(std::shared_ptr<hknvr> nvr, const std::string& src, const std::string& dst);
		std::shared_ptr<hksdk::hkvideodownload> create_download(std::shared_ptr<hknvr> nvr, const std::string &ipc_ip, const std::string &startTime, const std::string &endTime, const std::string& dst);
	
	private:
		std::map<std::string, std::shared_ptr<hksdk::hknvr>> m_nvr;
	};
};

#endif

