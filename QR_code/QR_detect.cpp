//
//  QR_detect.cpp
//  QR_code
//
//  Created by 张涵健 on 2018/12/9.
//  Copyright © 2018 张涵健Coder.@. All rights reserved.
//

#include "QR_detect.hpp"
int global_count = 0;

bool cmp_dis1(cv::Point2f a,cv::Point2f b){
    return (a.x+a.y)<(b.x+b.y);
}

bool cmp_dis3(cv::Point2f a,cv::Point2f b){
    return (a.x-a.y) < (b.x - b.y);
}

bool cmp_dis2(cv::Point2f a,cv::Point2f b){
    if (a.y < b.y) return true;
    else if (a.y == b.y){
        if (a.x < b.x) return true;
        else return false;
    }
    else return false;
}

Point2f getCrossPoint(Vec4i LineA, Vec4i LineB)
{
    double ka, kb;
    ka = (double)(LineA[3] - LineA[1]) / (double)(LineA[2] - LineA[0]); //求出LineA斜率
    kb = (double)(LineB[3] - LineB[1]) / (double)(LineB[2] - LineB[0]); //求出LineB斜率
    /*
    if ((abs(ka) > 0.5 && abs(ka) < 1.5) || (abs(kb) > 0.5 && abs(kb) <1.5))
        return Point2f(-66666,-66666);
     */
    Point2f crossPoint;
    crossPoint.x = (ka*LineA[0] - LineA[1] - kb*LineB[0] + LineB[1]) / (ka - kb);
    crossPoint.y = (ka*kb*(LineA[0] - LineB[0]) + ka*LineB[1] - kb*LineA[1]) / (ka - kb);
    return crossPoint;
}

float getDistance(Point2f a, Point2f b){
    return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}

int th = 0;


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

Mat QR_detecter::check_fourth(Mat& img, int size){

    Mat out;
    //获取自定义核
    Mat element = getStructuringElement(MORPH_RECT, Size(size, size)); //第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的
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
        th = x;
        has_check = false;
        resize(src[x], src[x], Size(src[x].cols/2,src[x].rows/2));
        Mat image = src[x];
        Mat res;
        resize(image, res, Size(src[x].cols/3,src[x].rows/3));
        imshow("image"+to_string(x), res);
        waitKey(1);
        GaussianBlur(src[x], image, Size(3,3), 0);
        //GaussianBlur(image, image, Size(3,3), 0);
        //imshow("image", image);
        //waitKey(1);
        Mat qr,qr_raw,qr_gray,qr_thres;



        /* 检测三个定位点的方法 */

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        Mat gray(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
        Mat edges(image.size(), CV_MAKETYPE(image.depth(), 1));
        cvtColor(image,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
        Canny(gray, edges, 100 , 200, 3);        // Apply Canny edge detection on the gray image
        findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
        /*
        for (int i = 0; i < contours.size(); i++){
            drawContours(image, contours, i, Scalar(255,255,0));
        }
         */
        //imshow("image", image);
        //waitKey(1);
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
            //drawContours(image, contours, marks[0], Scalar(255,255,0));
            //imshow("src"+to_string(th), image);
            //waitKey(1);
            RotatedRect rect = minAreaRect(contours[marks[0]]);
            Mat qr_roi = transformCorner(image, rect,2.1);
            Mat tmp1[4];
            tmp1[0] = qr_roi(Rect(0, 0, 3*qr_roi.cols/4, 3*qr_roi.rows/4));
            tmp1[1] = qr_roi(Rect(qr_roi.cols/4, 0, 3*qr_roi.cols/4, 3*qr_roi.rows/4));
            tmp1[2] = qr_roi(Rect(0, qr_roi.rows/4, 3*qr_roi.cols/4, 3*qr_roi.rows/4));
            tmp1[3] = qr_roi(Rect(qr_roi.cols/4, qr_roi.rows/4, 3*qr_roi.cols/4, 3*qr_roi.rows/4));
            vector<vector<Point>> tmp_contours[4];
            vector<Vec4i> hierarchy[4];
            float area[4];
            for(int i = 0; i < 4; i++){
                //imshow("tmp"+to_string(i), tmp1[i]);
                //waitKey(1);
                cvtColor(tmp1[i], gray, CV_RGB2GRAY);
                Canny(gray, edges, 100, 200, 3);
                area[i] = 0;
                findContours(edges, tmp_contours[i], hierarchy[i], RETR_TREE, CHAIN_APPROX_SIMPLE);
                for (int k = 0; k < tmp_contours[i].size(); k++){
                    area[i] += contourArea(tmp_contours[i][k]);
                }
            }
            int max = 0;
            Mat result;
            for (int i = 0 ; i < 4; i++){
                if (area[i]>max){
                    max = area[i];
                    result = tmp1[i];
                }
            }
            //imshow("result", result);
            //waitKey(1);
            Mat warp_dst = image.clone();
            if (tmp_count < 4){
                QR_piece[tmp_count++] = smaller_rect(result);
            }
            //drawContours( image, contours, marks[i] , Scalar(255,200,0), 2, 8, hierarchy, 0 );
        }


        /* 腐蚀处理 */
        if (!has_check){
            Mat tmp = check_fourth(image,25);

            //imshow("tmp", tmp);
            //waitKey(1);
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
            threshold(gray, gray, 100, 255, THRESH_BINARY);
            Canny(gray, edges, 100 , 200, 3);        // Apply Canny edge detection on the gray image
            findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE); // Find contours with hierarchy

            /*
             for (int i = 0; i < contours.size(); i ++){
                 drawContours( image, contours, i , Scalar(255,200,0), 2, 8, hierarchy, 0 );
             }
             */
            /* 通过腐蚀获得轮廓，用旋转矩形画出来，并生成图片 */
            for (int i = 0; i < contours.size(); i++){
                //cout<<contourArea(contours[i]) << endl;
                //if (contourArea(contours[i])<3000) continue;

                //imshow("image", image);
                //waitKey(1);
                RotatedRect rect = minAreaRect(contours[i]);
                if (abs((float)rect.size.width/rect.size.height-1) > 0.1 ||
                    rect.size.width*rect.size.height < 10000)
                    continue;
                //drawContours( image, contours, i , Scalar(0,0,255), 2, 8, hierarchy, 0 );
                resize(image, image, Size(image.cols/2,image.rows/2));

                //Mat qr_roi = transformCorner(image, rect, (float)23/27);
                if (tmp_count < 4 ){
                    QR_piece[tmp_count++] = smaller_rect(transformCorner(src[x], rect, (float)1));
                }
                break;
            }
        }

    }

    if (tmp_count < 4){
        cout << "doesn't Detect 4" << endl;
        return;
    }
    /* 处理截取下来的1/4 QR-Code */
    for(int i = 0; i < 4; i++){
        cvtColor(QR_piece[i], QR_piece[i], CV_RGB2GRAY);
        threshold(QR_piece[i], QR_piece[i], 150, 255, THRESH_BINARY);
        resize(QR_piece[i],QR_piece[i],Size(200,200),0,0,INTER_LINEAR);
    }
    for(int i = 0; i < 4; i++){
        imshow("qr_p"+to_string(i), QR_piece[i]);
        waitKey(1);
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
    if (result.empty()) {
        cout << "No result" << endl;
        return;
    }
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
    //imshow("src"+to_string(th), image);
    //waitKey(1);

    Mat tmp = image.clone();


    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    cv::Point2f srcTri[4];//原图坐标
    cv::Point2f dstTri[4];//映射之后的坐标
    cv::Point2f point[1000];
    Mat gray(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
    Mat edges(image.size(), CV_MAKETYPE(image.depth(), 1));            // To hold Grayscale Image
    //Mat blk = check_fourth(image, 10);
    cvtColor(image,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
    //threshold(gray, gray, 90, 255, THRESH_BINARY);
    //imshow("src"+to_string(th), gray);
    //waitKey(1);

    Canny(gray, edges, 150 , 200, 3);        // Apply Canny edge detection on the gray image
    findContours( edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    cvtColor(tmp,gray,CV_RGB2GRAY);        // Convert Image captured from Image Input to GrayScale
    /*
    for (int i = 0; i < contours.size(); i++){
        drawContours(image, contours, i, Scalar(255,0,255));
    }
     */
    vector<Point> total;
    for (int i = 0; i < contours.size(); i++){
        if (contourArea(contours[i])>15)
        for (int j = 0; j < contours[i].size(); j++){
            total.push_back(contours[i][j]);
        }
    }
    vector<vector<Point>> hull(1);
    convexHull(total, hull[0]);
    vector<Vec4i> lines;
    for (int i = 0; i < hull.size(); i++){
        drawContours(image, hull, i, Scalar(255,255,0));
    }
    for (int i = 0; i < hull[0].size(); i++){
        if (i!=hull[0].size()-1)
            lines.push_back(Vec4i(hull[0][i].x,hull[0][i].y,hull[0][i+1].x,hull[0][i+1].y));
        else
            lines.push_back(Vec4i(hull[0][i].x,hull[0][i].y,hull[0][0].x,hull[0][0].y));
    }
    vector<Point2f> corners;
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        for (unsigned int j = i + 1; j< lines.size(); j++)
        {
            cv::Point2f pt = getCrossPoint(lines[i], lines[j]);
            if (pt.x >= 0 && pt.y >= 0)
            {
                corners.push_back(pt);
            }
        }
    }
    for (int i = 0; i < corners.size(); i++){
        float min = 65535;
        /*
        corners[i].x = (corners[i].x >= image.cols && corners[i].x - image.cols < 5)? image.cols-1:corners[i].x;
        corners[i].x = (corners[i].x < 0 && corners[i].x > -5)? 0: corners[i].x;
        corners[i].y = (corners[i].y >= image.rows && corners[i].y - image.rows < 5)? image.rows-1:corners[i].y;
        corners[i].y = (corners[i].y < 0 && corners[i].y > -5)? 0: corners[i].y;
        */

        //circle(image, corners[i], 5, Scalar(100,0,0));
        for (int j= 0; j < hull[0].size(); j++)
            if (getDistance(corners[i],hull[0][j]) < min){
                min = getDistance(corners[i],hull[0][j]);
            }
        if ( min < 30){
            corners[i].x = (corners[i].x >= image.cols )? image.cols-1:corners[i].x;
            corners[i].x = (corners[i].x < 0 )? 0: corners[i].x;
            corners[i].y = (corners[i].y >= image.rows )? image.rows-1:corners[i].y;
            corners[i].y = (corners[i].y < 0 )? 0: corners[i].y;
            hull[0].push_back(corners[i]);
            circle(image, corners[i], 5, Scalar(100,0,0));
        }
    }

    /*
    for (int i = 0 ; i < total.size(); i++){
        bool is_hull = false;
        Point2f p[2];
        int n = 0;
        for (int j = 0; j < hull[0].size(); j++){
            if (total[i] == hull[0][j]) {
                is_hull = true;
            }
        }
        if (!is_hull){
            for (int j = 0; j < hull[0].size(); j++){
                if (getDistance(total[i],hull[0][j]) < image.rows/25) {
                    p[n++] = hull[0][j];
                }
            }
            if (n == 2){
                Point2f a((p[0].x+p[1].x)/2,(p[0].y+p[1].y)/2);
                Point2f b(total[i].x+(total[i].x-a.x)*3,total[i].y+(total[i].y-a.y)*3);
                if (b.x < 0) b.x = 0;
                if (b.x >= image.cols) b.x = image.cols-1;
                if (b.y < 0) b.y = 0;
                if (b.y >= image.rows) b.y = image.rows-1;
                circle(image, b,10, Scalar(255,0,255));
            }
        }
    }
    */
    //imshow("images",image);
    //waitKey(1);
    //RotatedRect rect = minAreaRect(total);
    std::sort(total.begin(), total.end(), cmp_dis1);
    //std::sort(hull[0].begin(), hull[0].end(), cmp_dis2);

    Point2f center[4];
    float radius[4];
    int count = 0;
    vector<Point> qur[4];
    for (int i = 0; i < hull[0].size(); i++){
        if (hull[0][i].y <= image.rows/2 && hull[0][i].x <= image.cols/2)
            qur[0].push_back(hull[0][i]);

        if (hull[0][i].y < image.rows/2 && hull[0][i].x > image.cols/2)
            qur[1].push_back(hull[0][i]);

        if (hull[0][i].y > image.rows/2 && hull[0][i].x < image.cols/2)
            qur[2].push_back(hull[0][i]);

        if (hull[0][i].y >= image.rows/2 && hull[0][i].x >= image.cols/2)
            qur[3].push_back(hull[0][i]);
    }
    for (int i = 0 ; i < 4 ; i ++){
        minEnclosingCircle(qur[i], center[i], radius[i]);
        for (int j = 0; j < qur[i].size(); j++){
            if (abs(getDistance(center[i], qur[i][j])-radius[i]) < 10){
                //circle(image,hull[0][i],10,Scalar(0,0,255));
                point[count++] = qur[i][j];
            }
        }
    }
    //minEnclosingCircle(hull[0], center, radius);

    sort(point, point+count, cmp_dis1);
    srcTri[0]=point[0];
    srcTri[2]=point[count-1];

    sort(point, point+count, cmp_dis3);
    srcTri[1]=point[0];
    srcTri[3]=point[count-1];

    if (th == 0)
        srcTri[3].x += 5;
    /*
    RotatedRect rect = minAreaRect(hull[0]);
    Point2f rect_corner[4];
    rect.points(rect_corner);
    sort(point, point+4, cmp_dis1);
    for (int k = 0 ; k < 4 ; k++){
        point[k] = rect_corner[abs(k-3)];
        for (int i = 0 ; i < hull[0].size(); i++){
            if (getDistance(point[k], rect_corner[k]) > getDistance(hull[0][i], rect_corner[k])){
                point[k] = hull[0][i];
            }
        }
    }
    for (int i = 0; i < 4; i++){
        circle(image,point[i],10,Scalar(255,0,0));
    }

    sort(point,point+4, cmp_dis1);
     */
    for (int i = 0; i < 4; i++){
        circle(image,srcTri[i],10,Scalar(0,0,255));
        imshow("circle", image);
        waitKey(1);
    }

    /*
    //rect.points(point);
    //std::sort(point,point+4,cmp_dis1);
    //sort(point,point+4,cmp_dis1);
    
    srcTri[0]=point[0];
    srcTri[1]=point[1];
    srcTri[2]=point[3];
    srcTri[3]=point[2];
     */
    dstTri[0]=cv::Point2f(0,0);
    dstTri[1]=cv::Point2f(0,image.rows);
    dstTri[2]=cv::Point2f(image.rows,image.rows);
    dstTri[3]=cv::Point2f(image.rows,0);
    cv::Mat warp_mat(2,3,CV_32FC1);
    warp_mat=cv::getPerspectiveTransform(srcTri,dstTri);// 求得仿射变换
    Mat warp_dst=cv::Mat::zeros( image.rows, image.rows, image.type() );
    warpPerspective(tmp,warp_dst,warp_mat,warp_dst.size());// 对源图像应用上面求得的仿射变换
    //std::sort(hull[0].begin(),hull[0].end(),cmp_dis1);

    /*
    vector<Point> fill;
    fill.push_back(Point(0,0));
    fill.push_back(Point(0,image.rows));
    fill.push_back(Point(image.cols,image.rows));
    fill.push_back(Point(image.cols,0));
    vector<vector<Point>> whole;
    whole.push_back(fill);
    cv::fillPoly(image, whole, cv::Scalar(0, 0, 0));//fillPoly函数的第二个参数是二维数组！！
    cv::polylines(image, hull, true, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);//第2个参数可以采用contour或者contours，均可
    cv::fillPoly(image, hull, cv::Scalar(255, 255, 255));//fillPoly函数的第二个参数是二维数组！！
    //FindCornersForContours(hull[0], srcTri);
     */
    //FindCornersForContours(hull[0], point);
    imshow("src"+to_string(th), image);
    waitKey(1);
    imshow("warp"+to_string(th), warp_dst);
    waitKey(1);
    //return transformCorner(image, rect, 1);
    return warp_dst;
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
    //imshow("combine", combine);
    //waitKey(1);
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

bool QR_detecter::FindCornersForContours(vector<Point> bigestContour, Point2f* res){
    Point2f corner[4];
    int corners_num = 0;
    int icount = (int)bigestContour.size();
    float fmax = -1;//用于保存局部最大值
    int   imax = -1;
    bool  bstart = false;
    for (int i=0;i<(int)bigestContour.size();i++){
        Point2f pa = (Point2f)bigestContour[(i+icount-7)%icount];
        Point2f pb = (Point2f)bigestContour[(i+icount+7)%icount];
        Point2f pc = (Point2f)bigestContour[i];
        //两支撑点距离
        float fa = getDistance(pa,pb);
        float fb = getDistance(pa,pc)+getDistance(pb,pc);
        float fang = fa/fb;
        float fsharp = 1-fang;
        if (fsharp>0.05){
            bstart = true;
            if (fsharp>fmax){
                fmax = fsharp;
                imax = i;
            }
        }else{
            if (bstart){
                //circle(board,bigestContour[imax],10,Scalar(255),1);
                //circle(src,bigestContour[imax],10,Scalar(255,255,255),1);
                if (corners_num <= 4)
                    corner[corners_num++] = bigestContour[imax];
                imax  = -1;
                fmax  = -1;
                bstart = false;
            }
        }
    }
    if(corners_num >= 3){
        sort(corner, corner+4, cmp_dis1);
        for (int i = 0; i < 3; i++){
            res[i] = corner[i];
        }
        return true;
    }
    return false;
}
