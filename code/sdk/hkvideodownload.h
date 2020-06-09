#ifndef _HK_VIDEO_DOWNLOAD_H_
#define _HK_VIDEO_DOWNLOAD_H_
#include "hksdk.h"

namespace hksdk
{
	class hkvideodownload
	{
	public:
		hkvideodownload();
		virtual ~hkvideodownload();
		
		void StartDownload();
		void StopDownload();

		int GetProgress();

		void SetDestFile(const std::string& file);
	protected:
		virtual void PreparePlayHandle()=0;
		void DownloadThread();

	protected:
		LONG m_lUserID;
		LONG m_hPlayback;
		int m_nProgress;
		BOOL m_bCancel;
		std::string m_destFile;
		std::thread m_thread;
	};

	class hkvideodownloadByName :
		public hkvideodownload
	{
	public:
		void SetUserID(LONG lUserID);
		void SetSrcFile(const std::string &file);
		
		void StartDownloadEx(LONG lUserID, const std::string& srcFile, const std::string& dstFile);

	protected:
		virtual void PreparePlayHandle();

	protected:
		std::string m_srcFile;
	};

	class hkvideodownloadByTime :
		public hkvideodownload
	{
	public:
		hkvideodownloadByTime();
		void SetUserID(LONG lUserID);
		void SetDownloadChannel(LONG lChannel);
		void SetTime(const std::string& startTime, const std::string &endTime);
		void StartDownloadEx(LONG lUserID, LONG lChannel, const std::string& startTime, const std::string &endTime, const std::string& dstFile);
	
	protected:
		virtual void PreparePlayHandle();

	protected:
		LONG m_lChannel;
		std::string m_startTime;
		std::string m_endTime;
	};
};

#endif
