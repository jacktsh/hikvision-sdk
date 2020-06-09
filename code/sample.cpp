#include <iostream>
#include <iomanip>
using namespace std;


#include "hksdk\hkobjectfactory.h"

using namespace hksdk;
using namespace std;


void test_ipc(hkobjectfactory &factory)
{
	cout << "测试实时预览" << endl;
	shared_ptr<hknvr> nvr = factory.get_nvr("192.168.1.64");
	if (!nvr)
		return;

	shared_ptr<hkipc> ipc;
	ipc = factory.create_ipc(nvr, "192.168.254.3", "admin", "admin1234");
	ipc->StartRealPlay(GetConsoleWindow());


	Sleep(100000);
}

void test_download_by_time(hkobjectfactory &factory)
{
	cout << "测试按照时间下载录像" << endl;
	shared_ptr<hknvr> nvr = factory.get_nvr("192.168.1.64");
	if (!nvr)
		return;

	shared_ptr<hkvideodownload> download = factory.create_download(
		nvr,
		"192.168.254.3",
		"2020-05-26 10:00:00",
		"2020-05-26 10:10:00",
		"d:\\123.mp4");

	download->StartDownload();
	
	while (download->GetProgress() != 100)
	{
		cout << "下载进度 " << download->GetProgress() << '\r';
		Sleep(1000);
	}
	cout << "下载进度 " << download->GetProgress() << endl;
}

void test_download_by_name(hkobjectfactory &factory)
{
	cout << "测试按照文件名下载录像" << endl;
	shared_ptr<hknvr> nvr = factory.get_nvr("192.168.1.64");
	if (!nvr) 
		return;

	std::list<FILE_RECORD_INFO> file_list;

	nvr->SearchPlayback(
		list<string>({ "192.168.254.3" }),
		"2020-05-26 10:00:00",
		"2020-05-26 10:30:00",
		file_list
	);


	shared_ptr<hkvideodownload> download = factory.create_download(nvr, file_list.begin()->file_name, "d:\\abc.mp4");
	download->StartDownload();

	while (download->GetProgress() != 100)
	{
		cout << "下载进度 " << download->GetProgress()<< '\r';
		Sleep(1000);
	}
	cout << "下载进度 " << download->GetProgress() << endl;
}

void test_playback_byname(hkobjectfactory &factory)
{
	cout << "测试按照文件名播放历史视频" << endl;
	shared_ptr<hknvr> nvr = factory.get_nvr("192.168.1.64");
	if (!nvr)
		return;

	std::list<FILE_RECORD_INFO> file_list;
	nvr->SearchPlayback(
		list<string>({ "192.168.254.3" }),
		"2020-05-26 10:00:00",
		"2020-05-26 10:30:00",
		file_list
	);

	shared_ptr<hkvideoplayback> playback = factory.create_playback(nvr, file_list.begin()->file_name);

	playback->StartPlayback(GetConsoleWindow());

	while (!playback->PlaybackDone())
	{
		cout << playback->GetCurrentFrame() << "/" << playback->GetTotalFrames() << '\r';
	}
	cout << playback->GetCurrentFrame() << "/" << playback->GetTotalFrames() << '\r';
}

void test_playback_bytime(hkobjectfactory &factory)
{
	cout << "测试按照时间播放历史视频" << endl;
	shared_ptr<hknvr> nvr = factory.get_nvr("192.168.1.64");
	if (!nvr)
		return;

	
	shared_ptr<hkvideoplayback> playback = factory.create_playback(nvr, 
		"192.168.254.3",
		"2020-05-26 10:00:00",
		"2020-05-26 10:00:20");

	playback->StartPlayback(GetConsoleWindow());

	while (!playback->PlaybackDone())
	{
		cout << playback->GetCurrentFrame() << "/" << playback->GetTotalFrames() << '\r';
	}
	cout << playback->GetCurrentFrame() << "/" << playback->GetTotalFrames() << '\r';
}

int main()
{
	hksdk::sdk_init();

	// 登录NVR
	hkobjectfactory factory;
	factory.login_nvr("192.168.1.64", "admin", "admin1234");

	test_ipc(factory);
	test_download_by_time(factory);
	test_download_by_name(factory);
	test_playback_bytime(factory);
	test_playback_byname(factory);
	
	hksdk::sdk_cleanup();
	return 0;
}
