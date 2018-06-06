#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "detection.h"
#include "./kcf/tracker.h"
#include "dataAssociation.h"

#include <dirent.h>

using namespace std;
using namespace cv;

char tagStatus[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int main()
{
    char *darknet_path = "./darknet";
    char *datacfg = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/voc.data";
    char *cfgfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/tiny-yolo.cfg";
    char *weightfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_oneClass/tiny-yolo.backup";
    //char *datacfg = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/voc.data";
    //char *cfgfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/tiny-yolo.cfg";
    //char *weightfile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/darknet/model/tiny-yolo_fineanchors/tiny-yolo_final.weights";
    double thresh = 0.15;//UAV 0.25
    double hier_thresh = 0.5;
    //char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/02.avi";
    //char *videofile = "/home/userver/Qt5.5.1/detect_track_0904_1021/video/result082401_0001.avi";
    char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/result082401_0004.avi";
    //char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/result08240801.avi";
    //char *videofile = "/home/userver/Qt5.5.1/Examples/detect_track_0904_1011/video/split03.avi";
    // Tracker results
    Rect result;

    int i, j;
    int obj_num = 0;
    box obj_rect[20];
    AREA curTag[20], befTag[40];

    char track_flag = 0, detect_flag = 0;
    char into_detect_flag = 0, into_track_flag = 0;
    int curNum = 0, befNum = 0;

    // show all boxes
    char box_IP[3];
    char key;
    int key_int;
    char track_IP;

    VideoCapture cap;
    Mat Camera_CImg, Process_Img;
    int framenum = 0;

    char str[25];
    char *frame_file = "./frame.jpg";
    // the rect at center with size of 720x576
    const Rect detect_rect(1280/2-720/2, 720/2-576/2+70, 720, 576);

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

        if(framenum == 11){
            int k = 0;
        }

        if(detect_flag == 0 && track_flag == 0){
            initData(curTag, befTag, tagStatus);
            detect_flag = 1;
            into_detect_flag = 1;
        }
        if(detect_flag == 1){
            obj_num = detection.detect_image(Process_Img, frame_file, thresh, hier_thresh, obj_rect);
            befNum = dataAssociation(befTag, befNum, curTag, obj_num, obj_rect, into_detect_flag);
            into_detect_flag = 0;

            for(i = 0; i < befNum; i++){
                if(befTag[i].IP > 0){
                    rectangle(Process_Img, Point(befTag[i].rect.x, befTag[i].rect.y ), Point( befTag[i].rect.x+befTag[i].rect.w, befTag[i].rect.y+befTag[i].rect.h), Scalar( 255, 0 , 0), 1, 8 );
                    sprintf(box_IP, "%d", befTag[i].IP - 1);
                    putText(Process_Img, box_IP, Point(befTag[i].rect.x, befTag[i].rect.y), FONT_HERSHEY_PLAIN, 1.0, Scalar(255, 0, 0), 1, 2, 0);
                }
            }

            key = cvWaitKey(3000);
            // char to int
            key_int = key - '0';
            if (key == 'q'){
                destroyAllWindows();
                break;
            }
            else if(0 <= key_int && tagStatus[key_int]){
                for(i = 0; i < befNum; i++){
                    if(befTag[i].IP == key_int + 1){
                        result.x = befTag[i].rect.x;
                        result.y = befTag[i].rect.y;
                        result.width = befTag[i].rect.w;
                        result.height = befTag[i].rect.h;

                        into_track_flag = 0;
                        track_flag = 1;
                        detect_flag = 0;

                        track_IP = i;
                        break;

                    }

                    else{
                        continue;
                    }
                }
            }
            else if(key_int != -49){
                cout << "Enter the wrong number!Please enter again!" << endl;
            }

        }

        if(track_flag == 1){
            track_flag = kcfTrack(Process_Img, result, into_track_flag);
            if(into_track_flag == 1 && track_flag == 1){
                sprintf(box_IP, "%d", befTag[track_IP].IP - 1);
                putText(Process_Img, box_IP, Point(result.x, result.y), FONT_HERSHEY_PLAIN, 1.0, Scalar(0, 0, 255), 1, 2, 0);
            }
            into_track_flag = 1;
        }




//        // new detection
//        if (track_flag == 0)
//        {
//            cout << "Enter detection: 3 0 0" << endl;
            
//            // clock_t t;
//            // t = clock();

//            //detect_box = detection.detect_image(Process_Img, frame_file, thresh, hier_thresh);//detect maximum probability target
//            //detect_box = detection.detect_image(Process_Img, thresh, hier_thresh);

//            obj_num = detection.detect_image(Process_Img, frame_file, thresh, hier_thresh, obj_rect);


//            // printf("All elapsed time for detecting: %f seconds.\n", sec(clock()-t));

            
//            // TODO create templete
//            cout << "x:" << obj_rect[0].x <<
//                    " y:" << obj_rect[0].y <<
//                    " w:" << obj_rect[0].w <<
//                    " h:" << obj_rect[0].h << endl;

//            if(obj_num == 0){
//                cout << "Detection failed: 2 0 0" << endl;
//            }
//            else{
//                result.x = obj_rect[0].x;
//                result.y = obj_rect[0].y;
//                result.width = obj_rect[0].w;
//                result.height = obj_rect[0].h;

//                //result = tagEdgeExtraction(Process_Img, result);

//                track_flag = kcfTrack(Process_Img, result, track_flag);
//            }
//        }
        
//        // just track
//        else
//        {
//             track_flag = kcfTrack(Process_Img, result, track_flag);
//        }
        
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
