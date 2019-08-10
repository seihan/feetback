import cv2

while True:
    k = cv2.waitKey(1) & 0xFF
    # press 'q' to exit
    if k == ord('q'):
        break
#    elif k == ord('b'):
        # change a variable / do something ...
#    elif k == ord('k'):
        # change a variable / do something ...
