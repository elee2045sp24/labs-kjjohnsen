import cv2 # cv2 is a newer interface to opencv
def main():
    cap = cv2.VideoCapture(0) # 0 is an index to the first camera
    while cap.isOpened():
        res, frame = cap.read() # ret will indicate success or failure, frame is a numpy array
        if not res:
            continue # no frame read
        process_frame(frame)
        cv2.imshow("my window", frame) # quick gui
        if cv2.waitKey(1) == ord('q'):
            break #exit this loop
    cap.release() # stop the camera
    cv2.destroyAllWindows() # closes the open gui window 
def process_frame(frame):
  print(frame.shape)

main()
