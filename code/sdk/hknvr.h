#ifndef _HKNVR_H_
#define _HKNVR_H_
#include "hksdk.h"

namespace hksdk
{
	class hknvr
	{
	public:
		hknvr();
		~hknvr();
		BOOL Login(const std::string &ip, const std::string &user, const std::string &pwd, int port = 8000);
		void Logout();

		LONG GetUserID();
		LONG GetPlayChannel(const std::string &strIP);
		LONG GetVoiceChannel(const std::string &strIP);

		BOOL GetStatus();
		void SearchPlayback(const std::list<std::string> &ips, const std::string &startTime, const std::string &endTime, std::list< FILE_RECORD_INFO> &file_list);

	private:
		void DoGetPlayChannel(const NET_DVR_IPPARACFG_V40 &ipParaCfg);
		void DoGetVoiceChannel(const NET_DVR_IPPARACFG_V40 &IPChanInfo, const NET_DVR_DEVICEINFO_V40& device);
		void SearchPlayback(const std::string &ip, const std::string &startTime, const std::string &endTime, std::list< FILE_RECORD_INFO> &file_list);
		
	private:
		LOGIN_INFO m_LoginInfo;
		LONG m_lUserID;
		std::map<std::string, LONG> m_PlayChannel;
		std::map<std::string, LONG> m_VoiceChannel; 
	};
};

#endif
