//
//  main.cpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/8.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#include "QR_detect.hpp"
using namespace cv;
using namespace std;



int main(int argc, char** argv) {
    Mat src = imread("QR3.png");

    QR_detecter test;
    test.detect(src);
    test.showCorners(0.1);

    
    return 0;
}
