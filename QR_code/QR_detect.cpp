//
//  QR_detect.cpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/9.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#include "QR_detect.hpp"
int s_count = 0;
double getMSSIM(const Mat& i1, const Mat& i2)
{

    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/
    int d = CV_32F;

    Mat I1, I2;
    i1.convertTo(I1, d);           // cannot calculate on one byte large values
    i2.convertTo(I2, d);

    Mat I2_2 = I2.mul(I2);        // I2^2
    Mat I1_2 = I1.mul(I1);        // I1^2
    Mat I1_I2 = I1.mul(I2);        // I1 * I2

    /*************************** END INITS **********************************/

    Mat mu1, mu2;   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);

    Mat mu1_2 = mu1.mul(mu1);
    Mat mu2_2 = mu2.mul(mu2);
    Mat mu1_mu2 = mu1.mul(mu2);

    Mat sigma1_2, sigma2_2, sigma12;

    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3;

    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

    Mat ssim_map;
    divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;

    Scalar mssim = mean(ssim_map); // mssim = average of ssim map
    return (mssim[0] + mssim[1] + mssim[2])/3;
}

bool QR_detecter::compare_sample(Mat &roi){
    double Rate = 0;
    int white_count = 0;
    int black_count = 0;
    for (int i = 0; i < roi.rows; i++){
        for (int j = 0; j < roi.cols; j++){
            //cout << (int)sample.at<uchar>(i,j) <<","<< (int)roi_dst.at<uchar>(i,j) << endl;
            if((int)roi.at<uchar>(i,j) == 255) white_count++;
            else black_count++;
        }
    }
    if ((double)white_count/black_count > 0.7 || (double)white_count/black_count < 0.55){
        return false;
    }
    //imwrite("sample"+to_string(rand()%10), roi);
    
    //cout << (double)white_count/black_count << " ";
    for (int t = 0; t <= 3; t++){
        Mat sample = imread("sample" + to_string(t) + ".png");
        Mat roi_dst = roi;
        //imwrite("sample" + to_string(s_count) + ".png", roi);
        s_count ++;
        resize(sample, sample, Size(roi.cols,roi.rows));
        //resize(roi, roi_dst, Size(sample.rows,sample.cols));

        threshold(sample, sample, 150, 255, CV_THRESH_BINARY);
        threshold(roi_dst, roi_dst, 150, 255, CV_THRESH_BINARY);

        imshow("sample", sample);
        waitKey(1);
        imshow("roi", roi_dst);
        waitKey(1);
        

        int count = 0;
        for (int i = 0; i < sample.rows; i++){
            for (int j = 0; j < sample.cols; j++){
                //cout << (int)sample.at<uchar>(i,j) <<","<< (int)roi_dst.at<uchar>(i,j) << endl;
                //cout << (sample.at<uchar>(i,j) == roi_dst.at<uchar>(i,j)) << endl;
                //printf("(%d,%d):%d %d\n",i,j,(int)sample.at<uchar>(i,j) ,(int)roi_dst.at<uchar>(i,j));
                if ((int)sample.at<uchar>(i,j) == (int)roi_dst.at<uchar>(i,j)){
                    count ++;
                }
            }
        }
        double rate = (float)count/(sample.rows*sample.cols);
        //double rate = getMSSIM(sample, roi_dst);
        cout << "Rate" << t << ":" << rate << " ";
        if (Rate < rate) Rate = rate;
    }

    cout << Rate << endl;
    if (Rate > 0.6){
        return true;
    }
    return false;
}

bool QR_detecter::isXCorner(Mat &image){
    int count = 0;
    int before = 0;

    for (int i = 3; i < image.cols; i++){
        //cout << abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]) << " ";
        if(abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]) > 140 ||
           i == image.cols-1 ||
           (abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]) >100 &&
            before > 100 && before <=140)){
               count++;
           }
        before = abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]);
    }
    //cout << endl;
    return (count == 5);
}

bool QR_detecter::isYCorner(Mat &image){
    int count = 0;
    int before = 0;

    for (int i = 3; i < image.rows; i++){
        if(abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]) > 140 ||
           i == image.rows-1 ||
           (abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]) > 100 &&
            before > 100 && before <=140)){
               count++;
           }
        before = abs(image.ptr<uchar>(image.rows/2)[i] - image.ptr<uchar>(image.rows/2)[i-1]);
    }
    return (count == 5);
}

Mat QR_detecter::transformCorner(Mat &image, RotatedRect &rect){
    Point2f points[4];
    Mat imag = image;
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
    Mat result1 = rot_image(Rect(center.x - (rect.size.width / 2), center.y - (rect.size.height/2), rect.size.width, rect.size.height));//提取ROI

    return result1;
}

void QR_detecter::detect(Mat &src){
    resize(src, src, Size(src.cols/2,src.rows/2));
    //cout << src.channels();
    Mat clone_src = src;
    cvtColor(src, src, CV_BGR2GRAY);
    Mat result;
    threshold(src, result, 150, 255, CV_THRESH_BINARY);
    vector<vector<Point>> contours;
    vector<Vec4i> hireachy;
    Moments monents;
    findContours(result.clone(), contours, hireachy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());

    Mat result1 = Mat::zeros(src.size(), CV_8UC3);
    for (size_t t = 0; t < contours.size(); t++) {
        double area = contourArea(contours[t]);
        if (area < 170) continue;
        RotatedRect rect = minAreaRect(contours[t]);
        // 根据矩形特征进行几何分析
        float w = rect.size.width;
        float h = rect.size.height;
        float rate = min(w, h) / max(w, h);
        if (rate > 0.85 && w < src.cols/4 && h<src.rows/4) {
            //printf("angle : %.2f\n", rect.angle);
            Mat qr_roi = transformCorner(result, rect);

            imshow("QR", clone_src);
            waitKey(1);
            
            /*
             imshow("QR", qr_roi);
             waitKey(1);
             */
            if (compare_sample(qr_roi)){
                drawContours(clone_src, contours, static_cast<int>(t), Scalar(0,0,230), 2, 4);
                //imwrite(format("D:/gloomyfish/outimage/contour_%d.jpg", static_cast<int>(t)), qr_roi);
                //drawContours(result, contours, static_cast<int>(t), Scalar(255, 0, 0), 2, 8);
            }
        }
    }
    output = clone_src;
}

void QR_detecter::showCorners(double time){
    imshow("output", output);
    waitKey(time*1000);
}

