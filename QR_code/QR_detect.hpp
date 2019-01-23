//
//  QR_detect.hpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/9.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#ifndef QR_detect_hpp
#define QR_detect_hpp

#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <zbar.h>
using namespace std;
using namespace cv;
using namespace zbar;

class QR_detecter{
private:
    int data;
    Mat result;

    /*
     * 腐蚀处理提取第四章图片
     */
    Mat check_fourth(Mat& img,int);
    /* 二次处理 */
    Mat smaller_rect(Mat image);
    /* 判断image里的四张图片能否识别 */
    bool getQR(Mat image[]);
    /* 逆时针旋转图片，angle是旋转度数（360一周） */
    void Rotate(Mat &image,float angle);
    bool FindCornersForContours(vector<Point> bigestContours, Point2f* point);
public:
    QR_detecter(){}
    /* 输入4张图片，并提取其中的二维码 */
    void detect(Mat A, Mat B, Mat C, Mat D);
    /* 显示最终获得的二维码 */
    void show_QR();
    /* 截取图片中旋转矩形部分，返回Mat
     * larger：以rect中心扩大rect截取范围的倍数
     */
    Mat transformCorner(Mat &image, RotatedRect &rect, float larger = 1);
    /* 显示二维码获得的数字 */
    int get_number();
};
#endif /* QR_detect_hpp */
