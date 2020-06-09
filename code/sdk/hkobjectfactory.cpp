#include "hkobjectfactory.h"

using namespace hksdk;

std::shared_ptr<hknvr> hkobjectfactory::login_nvr(const std::string & ip, const std::string & user, const std::string & pwd)
{
	if (m_nvr.find(ip) != m_nvr.end())
	{
		return m_nvr[ip];
	}

	std::shared_ptr<hknvr> nvr = std::make_shared<hknvr>();
	nvr->Login(ip, user, pwd);
	m_nvr[ip] = nvr;
	return nvr;
}

std::shared_ptr<hknvr> hkobjectfactory::get_nvr(const std::string & ip)
{
	if (m_nvr.find(ip) != m_nvr.end())
	{
		return m_nvr[ip];
	}
	return nullptr;
}

std::shared_ptr<hkipc> hkobjectfactory::create_ipc(std::shared_ptr<hknvr> nvr, const std::string &ip, const std::string &user, const std::string &pwd)
{
	if (!nvr)
	{
		std::shared_ptr<hkipc> ipc = std::make_shared<hkipc>();
		ipc->Login(ip, user, pwd);
		return ipc;
	}
	else
	{
		std::shared_ptr<hkipcEx> ipcEx = std::make_shared<hkipcEx>();
		LONG channel = nvr->GetPlayChannel(ip);
		if (channel >= 0)
		{
			ipcEx->SetUserID(nvr->GetUserID());
			ipcEx->SetPlayChannel(channel);
		}
		channel = nvr->GetVoiceChannel(ip);
		if (channel >= 0)
		{
			ipcEx->SetVoiceChannel(channel);
		}
		return ipcEx;
	}
	return nullptr;
}

std::shared_ptr<hkvideoplayback> hkobjectfactory::create_playback(std::shared_ptr<hknvr> nvr, const std::string& file)
{
	std::shared_ptr<hkvideoplaybackByName> playback = std::make_shared<hkvideoplaybackByName>();
	playback->SetUserID(nvr->GetUserID());
	playback->SetPlaybackFile(file);

	return playback;
}

std::shared_ptr<hksdk::hkvideoplayback> hkobjectfactory::create_playback(std::shared_ptr<hknvr> nvr, const std::string &ipc_ip, const std::string & startTime, const std::string & endTime)
{
	std::shared_ptr<hkvideoplaybackByTime> playback = std::make_shared<hkvideoplaybackByTime>();
	playback->SetUserID(nvr->GetUserID());
	playback->SetPlayChannel(nvr->GetPlayChannel(ipc_ip));
	playback->SetTime(startTime, endTime);
	
	return playback;
}

std::shared_ptr<hkvideodownload> hkobjectfactory::create_download(std::shared_ptr<hknvr> nvr, const std::string& src, const std::string& dst)
{
	std::shared_ptr<hkvideodownloadByName> download = std::make_shared<hkvideodownloadByName>();
	download->SetUserID(nvr->GetUserID());
	download->SetSrcFile(src);
	download->SetDestFile(dst);
	
	return download;
}

std::shared_ptr<hkvideodownload> hkobjectfactory::create_download(std::shared_ptr<hknvr> nvr, const std::string &ipc_ip, const std::string &startTime, const std::string &endTime, const std::string& dst)
{
	std::shared_ptr<hkvideodownloadByTime> download = std::make_shared<hkvideodownloadByTime>();
	download->SetUserID(nvr->GetUserID());
	download->SetDownloadChannel(nvr->GetPlayChannel(ipc_ip));
	download->SetDestFile(dst);
	download->SetTime(startTime, endTime);
	
	return download;
}