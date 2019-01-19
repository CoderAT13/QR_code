//
//  QR_detect.hpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/9.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#include "QR_detect.hpp"

int main(){
    Mat a = imread("a.jpg");
    Mat b = imread("b.jpg");
    Mat c = imread("c.jpg");
    Mat d = imread("d.jpg");
    QR_detecter test;
    test.detect(a, d, b, c);
    test.show_QR();
    cout << test.get_number() << endl;
    return 0;
}
