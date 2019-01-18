//
//  QR_detect.hpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/9.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#ifndef QR_detect_hpp
#define QR_detect_hpp

#include <opencv2/opencv.hpp>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace cv;
using namespace std;

class QR_detecter{
private:
    bool compare_sample(Mat &roi);
    bool isXCorner(Mat &image);
    bool isYCorner(Mat &image);
    Mat transformCorner(Mat &image, RotatedRect &rect);
    Mat output;
public:
    QR_detecter(){

    }
    void detect(Mat &src);
    void showCorners(double time);
};
#endif /* QR_detect_hpp */
