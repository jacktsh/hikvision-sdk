#include "hkvideodownload.h"

using namespace hksdk;
hkvideodownload::hkvideodownload()
{
	m_lUserID = -1;
	m_hPlayback = -1;
	m_nProgress = 0;
	m_bCancel = FALSE;
}

hkvideodownload::~hkvideodownload()
{
	StopDownload();
}

void hkvideodownload::SetDestFile(const std::string& file)
{
	m_destFile = file;
}

void hkvideodownload::StartDownload()
{
	StopDownload();
	m_bCancel = FALSE;
	m_thread = std::thread(&hkvideodownload::DownloadThread, this);
}

void hkvideodownload::StopDownload()
{
	m_bCancel = TRUE;
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void hkvideodownload::DownloadThread()
{
	PreparePlayHandle();

	if (m_hPlayback < 0 ||
		!NET_DVR_PlayBackControl_V40(m_hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL))
	{
		m_nProgress = -1;
		return;
	}

	while (!m_bCancel)
	{
		m_nProgress = NET_DVR_GetDownloadPos(m_hPlayback); // 如返回200表明出现网络异常
		if (m_nProgress >= 100 || -1 == m_nProgress)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	if (m_nProgress == 200)
	{
		m_nProgress = -1;
	}

	NET_DVR_StopGetFile(m_hPlayback);
}

int hkvideodownload::GetProgress()
{
	return m_nProgress;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void hkvideodownloadByName::SetUserID(LONG lUserID)
{
	m_lUserID = lUserID;
}

void hkvideodownloadByName::SetSrcFile(const std::string &file)
{
	m_srcFile = file;
}

void hkvideodownloadByName::StartDownloadEx(LONG lUserID, const std::string& srcFile, const std::string& dstFile)
{
	m_lUserID = lUserID;
	m_srcFile = srcFile;
	m_destFile = dstFile;
	hkvideodownload::StartDownload();
}

void hkvideodownloadByName::PreparePlayHandle()
{
	m_hPlayback = NET_DVR_GetFileByName(m_lUserID, (char*)m_srcFile.c_str(), (char*)m_destFile.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
hkvideodownloadByTime::hkvideodownloadByTime()
{
	m_lChannel = -1;
}

void hkvideodownloadByTime::SetUserID(LONG lUserID)
{
	m_lUserID = lUserID;
}

void hkvideodownloadByTime::SetDownloadChannel(LONG lChannel)
{
	m_lChannel = lChannel;
}

void hkvideodownloadByTime::SetTime(const std::string& startTime, const std::string &endTime)
{
	m_startTime = startTime;
	m_endTime = endTime;
}

void hkvideodownloadByTime::StartDownloadEx(LONG lUserID, LONG lChannel, const std::string& startTime, const std::string &endTime, const std::string& dstFile)
{
	m_lUserID = lUserID;
	m_lChannel = lChannel;
	m_startTime = startTime;
	m_endTime = endTime;
	m_destFile = dstFile;
	hkvideodownload::StartDownload();
}

void hkvideodownloadByTime::PreparePlayHandle()
{
	int year, month, day, hour, minute, second;
	NET_DVR_PLAYCOND struDownloadCond = { 0 };
	struDownloadCond.dwChannel = m_lChannel;

	sscanf_s(m_startTime.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
	struDownloadCond.struStartTime.dwYear = year;
	struDownloadCond.struStartTime.dwMonth = month;
	struDownloadCond.struStartTime.dwDay = day;
	struDownloadCond.struStartTime.dwHour = hour;
	struDownloadCond.struStartTime.dwMinute = minute;
	struDownloadCond.struStartTime.dwSecond = second;

	sscanf_s(m_endTime.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
	struDownloadCond.struStopTime.dwYear = year;
	struDownloadCond.struStopTime.dwMonth = month;
	struDownloadCond.struStopTime.dwDay = day;
	struDownloadCond.struStopTime.dwHour = hour;
	struDownloadCond.struStopTime.dwMinute = minute;
	struDownloadCond.struStopTime.dwSecond = second;

	m_hPlayback = NET_DVR_GetFileByTime_V40(m_lUserID, (char*)m_destFile.c_str(), &struDownloadCond);
}