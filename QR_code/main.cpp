
#include "QR_detect.hpp"

int main(){
    Mat a = imread("rb.jpg");
    Mat b = imread("ra.jpg");
    Mat c = imread("rc.jpg");
    Mat d = imread("rd.jpg");

    QR_detecter t;
    t.detect(a, b, c, d);
    t.show_QR();
    t.get_number();
    return 0;
}

