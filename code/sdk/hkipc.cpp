#include "hkipc.h"
using namespace hksdk;

std::map<long, hkipc*> hkipc::g_Port2Ptr;
hkipc::hkipc()
{
	m_lPlayHandle = -1;
	m_lVoiceHanle = -1;
	m_lUserID = -1;
	m_lPort = -1;
	m_lPlayChannel = -1;
	m_lVoiceChannel = -1;
	m_hPlayWnd = NULL;
	m_ImageData = NULL;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
	m_bDontLogout = FALSE;
	m_lStreamType = 1;
}

hkipc::~hkipc()
{
	StopVoice();
	StopRealPlay();
	UnRegister();
	FreePlayLib();
	FreeImageData();
	Logout();
}

BOOL hkipc::Login(const std::string & ip, const std::string & user, const std::string & pwd, int port)
{
	m_LoginInfo.ip = ip;
	m_LoginInfo.user = user;
	m_LoginInfo.pwd = pwd;
	m_LoginInfo.port = port;
	m_lPlayChannel = 1;
	m_lVoiceChannel = 1;
	m_bDontLogout = FALSE;
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
	return TRUE;
}

void hkipc::Logout()
{
	if (!m_bDontLogout && m_lUserID >= 0)
	{
		NET_DVR_Logout(m_lUserID);
		m_lUserID = -1;
	}
}

void hkipc::VoiceDataCallBack(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, void*pUser)
{
	//printf("receive voice data, %d\n", dwBufSize);
}

BOOL hkipc::StartVoice()
{
	m_lVoiceHanle = NET_DVR_StartVoiceCom_V30(m_lUserID, m_lVoiceChannel, 0, VoiceDataCallBack, NULL);
	return m_lVoiceHanle >= 0;
}

void hkipc::StopVoice()
{
	if (m_lVoiceHanle >= 0)
	{
		NET_DVR_StopVoiceCom(m_lVoiceHanle);
		m_lVoiceHanle = -1;
	}
}

BOOL hkipc::StartRealPlay(HWND hWnd)
{
	m_hPlayWnd = hWnd;
	NET_DVR_PREVIEWINFO struPlayInfo{};
	struPlayInfo.hPlayWnd = hWnd;
	struPlayInfo.lChannel = m_lPlayChannel;
	struPlayInfo.dwStreamType = m_lStreamType;
	struPlayInfo.dwLinkMode = 0;
	struPlayInfo.bBlocked = 1;
	
	m_lPlayHandle = NET_DVR_RealPlay_V40(m_lUserID, &struPlayInfo, &hkipc::RealDataCallBack, this);
	if (m_lPlayHandle < 0)
	{
		DWORD dwErr = NET_DVR_GetLastError();
		return FALSE;
	}
	return TRUE;
}

void hkipc::StopRealPlay()
{
	if (m_lPlayHandle >= 0)
	{
		NET_DVR_StopRealPlay(m_lPlayHandle);
		m_lPlayHandle = -1;
	}
}

void hkipc::RealDataCallBack(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;
	hkipc* ipc = (hkipc*)pUser;
	ipc->m_time_point = std::chrono::steady_clock::now();

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		{
			if (ipc->m_lPort >= 0)
			{
				break;
			}

			PlayM4_GetPort(&ipc->m_lPort);
			PlayM4_SetStreamOpenMode(ipc->m_lPort, STREAME_REALTIME);
			PlayM4_OpenStream(ipc->m_lPort, pBuffer, dwBufSize, 6000 * 6000);
			PlayM4_SetDecCallBack(ipc->m_lPort, &hkipc::DecCBFun);
			PlayM4_Play(ipc->m_lPort, NULL);
			ipc->Register();
		}
		break;

	case NET_DVR_STREAMDATA:
		for (int i = 0; i < 1000; i++)
		{
			inData = PlayM4_InputData(ipc->m_lPort, pBuffer, dwBufSize);
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
			inData = PlayM4_InputData(ipc->m_lPort, pBuffer, dwBufSize);
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

void hkipc::DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	//std::cout << "realtime-->" << " port:" << nPort << " nWidth: " << pFrameInfo->nWidth << " nHeight: " << pFrameInfo->nHeight << " frame num:" << pFrameInfo->dwFrameNum << std::endl;
	hkipc* ipc = g_Port2Ptr[nPort];
	
	if (!ipc || pFrameInfo->nType != T_YV12)
	{
		return;
	}

	if (ipc->m_ImageData == NULL ||
		ipc->m_nImageWidth != pFrameInfo->nWidth ||
		ipc->m_nImageHeight != pFrameInfo->nHeight)
	{
		ipc->FreeImageData();
		ipc->m_ImageData = (char*)malloc(nSize);
		ipc->m_nImageWidth = pFrameInfo->nWidth;
		ipc->m_nImageHeight = pFrameInfo->nHeight;
	}

	memcpy(ipc->m_ImageData, pBuf, ipc->m_nImageWidth * ipc->m_nImageHeight); // y
	memcpy(ipc->m_ImageData + ipc->m_nImageWidth * ipc->m_nImageHeight, pBuf + ipc->m_nImageWidth * ipc->m_nImageHeight * 5 / 4, ipc->m_nImageWidth * ipc->m_nImageHeight / 4); // u
	memcpy(ipc->m_ImageData + ipc->m_nImageWidth * ipc->m_nImageHeight * 5 / 4, pBuf + ipc->m_nImageWidth * ipc->m_nImageHeight, ipc->m_nImageWidth * ipc->m_nImageHeight / 4); // v
}

BOOL hkipc::PTZControl(DWORD dwPTZCommand, DWORD dwStartOrStop)
{
	// dwPTZCommand:
	// TILT_UP 		21 云台上仰
	// TILT_DOWN 	22 云台下俯
	// PAN_LEFT 	23 云台左转
	// PAN_RIGHT 	24 云台右转
	// UP_LEFT 		25 云台上仰和左转
	// UP_RIGHT 	26 云台上仰和右转
	// DOWN_LEFT 	27 云台下俯和左转
	// DOWN_RIGHT 	28 云台下俯和右转

	// dwStartOrStop: start-0, stop-1
	if (m_lPlayHandle >= 0)
		return NET_DVR_PTZControl(m_lPlayHandle, dwPTZCommand, dwStartOrStop);
	else
		return NET_DVR_PTZControl_Other(m_lUserID, m_lPlayChannel, dwPTZCommand, dwStartOrStop);
}

BOOL hkipc::PTZPreset(DWORD dwPTZPresetCmd, DWORD dwPresetIndex)
{
	// dwPTZPresetCmd:
	// SET_PRESET	8 设置预置点
	// CLE_PRESET	9 清除预置点
	// GOTO_PRESET 39 转到预置点

	if (m_lPlayHandle >= 0)
		return NET_DVR_PTZPreset(m_lPlayHandle, dwPTZPresetCmd, dwPresetIndex);
	else
		return NET_DVR_PTZPreset_Other(m_lUserID, m_lPlayChannel, dwPTZPresetCmd, dwPresetIndex);
}

BOOL hkipc::Snapshot(const std::string& fileName)
{
	NET_DVR_JPEGPARA jpegPara;
	jpegPara.wPicSize = 0xff;
	jpegPara.wPicQuality = 0;
	return NET_DVR_CaptureJPEGPicture(m_lUserID, m_lPlayChannel, &jpegPara, (char*)fileName.c_str());
}

char* hkipc::GetImageData()
{
	return m_ImageData;
}

int hkipc::GetImageWidth()
{
	return m_nImageWidth;
}

int hkipc::GetImageHeight()
{
	return m_nImageHeight;
}

void hkipc::SetStreamType(LONG lStreamType)
{
	m_lStreamType = lStreamType;
}

void hkipc::FreePlayLib()
{
	if (m_lPort >= 0)
	{
		PlayM4_Stop(m_lPort);
		PlayM4_CloseStream(m_lPort);
		PlayM4_FreePort(m_lPort);
		m_lPort = -1;
	}
}

void hkipc::FreeImageData()
{
	free(m_ImageData);
	m_ImageData = NULL;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
}

void hkipc::Register()
{
	g_Port2Ptr[m_lPort] = this;
}

void hkipc::UnRegister()
{
	if (g_Port2Ptr.find(m_lPort) != g_Port2Ptr.end())
	{
		g_Port2Ptr.erase(m_lPort);
	}
}

BOOL hkipc::GetStatus()
{
	auto diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_time_point);
	return diff.count() <= 10;
}

///////////////////////////////////////////////////////////////////
void hkipcEx::SetUserID(LONG lUserID)
{
	m_lUserID = lUserID;
	m_bDontLogout = TRUE;
}

void hkipcEx::SetPlayChannel(LONG lChannel)
{
	m_lPlayChannel = lChannel;
}

void hkipcEx::SetVoiceChannel(LONG lChannel)
{
	m_lVoiceChannel = lChannel;
}

BOOL hkipcEx::StartRealPlayEx(LONG lUserID, LONG lChannel, HWND hWnd)
{
	m_lUserID = lUserID;
	m_lPlayChannel = lChannel;
	m_bDontLogout = TRUE;
	return hkipc::StartRealPlay(hWnd);
}

BOOL hkipcEx::StartVoiceEx(LONG lUserID, LONG lChannel)
{
	m_lUserID = lUserID;
	m_lVoiceChannel = lChannel;
	m_bDontLogout = TRUE;
	return hkipc::StartVoice();
}