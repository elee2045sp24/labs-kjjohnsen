import mediapipe as mp
import cv2
import time

# simplification of syntax
BaseOptions = mp.tasks.BaseOptions
FaceDetector = mp.tasks.vision.FaceDetector
FaceDetectorOptions = mp.tasks.vision.FaceDetectorOptions
VisionRunningMode = mp.tasks.vision.RunningMode

model_path = 'models/blaze_face_short_range.tflite'
    
options = FaceDetectorOptions(
    base_options=BaseOptions(model_asset_path=model_path),
    running_mode=VisionRunningMode.VIDEO)
detector = FaceDetector.create_from_options(options)
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
    detector.close()
    cv2.destroyAllWindows() # closes the open gui window 

def process_frame(frame):
    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame)
    res = detector.detect_for_video(mp_image, int(time.time()*1000))
    
    # draw any detections
    for r in res.detections:
        b = r.bounding_box
        s = r.categories[0].score
        x1,y1 = (b.origin_x,b.origin_y)
        x2,y2 = ((x1+b.width),(y1+b.height))
        cv2.rectangle(frame,(x1,y1),(x2,y2),(255,0,0),2)
        cv2.putText(frame,f"{s:.2f}",(x1,y1),cv2.FONT_HERSHEY_SIMPLEX,1,(255,0,0),2)
if __name__=="__main__":
  main()

