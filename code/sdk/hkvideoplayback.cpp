#include "hkvideoplayback.h"

using namespace hksdk;
std::map<long, hkvideoplayback*> hkvideoplayback::g_Port2Ptr;

hkvideoplayback::hkvideoplayback()
{
	m_lPlayHandle = -1;
	m_lUserID = -1;
	m_lPort = -1;
	m_hPlayWnd = NULL;
	m_dwCurrentFrame = 0;
	m_dwTotalFrames = 0;
	m_dwTotalTime = 0;
	m_ImageData = NULL;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
}

hkvideoplayback::~hkvideoplayback()
{
	UnRegister();
	StopPlayback();
	FreePlayLib();
	FreeImageData();
}

BOOL hkvideoplayback::StartPlayback(HWND hwnd)
{
	m_hPlayWnd = hwnd;

	PreParePlayHandle();
	if (m_lPlayHandle < 0)
	{
		return FALSE;
	}

	NET_DVR_SetPlayDataCallBack_V40(m_lPlayHandle, &hkvideoplayback::PlayDataCallBack, this);
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTART, 0, NULL);
	return TRUE;
}

void hkvideoplayback::StopPlayback()
{
	if (m_lPlayHandle >= 0)
	{
		NET_DVR_StopPlayBack(m_lPlayHandle);
		m_lPlayHandle = -1;
	}
}

void hkvideoplayback::PlayDataCallBack(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;
	hkvideoplayback* playback = (hkvideoplayback*)pUser;
	
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
	{
		if (playback->m_lPort >= 0)
		{
			break;
		}

		PlayM4_GetPort(&playback->m_lPort);
		PlayM4_SetStreamOpenMode(playback->m_lPort, STREAME_FILE);
		PlayM4_OpenStream(playback->m_lPort, pBuffer, dwBufSize, 6000 * 1000);
		PlayM4_SetDecCallBack(playback->m_lPort, &hkvideoplayback::DecCBFun);
		PlayM4_Play(playback->m_lPort, NULL);
		playback->Register();
	}
	break;

	case NET_DVR_STREAMDATA:
		for (int i = 0; i < 1000; i++)
		{
			inData = PlayM4_InputData(playback->m_lPort, pBuffer, dwBufSize);
			if (!inData)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
			else
			{
				break;
			}
		}
		break;

	default:
		for (int i = 0; i < 1000; i++)
		{
			inData = PlayM4_InputData(playback->m_lPort, pBuffer, dwBufSize);
			if (!inData)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
			else
			{
				break;
			}
		}
		break;
	}
}

void hkvideoplayback::DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	//std::cout << "history-->" << " port:" << nPort << " nWidth: " << pFrameInfo->nWidth << " nHeight: " << pFrameInfo->nHeight << " frame num:" << pFrameInfo->dwFrameNum << std::endl;
	hkvideoplayback* playback = g_Port2Ptr[nPort];

	if (!playback || pFrameInfo->nType != T_YV12)
	{
		return;
	}

	if (!playback->m_dwTotalFrames)
	{
		playback->m_dwTotalFrames = playback->m_dwTotalTime * pFrameInfo->nFrameRate;
	}

	playback->m_dwCurrentFrame = pFrameInfo->dwFrameNum;

	if (playback->m_ImageData == NULL ||
		playback->m_nImageWidth != pFrameInfo->nWidth ||
		playback->m_nImageHeight != pFrameInfo->nHeight)
	{
		playback->FreeImageData();
		playback->m_ImageData = (char*)malloc(nSize);
		playback->m_nImageWidth = pFrameInfo->nWidth;
		playback->m_nImageHeight = pFrameInfo->nHeight;
	}

	memcpy(playback->m_ImageData, pBuf, playback->m_nImageWidth * playback->m_nImageHeight); // y
	memcpy(playback->m_ImageData + playback->m_nImageWidth * playback->m_nImageHeight, pBuf + playback->m_nImageWidth * playback->m_nImageHeight * 5 / 4, playback->m_nImageWidth * playback->m_nImageHeight / 4); // u
	memcpy(playback->m_ImageData + playback->m_nImageWidth * playback->m_nImageHeight * 5 / 4, pBuf + playback->m_nImageWidth * playback->m_nImageHeight, playback->m_nImageWidth * playback->m_nImageHeight / 4); // v
}

char* hkvideoplayback::GetImageData()
{
	return m_ImageData;
}

int hkvideoplayback::GetImageWidth()
{
	return m_nImageWidth;
}

int hkvideoplayback::GetImageHeight()
{
	return m_nImageHeight;
}

DWORD hkvideoplayback::GetCurrentFrame()
{
	return m_dwCurrentFrame;
}

DWORD hkvideoplayback::GetTotalFrames()
{
	return m_dwTotalFrames;
}

void hkvideoplayback::Normal()
{
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYNORMAL, 0, NULL);
	PlayM4_Play(m_lPort, NULL);
}

void hkvideoplayback::Fast()
{
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYFAST, 0, NULL);
	PlayM4_Fast(m_lPort);
}

void hkvideoplayback::Slow()
{
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSLOW, 0, NULL);
	PlayM4_Slow(m_lPort);
}

void hkvideoplayback::Pause()
{
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYPAUSE, 0, NULL);
	PlayM4_Pause(m_lPort, 1);
}

void hkvideoplayback::Resume()
{
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYRESTART, 0, NULL);
	PlayM4_Pause(m_lPort, 0);
}

void hkvideoplayback::SetPos(int pos)
{
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSETPOS, pos, NULL);
	PlayM4_ResetSourceBuffer(m_lPort);
}

BOOL hkvideoplayback::Snapshot(const std::string& fileName)
{
	if (!m_ImageData)
	{
		return FALSE;
	}

	char *jpegbuffer = (char*)malloc(m_nImageWidth * m_nImageHeight*3/2);
	memcpy(jpegbuffer, m_ImageData, m_nImageWidth * m_nImageHeight); // y
	memcpy(jpegbuffer + m_nImageWidth * m_nImageHeight, m_ImageData + m_nImageWidth * m_nImageHeight * 5 / 4, m_nImageWidth * m_nImageHeight / 4); // v
	memcpy(jpegbuffer + m_nImageWidth * m_nImageHeight * 5 / 4, m_ImageData + m_nImageWidth * m_nImageHeight, m_nImageWidth * m_nImageHeight / 4); // u

	PlayM4_ConvertToJpegFile(jpegbuffer, m_nImageWidth * m_nImageHeight*3/2, m_nImageWidth,
		m_nImageHeight, T_YV12, (char*)fileName.c_str());

	free(jpegbuffer);
	return TRUE;
}

BOOL hkvideoplayback::PlaybackDone()
{
	DWORD dwPos = 0;
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYGETPOS, 0, &dwPos);
	if (dwPos >= 100)
	{
		m_dwCurrentFrame = m_dwTotalFrames; // just in case: playback by time
		return TRUE;
	}
	return FALSE;
}

void hkvideoplayback::FreePlayLib()
{
	if (m_lPort >= 0)
	{
		PlayM4_Stop(m_lPort);
		PlayM4_CloseStream(m_lPort);
		PlayM4_FreePort(m_lPort);
		m_lPort = -1;
	}
}

void hkvideoplayback::FreeImageData()
{
	free(m_ImageData);
	m_ImageData = NULL;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
}

void hkvideoplayback::Register()
{
	g_Port2Ptr[m_lPort] = this;
}

void hkvideoplayback::UnRegister()
{
	if (g_Port2Ptr.find(m_lPort) != g_Port2Ptr.end())
	{
		g_Port2Ptr.erase(m_lPort);
	}
}

//////////////////////////////////////////////////////////////////////////

void hkvideoplaybackByName::SetUserID(LONG lUserID)
{
	m_lUserID = lUserID;
}

void hkvideoplaybackByName::SetPlaybackFile(const std::string& file)
{
	m_fileName = file;
}

BOOL hkvideoplaybackByName::StartPlaybackEx(LONG lUserID, const std::string &fileName, HWND hWnd)
{
	m_lUserID = lUserID;
	m_fileName = fileName;
	return hkvideoplayback::StartPlayback(hWnd);
}

void hkvideoplaybackByName::PreParePlayHandle()
{
	m_lPlayHandle = NET_DVR_PlayBackByName(m_lUserID, (char*)m_fileName.c_str(), m_hPlayWnd);
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_GETTOTALFRAMES, 0, &m_dwTotalFrames);
	NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_GETTOTALTIME, 0, &m_dwTotalTime);
}

////////////////////////////////////////////////////////////////////////////

void hkvideoplaybackByTime::SetUserID(LONG lUserID)
{
	m_lUserID = lUserID;
}

void hkvideoplaybackByTime::SetPlayChannel(LONG lChannel)
{
	m_lChannel = lChannel;
}

void hkvideoplaybackByTime::SetTime(const std::string &startTime, const std::string &endTime)
{
	m_startTime = startTime;
	m_endTime = endTime;
}

BOOL hkvideoplaybackByTime::StartPlaybackEx(LONG lUserID, LONG lChannel, const std::string &startTime, const std::string &endTime, HWND hWnd)
{
	m_lUserID = lUserID;
	m_lChannel = lChannel;
	m_startTime = startTime;
	m_endTime = endTime;
	return hkvideoplayback::StartPlayback(hWnd);
}

void hkvideoplaybackByTime::PreParePlayHandle()
{
	int year, month, day, hour, minute, second;
	NET_DVR_VOD_PARA struVodPara = { 0 };
	struVodPara.dwSize = sizeof(struVodPara);
	struVodPara.struIDInfo.dwSize = sizeof(NET_DVR_STREAM_INFO);
	struVodPara.struIDInfo.dwChannel = m_lChannel;
	struVodPara.hWnd = m_hPlayWnd;

	sscanf_s(m_startTime.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
	struVodPara.struBeginTime.dwYear = year;
	struVodPara.struBeginTime.dwMonth = month;
	struVodPara.struBeginTime.dwDay = day;
	struVodPara.struBeginTime.dwHour = hour;
	struVodPara.struBeginTime.dwMinute = minute;
	struVodPara.struBeginTime.dwSecond = second;

	sscanf_s(m_endTime.c_str(), "%4d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
	struVodPara.struEndTime.dwYear = year;
	struVodPara.struEndTime.dwMonth = month;
	struVodPara.struEndTime.dwDay = day;
	struVodPara.struEndTime.dwHour = hour;
	struVodPara.struEndTime.dwMinute = minute;
	struVodPara.struEndTime.dwSecond = second;

	// caculate total frame
	auto l = [](const char * szTime)
	{
		struct tm tm_;
		time_t time_;
		sscanf_s(szTime, "%4d-%02d-%02d %02d:%02d:%02d",
			&tm_.tm_year,
			&tm_.tm_mon,
			&tm_.tm_mday,
			&tm_.tm_hour,
			&tm_.tm_min,
			&tm_.tm_sec);
		tm_.tm_year -= 1900;
		tm_.tm_mon--;
		tm_.tm_isdst = -1;

		time_ = mktime(&tm_);
		return time_;
	};

	m_lPlayHandle = NET_DVR_PlayBackByTime_V40(m_lUserID, &struVodPara);

	std::chrono::time_point<std::chrono::system_clock> tp1 = std::chrono::system_clock::from_time_t(l(m_startTime.c_str()));
	std::chrono::time_point<std::chrono::system_clock> tp2 = std::chrono::system_clock::from_time_t(l(m_endTime.c_str()));
	auto diff = std::chrono::duration_cast<std::chrono::seconds>(tp2 - tp1);
	m_dwTotalTime = (DWORD)diff.count();

	if (!m_hPlayWnd) // very weird: will crash without calling the fowlling code
	{
		LONG nPort = -1;
		PlayM4_GetPort(&nPort);
		PlayM4_FreePort(nPort);
	}
}
