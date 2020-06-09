#include "hknvr.h"

using namespace hksdk;

std::string formatstr(const char* fmt, ...)
{
	std::string tmp;
	int len = 0;
	va_list args;
	va_start(args, fmt);
	len = _vscprintf(fmt, args);
	tmp.resize(len + 1);
	vsnprintf((char*)tmp.data(), tmp.capacity(), fmt, args);
	va_end(args);
	return tmp;
}

hknvr::hknvr()
{
	m_lUserID = -1;
}

hknvr::~hknvr()
{
	Logout();
}

BOOL hknvr::Login(const std::string & ip, const std::string & user, const std::string & pwd, int port)
{
	m_LoginInfo.ip = ip;
	m_LoginInfo.user = user;
	m_LoginInfo.pwd = pwd;
	m_LoginInfo.port = port;

	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	NET_DVR_USER_LOGIN_INFO struLoginInfo{};
	struLoginInfo.wPort = port;
	strcpy_s(struLoginInfo.sDeviceAddress, ip.c_str());
	strcpy_s(struLoginInfo.sUserName, user.c_str());
	strcpy_s(struLoginInfo.sPassword, pwd.c_str());

	NET_DVR_DEVICEINFO_V40 struDeviceInfoV40{};
	m_lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
	if (m_lUserID < 0)
	{
		return FALSE;
	}

	NET_DVR_IPPARACFG_V40 IPChanInfo{};
	DWORD dwReturned;
	NET_DVR_GetDVRConfig(m_lUserID, NET_DVR_GET_IPPARACFG_V40, 0, &IPChanInfo, sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);

	DoGetPlayChannel(IPChanInfo);
	DoGetVoiceChannel(IPChanInfo, struDeviceInfoV40);
	return TRUE;
}

void hknvr::Logout()
{
	if (m_lUserID >= 0)
	{
		NET_DVR_Logout(m_lUserID);
		m_lUserID = -1;
	}
}

LONG hknvr::GetUserID()
{
	return m_lUserID;
}

BOOL hknvr::GetStatus()
{
	return m_lUserID >= 0;
}

LONG hknvr::GetPlayChannel(const std::string &strIP)
{
	LONG lChannel = -1;
	if (m_PlayChannel.find(strIP) != m_PlayChannel.end())
	{
		lChannel = m_PlayChannel[strIP];
	}
	return lChannel;
}

LONG hknvr::GetVoiceChannel(const std::string &strIP)
{
	LONG lChannel = -1;
	if (m_VoiceChannel.find(strIP) != m_VoiceChannel.end())
	{
		lChannel = m_VoiceChannel[strIP];
	}
	return lChannel;
}

void hknvr::DoGetPlayChannel(const NET_DVR_IPPARACFG_V40 &IPChanInfo)
{
	m_PlayChannel.clear();
	for (DWORD i = 0; i < IPChanInfo.dwDChanNum; ++i)
	{
		if (strlen(IPChanInfo.struIPDevInfo[i].struIP.sIpV4))
		{
			m_PlayChannel[IPChanInfo.struIPDevInfo[i].struIP.sIpV4] = i + IPChanInfo.dwStartDChan;
		}
	}
}

void hknvr::DoGetVoiceChannel(const NET_DVR_IPPARACFG_V40 &IPChanInfo, const NET_DVR_DEVICEINFO_V40& device)
{
	for (DWORD i = 0; i < IPChanInfo.dwDChanNum; ++i)
	{
		if (strlen(IPChanInfo.struIPDevInfo[i].struIP.sIpV4) &&
			IPChanInfo.struStreamMode[i].uGetStream.struChanInfo.byEnable)
		{
			m_VoiceChannel[IPChanInfo.struIPDevInfo[i].struIP.sIpV4] = i + device.struDeviceV30.byStartDTalkChan;
		}
	}
}

void hknvr::SearchPlayback(const std::list<std::string>& ips, const std::string & startTime, const std::string & endTime, std::list<FILE_RECORD_INFO>& file_list)
{
	for (auto ip : ips)
	{
		SearchPlayback(ip, startTime, endTime, file_list);
	}
}

void hknvr::SearchPlayback(const std::string & ip, const std::string & startTime, const std::string & endTime, std::list<FILE_RECORD_INFO>& file_list)
{
	LONG lChannel = GetPlayChannel(ip);
	if (lChannel <= 0)
	{
		return;
	}

	int year, month, day, hour, minute, second;
	sscanf_s(startTime.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);

	NET_DVR_FILECOND FileCond;
	FileCond.dwFileType = 0;
	FileCond.dwIsLocked = 0;
	FileCond.dwUseCardNo = 0;
	FileCond.lChannel = lChannel;

	FileCond.struStartTime.dwYear = year;
	FileCond.struStartTime.dwMonth = month;
	FileCond.struStartTime.dwDay = day;
	FileCond.struStartTime.dwHour = hour;
	FileCond.struStartTime.dwMinute = minute;
	FileCond.struStartTime.dwSecond = second;

	sscanf_s(endTime.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
	FileCond.struStopTime.dwYear = year;
	FileCond.struStopTime.dwMonth = month;
	FileCond.struStopTime.dwDay = day;
	FileCond.struStopTime.dwHour = hour;
	FileCond.struStopTime.dwMinute = minute;
	FileCond.struStopTime.dwSecond = second;

	LONG hFindHandle = NET_DVR_FindFile_V30(m_lUserID, &FileCond);
	if (-1 == hFindHandle)
	{
		return;
	}

	NET_DVR_FINDDATA_V30 FindData;
	int result = NET_DVR_FindNextFile_V30(hFindHandle, &FindData);

	while (result > 0)
	{
		if (NET_DVR_FILE_EXCEPTION == result ||
			NET_DVR_FILE_NOFIND == result ||
			NET_DVR_NOMOREFILE == result)
		{
			break;
		}

		else if (NET_DVR_ISFINDING == result)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		else if (NET_DVR_FILE_SUCCESS == result)
		{
			FILE_RECORD_INFO file;
			file.ip = ip;
			file.nvr_ip = m_LoginInfo.ip;
			file.file_name = FindData.sFileName;
			file.file_size = formatstr("%0.2fM", (double)FindData.dwFileSize / 1024 / 1024);

			file.start_time = formatstr("%04d-%02d-%02d %02d:%02d:%02d", FindData.struStartTime.dwYear, FindData.struStartTime.dwMonth, FindData.struStartTime.dwDay, \
				FindData.struStartTime.dwHour, FindData.struStartTime.dwMinute, FindData.struStartTime.dwSecond);

			file.end_time = formatstr("%04d-%02d-%02d %02d:%02d:%02d", FindData.struStopTime.dwYear, FindData.struStopTime.dwMonth, FindData.struStopTime.dwDay, \
				FindData.struStopTime.dwHour, FindData.struStopTime.dwMinute, FindData.struStopTime.dwSecond);

			file_list.push_back(file);
		}
		result = NET_DVR_FindNextFile_V30(hFindHandle, &FindData);
	}
	NET_DVR_FindClose_V30(hFindHandle);
}