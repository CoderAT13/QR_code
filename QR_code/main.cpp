//
//  main.cpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/8.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//
/*
#include "QR_detect.hpp"
using namespace cv;
using namespace std;



int main(int argc, char** argv) {
    //srand(2131231);
    Mat src = imread("QR_rotated.png");

    QR_detecter test;
    test.detect(src);
    test.showCorners(0.1);

    return 0;
}
*/
//______________________________________________________________________________________
// Program : OpenCV based QR code Detection and Retrieval
// Author  : Bharath Prabhuswamy
//______________________________________________________________________________________

/*
 * 潜在问题：
 * 1. 提取时可能出现旋转矩形越界问题
 * 2. 由真实图片转换过来的二维码，局限于正面拍摄识别（即image中的定位框必须是正方形的）
 * 3. 参考 https://github.com/bharathp666/opencv_qr.git
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;


Mat transformCorner(Mat &image, RotatedRect &rect);

// Start of Main Loop
//------------------------------------------------------------------------------------------------------------------------
int main ( int argc, char **argv )
{


    Mat image = imread("final2.png");
    resize(image, image, Size(image.cols/2,image.rows/2));
    if(image.empty()){ cerr << "ERR: Unable to find image.\n" << endl;
        return -1;
    }


    // Creation of Intermediate 'Image' Objects required later
    Mat gray(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
    Mat edges(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
    Mat traces(image.size(), CV_8UC3);                                // For Debug Visuals
    Mat qr,qr_raw,qr_gray,qr_thres;

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    int mark;
    int marks[100];
    int key = 0;
    while(key != 'q')                // While loop to query for Image Input frame
    {
        traces = Scalar(0,0,0);
        qr_raw = Mat::zeros(100, 100, CV_8UC3 );
        qr = Mat::zeros(100, 100, CV_8UC3 );
        qr_gray = Mat::zeros(100, 100, CV_8UC1);
        qr_thres = Mat::zeros(100, 100, CV_8UC1);

        // capture >> image;                // For Video input        // Capture Image from Image Input

        cvtColor(image,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
        Canny(gray, edges, 100 , 200, 3);        // Apply Canny edge detection on the gray image


        findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE); // Find contours with hierarchy

        mark = 0;                                // Reset all detected marker count for this frame

        // Get Moments for all Contours and the mass centers
        vector<Moments> mu(contours.size());
        vector<Point2f> mc(contours.size());

        for( int i = 0; i < contours.size(); i++ )
        {    mu[i] = moments( contours[i], false );
            mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
        }

        for( int i = 0; i < contours.size(); i++ )
        {
            int k=i;
            int c=0;

            while(hierarchy[k][2] != -1)
            {
                k = hierarchy[k][2] ;
                c = c+1;
            }
            if(hierarchy[k][2] != -1)
                c = c+1;

            if (c >= 5)
            {
                marks[mark] = i;        // i.e., A and B are already found, assign current contour to C
                mark = mark + 1 ;
            }
        }
        for(int i = 0; i <mark; i++){
            RotatedRect rect = minAreaRect(contours[marks[i]]);

            Mat qr_roi = transformCorner(image, rect);
            cvtColor(qr_roi, qr_roi, CV_BGR2GRAY);
            threshold(qr_roi, qr_roi, 100, 255, CV_THRESH_BINARY);
            imshow("roi"+to_string(i), qr_roi);
            waitKey(1);
            //imwrite("roi"+to_string(i), qr_roi);
            drawContours( image, contours, marks[i] , Scalar(255,200,0), 2, 8, hierarchy, 0 );
        }
        imshow ( "Image", image );
        //imshow ( "QR code", qr_thres );
        waitKey(1);
        key = waitKey(500000);    // OPENCV: wait for 1ms before accessing next frame

    }    // End of 'while' loop

    return 0;
}

Mat transformCorner(Mat &image, RotatedRect &rect){
    Point2f points[4];
    Mat imag = image;
    rect.size.height = rect.size.height*2;
    rect.size.width = rect.size.width*2;
    rect.points(points);
    /*
     for (int i = 0; i < 4; i++)//画矩形
     line(imag, points[i], points[(i + 1) % 4], Scalar(255, 0, 0));
     */
    Point2f center = rect.center;//外接矩形中心点坐标
    Mat rot_mat = getRotationMatrix2D(center, rect.angle, 1.0);//求旋转矩阵
    Mat rot_image;
    Size dst_sz(image.size());
    warpAffine(image, rot_image, rot_mat, dst_sz);//原图像旋转
    int a,b;
    a = (center.x - (rect.size.width / 2) >= 0)? center.x - (rect.size.width / 2) : 0;
    b = (center.y - (rect.size.height/2) >= 0)? center.y - (rect.size.height/2):0;
    Mat result1 = rot_image(Rect(a, b, rect.size.width, rect.size.height));//提取ROI
    return result1;
}
