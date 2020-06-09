#ifndef _HKIPC_H_
#define _HKIPC_H_
#include "hksdk.h"

namespace hksdk
{
	class hkipc
	{
	public:
		hkipc();
		virtual ~hkipc();

		BOOL Login(const std::string &ip, const std::string &user, const std::string &pwd, int port = 8000);
		void Logout();

		void SetStreamType(LONG lStreamType);

		BOOL StartRealPlay(HWND hWnd=0);
		void StopRealPlay();

		BOOL StartVoice();
		void StopVoice();

		BOOL GetStatus();
		BOOL PTZControl(DWORD dwPTZCommand, DWORD dwStartOrStop);
		BOOL PTZPreset(DWORD dwPTZPresetCmd, DWORD dwPresetIndex);
		BOOL Snapshot(const std::string& fileName);

		char* GetImageData();
		int GetImageWidth();
		int GetImageHeight();

	protected:
		static void RealDataCallBack(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser);
		static void DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
		static void VoiceDataCallBack(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, void*pUser);

		void FreePlayLib();
		void FreeImageData();
		void Register();
		void UnRegister();

	protected:
		LOGIN_INFO m_LoginInfo;
		LONG m_lPlayHandle;
		LONG m_lVoiceHanle;
		LONG m_lUserID;
		HWND m_hPlayWnd;
		LONG m_lPlayChannel;
		LONG m_lVoiceChannel;
		LONG m_lPort;
		LONG m_lStreamType;
		BOOL m_bDontLogout;

		char* m_ImageData;
		int m_nImageWidth;
		int m_nImageHeight;
		std::chrono::time_point<std::chrono::steady_clock> m_time_point;
		static std::map<long, hkipc*> g_Port2Ptr;
	};

	class hkipcEx : 
		public hkipc
	{
	public:
		void SetUserID(LONG lUserID);
		void SetPlayChannel(LONG lChannel);
		void SetVoiceChannel(LONG lChannel);

		BOOL StartRealPlayEx(LONG lUserID, LONG lChannel, HWND hWnd = 0);
		BOOL StartVoiceEx(LONG lUserID, LONG lChannel);

	private:
		BOOL Login(const std::string &ip, const std::string &user, const std::string &pwd, int port = 8000) = delete;
	};
};
#endif