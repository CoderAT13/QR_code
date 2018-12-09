# QR_code
## Author：CoderAt
## 说明：
1. 删除了部分识别特殊二维码功能，专注比赛标准
2. 很少注释请见谅，未分目录，项目cpp、hpp在QR_code目录里
3. 目前识别判断方式compare_sample(Mat &roi)：判断与sample的符合率（这里有个bug，待修）, 计算区域黑白面积比例过滤（0.55～0.66，虽然理论是0.33（16/49））。
