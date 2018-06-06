#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "detection.h"
#include "./kcf/tracker.h"

#include <dirent.h>

using namespace std;
using namespace cv;

int main()
{
    char *darknet_path = "./darknet";
    char *datacfg = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/voc.data";
    char *cfgfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/tiny-yolo.cfg";
    char *weightfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/tiny-yolo.backup";
    //char *datacfg = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/voc.data";
    //char *cfgfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/tiny-yolo.cfg";
    //char *weightfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/tiny-yolo_final.weights";
    double thresh = 0.05;//UAV 0.25
    double hier_thresh = 0.5;
    //char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/02.avi";
    //char *videofile = "/home/userver/Qt5.5.1/detect_track_0904_1021/video/result082401.avi";
    char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/result08240802.avi";
    //char *videofile = "/home/userver/Qt5.5.1/Examples/detect_track_0904_1011/video/split03.avi";
    // Tracker results
    Rect result;

    VideoCapture cap;
    Mat Camera_CImg, Camera_GImg, Process_Img;
    int framenum = 0;
    int track_flag = 0;
    char str[25];
    char *frame_file = "./frame.jpg";
    box detect_box;
    // the rect at center with size of 720x576
    const Rect detect_rect(1280/2-720/2, 720/2-576/2, 720, 576);

//    clock_t time;
    cap.open(videofile);
    if (!cap.isOpened()) {
        cout << "Can't open video!" << endl;
        return -1;
    }

    Detection detection(darknet_path, datacfg, cfgfile, weightfile);

    while (true)
    {
        cap >> Camera_CImg;

        if (Camera_CImg.empty()) {
            cout << "Video Ends: 4 0 0" << endl;
            break;
        }

        if(Camera_CImg.cols > 800){//image size:1280*720
            Process_Img = Camera_CImg(detect_rect);
        }
        else{//image size:720*576
            Process_Img= Camera_CImg;
        }

        framenum++;

        if(framenum == 122){

            int t = 1;
        }
//        cvtColor(Camera_CImg, Camera_GImg, CV_RGB2GRAY);
        // new detection
        if (track_flag == 0)
        {
            cout << "Enter detection: 3 0 0" << endl;
            
            // clock_t t;
            // t = clock();

            detect_box = detection.detect_image(Process_Img, frame_file, thresh, hier_thresh);
            //detect_box = detection.detect_image(Process_Img, thresh, hier_thresh);


            // printf("All elapsed time for detecting: %f seconds.\n", sec(clock()-t));

            
            // TODO create templete
            cout << "x:" << detect_box.x <<
                    " y:" << detect_box.y <<
                    " w:" << detect_box.w <<
                    " h:" << detect_box.h << endl;

            if(detect_box.h == 0 || detect_box.w == 0){
                cout << "Detection failed: 2 0 0" << endl;
            }
            else{
                result.x = (int)detect_box.x - (int)detect_box.w / 2 + 1;
                result.y = (int)detect_box.y - (int)detect_box.h / 2 + 1;
                result.width = (int)detect_box.w;
                result.height = (int)detect_box.h;

                result = tagEdgeExtraction(Process_Img, result);

                track_flag = kcfTrack(Process_Img, result, track_flag);
            }
        }
        
        // just track
        else 
        {
             track_flag = kcfTrack(Process_Img, result, track_flag);
        }
        
        sprintf(str, "%d", framenum);
        putText(Process_Img, str, Point(100,100), FONT_HERSHEY_PLAIN,
                2.0, Scalar(255, 255, 255), 1, 8, 0);
        imshow("input", Process_Img);
        printf("framenum = %d, i = %d\n", framenum, track_flag);
        if (cvWaitKey(20) == 'q')
            break;
    }

    return 0;
}
