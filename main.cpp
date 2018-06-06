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
    //char *datacfg = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/voc.data";
    //char *cfgfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/tiny-yolo.cfg";
   // char *weightfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/tiny-yolo.backup";
    char *datacfg = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/voc.data";
    char *cfgfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/tiny-yolo.cfg";
    char *weightfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/tiny-yolo_final.weights";
    double thresh = 0.25;
    double hier_thresh = 0.5;
    //char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/02.avi";
    char *videofile = "/home/userver/darknet/51.avi";

    // Tracker results
    Rect result;
    int update_num = 0;

    VideoCapture cap;
    Mat Camera_CImg, Camera_GImg; //sub_img;
    int framenum = 0;
    int track_flag = 0;
    char str[25];
    char *frame_file = "./frame.jpg";
    box detect_box;
    // the rect at center with size of 720x576
    //const Rect center_rect(1280/2-720/2, 720/2-576/2, 720, 576);

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

        framenum++;

        if(framenum == 250){
            int t = 1;
        }
//        cvtColor(Camera_CImg, Camera_GImg, CV_RGB2GRAY);
        // new detection
        if (track_flag == 0)
        {
            cout << "Enter detection: 3 0 0" << endl;
            
           // clock_t t;
           // t = clock();

            detect_box = detection.detect_image(Camera_CImg, frame_file, thresh, hier_thresh);


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

                result = tagEdgeExtraction(Camera_CImg, result);

                track_flag = kcfTrack(Camera_CImg, result, track_flag);
            }
        }
        
        // just track
        else 
        {
             track_flag = kcfTrack(Camera_CImg, result, track_flag);
        }
        
        sprintf(str, "%d", framenum);
        putText(Camera_CImg, str, Point(100,100), FONT_HERSHEY_PLAIN,
                2.0, Scalar(255, 255, 255), 1, 8, 0);
        imshow("input", Camera_CImg);
        printf("framenum = %d, i = %d\n", framenum, track_flag);
        if (cvWaitKey(20) == 'q')
            break;
    }

    return 0;
}
