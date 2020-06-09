#include "hksdk.h"
#include "HCNetSDK.h"

#pragma comment(lib, "HCNetSDK.lib")
#pragma comment(lib, "PlayCtrl.lib")

void hksdk::sdk_init()
{
	NET_DVR_Init();
}

void hksdk::sdk_cleanup()
{
	NET_DVR_Cleanup();
}