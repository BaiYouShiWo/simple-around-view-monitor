# simple-around-view-monitor
## 一个简单的四摄像头360环视系统，由cpp实现。
查看了网上各个环视项目，发现众多算法都是python结合opencv实现，在此结合前人智慧，用cpp简单地拼凑了一下优秀的项目。  
目的大抵就是解决前人环视算法实现时提到的性能问题。但实现过程终究是opencv调包，故更换语言对于性能提升其实帮助不大😅。  
鉴于opencv中的对于摄像头的读取操作`` VideoCapture.read() ``是一个阻塞的操作，所以在多摄像头读取任务中使用多线程可略微提速。
# 相关项目:
## 1.(Camera Calibration)[https://github.com/dyfcalid/CameraCalibration]
## 2.(surround-view-system-introduction)[https://github.com/neozhaoliang/surround-view-system-introduction]
