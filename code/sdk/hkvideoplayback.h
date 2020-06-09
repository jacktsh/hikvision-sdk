#ifndef _HK_VIDEO_PLAYBACK_H
#define _HK_VIDEO_PLAYBACK_H
#include "hksdk.h"

namespace hksdk
{
	class hkvideoplayback
	{
	public:
		hkvideoplayback();
		virtual ~hkvideoplayback();

		BOOL StartPlayback(HWND hwnd=0);
		void StopPlayback();

		char* GetImageData();
		int GetImageWidth();
		int GetImageHeight();

		DWORD GetCurrentFrame();
		DWORD GetTotalFrames();

		void Normal();
		void Fast();
		void Slow();
		void Pause();
		void Resume();
		void SetPos(int pos);

		BOOL Snapshot(const std::string& fileName);
		BOOL PlaybackDone();

	protected:
		static void PlayDataCallBack(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser);
		static void DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
		void FreePlayLib();
		void FreeImageData();
		void Register();
		void UnRegister();
		virtual void PreParePlayHandle() = 0;
		
	protected:
		LONG m_lPlayHandle;
		LONG m_lUserID;
		HWND m_hPlayWnd;
		LONG m_lPort;
		DWORD m_dwCurrentFrame;
		DWORD m_dwTotalFrames;
		DWORD m_dwTotalTime;
		char* m_ImageData;
		int m_nImageWidth;
		int m_nImageHeight;
		static std::map<long, hkvideoplayback*> g_Port2Ptr;
	};

	class hkvideoplaybackByName :
		public hkvideoplayback
	{
	public:
		void SetUserID(LONG lUserID);
		void SetPlaybackFile(const std::string& file);
		BOOL StartPlaybackEx(LONG lUserID, const std::string &fileName, HWND hWnd = 0);

	private:
		virtual void PreParePlayHandle();

	private:
		std::string m_fileName;
	};

	class hkvideoplaybackByTime :
		public hkvideoplayback
	{
	public:
		void SetUserID(LONG lUserID);
		void SetPlayChannel(LONG lChannel);
		void SetTime(const std::string &startTime, const std::string &endTime);

		BOOL StartPlaybackEx(LONG lUserID, LONG lChannel, const std::string &startTime, const std::string &endTime, HWND hWnd = 0);
	private:
		virtual void PreParePlayHandle();

	private:
		std::string m_startTime;
		std::string m_endTime;
		LONG m_lChannel;
	};
};
#endif

