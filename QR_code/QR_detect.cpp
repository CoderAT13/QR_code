//
//  QR_detect.cpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/9.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#include "QR_detect.hpp"
int global_count = 0;
Mat QR_detecter::transformCorner(Mat &image, RotatedRect &rect, float larger ){
    Point2f points[4];
    Mat imag = image;
    rect.size.height = rect.size.height*larger;
    rect.size.width = rect.size.width*larger;
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
    int a,b,c,d;
    a = (center.x - (rect.size.width / 2) >= 0)? center.x - (rect.size.width / 2) : 0;
    b = (center.y - (rect.size.height/2) >= 0)? center.y - (rect.size.height/2):0;
    c = (a + rect.size.width < rot_image.cols)? c = rect.size.width : rot_image.cols-a;
    d = (b + rect.size.height < rot_image.rows)? d = rect.size.height : rot_image.rows-b;
    Mat result1 = rot_image(Rect(a, b, c, d));//提取ROI
    //cv::ellipse(image, rect, cv::Scalar(0, 255, 255), 2, 8);
    cv::Point2f* vertices = new cv::Point2f[4];
    rect.points(vertices);

    std::vector<cv::Point> contour;

    for (int i = 0; i < 4; i++)
    {
        contour.push_back(vertices[i]);
    }

    std::vector<std::vector<cv::Point>> contours;
    contours.push_back(contour);
    //cv::drawContours(image, contours, 0, cv::Scalar(0, 0, 255), 1);

    return result1;
}

Mat QR_detecter::check_fourth(Mat& img){

    Mat out;
    //获取自定义核
    Mat element = getStructuringElement(MORPH_RECT, Size(25, 25)); //第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的
    //腐蚀操作
    erode(img, out, element);

    return out;
}

void QR_detecter::detect(Mat A, Mat B, Mat C, Mat D){
    bool has_check;
    Mat src[4];
    int tmp_count = 0;
    Mat QR_piece[10];
    src[0] = A;
    src[1] = B;
    src[2] = C;
    src[3] = D;

    /* 提取4张图片中的唯一 QR-Code 部分 */

    for (int x = 0; x < 4; x++){
        has_check = false;
        Mat image = src[x];
        Mat qr,qr_raw,qr_gray,qr_thres;

        resize(image, image, Size(image.cols/2,image.rows/2));

        /* 检测三个定位点的方法 */

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        Mat gray(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
        Mat edges(image.size(), CV_MAKETYPE(image.depth(), 1));
        cvtColor(image,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
        Canny(gray, edges, 100 , 200, 3);        // Apply Canny edge detection on the gray image
        findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        int mark;
        int marks[100];
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
        if (mark > 0){
            has_check = true;
            RotatedRect rect = minAreaRect(contours[marks[0]]);
            Mat qr_roi = transformCorner(image, rect,2);
            cvtColor(qr_roi, qr_roi, CV_BGR2GRAY);
            threshold(qr_roi, qr_roi, 100, 255, CV_THRESH_BINARY);
            if (tmp_count < 4 ){
                QR_piece[tmp_count++] = smaller_rect(transformCorner(image, rect, (float)1));
            }
            //drawContours( image, contours, marks[i] , Scalar(255,200,0), 2, 8, hierarchy, 0 );
        }


        /* 腐蚀处理 */
        if (!has_check){
            Mat tmp = check_fourth(image);

            if(image.empty()){ cerr << "ERR: Unable to find image.\n" << endl;
                return;
            }
            // Creation of Intermediate 'Image' Objects required later
            Mat gray(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
            Mat edges(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
            //Mat traces(image.size(), CV_8UC3);                                // For Debug Visuals

            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;



            cvtColor(tmp,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
            Canny(gray, edges, 100 , 200, 3);        // Apply Canny edge detection on the gray image

            findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE); // Find contours with hierarchy
            /*
             for (int i = 0; i < contours.size(); i ++){
             drawContours( image, contours, i , Scalar(255,200,0), 2, 8, hierarchy, 0 );
             }
             */

            /* 通过腐蚀获得轮廓，用旋转矩形画出来，并生成图片 */
            for (int i = 0; i < contours.size(); i++){
                if (contourArea(contours[i])<900) continue;
                RotatedRect rect = minAreaRect(contours[i]);
                //Mat qr_roi = transformCorner(image, rect, (float)23/27);
                if (tmp_count < 4 ){
                    QR_piece[tmp_count++] = smaller_rect(transformCorner(image, rect, (float)1));
                }
                break;
            }
        }

    }

    /* 处理截取下来的1/4 QR-Code */
    for(int i = 0; i < 4; i++){
        cvtColor(QR_piece[i], QR_piece[i], CV_RGB2GRAY);
        threshold(QR_piece[i], QR_piece[i], 100, 255, THRESH_BINARY);
        resize(QR_piece[i],QR_piece[i],Size(200,200),0,0,INTER_LINEAR);

    }


    /*
     * 排列、组合、旋转直到检测出二维码的值
     */
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 4; j++)
            if (getQR(QR_piece)) return;
            else{
                for(int k = 0; k < 4; k++)
                    if (getQR(QR_piece)) return;
                    else{
                        for(int l = 0; l < 4; l++)
                            if (getQR(QR_piece)) return;
                            else{
                                for (int m = 0; m < 4; m++)
                                    if (getQR(QR_piece)) return;
                                    else Rotate(QR_piece[3], 90);
                                Rotate(QR_piece[2], 90);
                            }
                        Rotate(QR_piece[1], 90);
                    }
                Rotate(QR_piece[0], 90);
            }

        if (i == 0){
            Mat hold = QR_piece[3];
            QR_piece[3] = QR_piece[2];
            QR_piece[2] = hold;
        }
        if (i == 1){
            Mat hold = QR_piece[2];
            QR_piece[2] = QR_piece[1];
            QR_piece[1] = hold;
        }
        if (i == 2){
            Mat hold = QR_piece[1];
            QR_piece[1] = QR_piece[0];
            QR_piece[0] = hold;
        }
    }
    

}

void QR_detecter::show_QR(){
    imshow("QR_code",result);
    waitKey(1);
}

int QR_detecter::get_number(){
    if (data > 999 && data < 10000)
        return data;
    else
        return 0;
}

Mat QR_detecter::smaller_rect(Mat image){
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat gray(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
    Mat edges(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
    cvtColor(image,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
    Canny(gray, edges, 100 , 200, 3);        // Apply Canny edge detection on the gray image
    findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    vector<Point> total;
    for (int i = 0; i < contours.size(); i++){
        for (int j = 0; j < contours[i].size(); j++){
            total.push_back(contours[i][j]);
        }
    }
    RotatedRect rect = minAreaRect(total);
    return transformCorner(image, rect, 1);
}

void QR_detecter::Rotate(Mat &image,float angle){
    Point2f points[4];
    /*
     for (int i = 0; i < 4; i++)//画矩形
     line(imag, points[i], points[(i + 1) % 4], Scalar(255, 0, 0));
     */
    Point2f center = Point(image.cols/2, image.rows/2);//外接矩形中心点坐标
    Mat rot_mat = getRotationMatrix2D(center, angle, 1.0);//求旋转矩阵
    Size dst_sz(image.size());
    warpAffine(image, image, rot_mat, dst_sz);//原图像旋转
}

bool QR_detecter::getQR(Mat QR_piece[]){
    //bool result = false;
    Mat combine,combine1,combine2;
    //combine picture
    hconcat(QR_piece[0],QR_piece[1],combine1);//行排列   a+b=combine1
    hconcat(QR_piece[2],QR_piece[3],combine2);
    vconcat(combine1,combine2,combine);//列排列  combine1+combine2=combine
    //adaptiveThreshold( combine, combine, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 21,0);
    //threshold(combine, combine, 30, 200.0, CV_THRESH_OTSU + CV_THRESH_BINARY_INV);
    //============ratation test===================//
    /*Point2f src_center(combine.rows / 2.0,combine.cols / 2.0);
     Mat warp_mat = getRotationMatrix2D(src_center,45,0.75);//得到旋转矩阵，逆时针旋转45度，尺度缩放为1，即保持原尺寸
     warpAffine(combine,combine,warp_mat,combine.size());*/

    //===============识别二维码=====================//
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    int width1 = combine.cols;
    int height1 = combine.rows;  //get cols and rows
    uchar *raw = (uchar *)combine.data;
    Image imageZbar(width1, height1, "Y800", raw, width1 * height1);
    scanner.scan(imageZbar); //扫描条码
    Image::SymbolIterator symbol = imageZbar.symbol_begin();
    if(imageZbar.symbol_begin()==imageZbar.symbol_end())
    {
        //cout<< global_count++ <<endl;
        return false;
    }
    if(symbol != imageZbar.symbol_end())
    {
        cout<<"类型："<<endl<<symbol->get_type_name()<<endl<<endl;
        cout<<"条码："<<endl<<symbol->get_data()<<endl<<endl;
        result = combine;
        imageZbar.set_data(NULL,0);
        data = stoi(symbol->get_data());
        return true;
    }
    return true;
}


