# hikvision-sdk
海康威视网络录像机（NVR）和网络摄像机（IPC）的封装类

## 一、功能包括：

1 实时视频预览
> 实时获取码流数据，如果传入了显示句柄，则视频会显示在窗口句柄上。

2 按时间查询历史视频
> 选择一个IPC和时间段，返回历史视频列表。

3 历史视频按照文件名回放
> 根据查询到的历史视频，进行历史回放。

4 历史视频按照时间回放
> 根据选择的时间段，进行历史视频的回放。

5 历史视频播放控制：快放、慢放、正常播放、暂停、恢复、拖动进度
> 历史视频在播放过程中，可以进行控制

6 实时预览抓图
> 实时预览过程中可以抓拍图片

7 历史回放抓图
> 历史回放过程中可以抓拍图片

8 云台控制
> 实时预览过程中可以进行云台控制

9 云台预置点设置
> 预置点设置、运行和删除

10 实时预览远程喊话
> 如果IPC支持语音对讲，则可以进行远程喊话

## 二、支持的网络拓扑图

![](doc/image/典型应用.JPG)

