
#include "QR_detect.hpp"

int main(){
    Mat a = imread("rb.jpg");
    Mat b = imread("rc.jpg");
    Mat c = imread("rd.jpg");
    Mat d = imread("ra.jpg");

    QR_detecter t;
    t.detect(a, b, c, d);
    t.show_QR();
    t.get_number();
    return 0;
}

