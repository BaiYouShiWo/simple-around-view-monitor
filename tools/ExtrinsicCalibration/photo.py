# coding:utf-8
import cv2
cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
cap.set(3, 1920) # width=1920
cap.set(4, 1080) # height=1080
cap.set(cv2.CAP_PROP_FPS, 30)

cap.set(cv2.CAP_PROP_FOURCC,cv2.VideoWriter.fourcc('M','J','P','G'))
flag = cap.isOpened()
index = 0
while (flag):
    ret, frame = cap.read()
    cv2.imshow("Capture_Paizhao", frame)
    k = cv2.waitKey(1) & 0xFF
    if k == ord('s'):  # 按下s键，进入下面的保存图片操作
        cv2.imwrite("C:\\Users\\30570\\Desktop\\ins\\IntrinsicCalibration\\data\\img_raw" + str(index) + ".jpg", frame)
        print("save" + str(index) + ".jpg successfuly!")
        print("-------------------------")
        index += 1
    elif k == ord('q'):  # 按下q键，程序退出
        break
cap.release() # 释放摄像头
cv2.destroyAllWindows()# 释放并销毁窗口
