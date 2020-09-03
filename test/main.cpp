#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

#define MAT_HELPER_USE_OPENCV
#include "../mat_storage/mat_helper.h"

int main(int argc, char **argv)
{
    if (1)
    {
        int dims = 3;
        int dim_size[3] = {240, 320, 4};
        
        static unsigned char buf[320 * 240 * 4];
        int mat_size = mat_helper_getsize(dims, dim_size, MAT_HELPER_UINT8);
        for (int i = 0; i < mat_size; ++i)
        {
            //buf[i] = rand();
            buf[i] = i;
            printf("%u\n", buf[i]);
        }
        
        int r = mat_helper_write_mat("127.0.0.1", "test", dims, dim_size, MAT_HELPER_UINT8, (char *)buf);
        printf("r:%d\n", r);
    }
    if (0)
    {
        cv::Mat img = cv::imread("1.png");
        cv::resize(img, img, cv::Size(1280, 720), 1, 1);
        mat_helper_save("127.0.0.1", "test", img);
        cv::imshow("save", img);
        cv::waitKey(10);
    }
    
    {
        cv::Mat img;
        mat_helper_load("127.0.0.1", "test", img);
        
        cv::imshow("load", img);
        cv::waitKey();
    }
    
    if (0)
    {
        int dims = 0;
        int dim_size[8] = {0};
        int type = 0;
        static unsigned char buf[320 * 240];
        int r = mat_helper_read_mat("127.0.0.1", "test", &dims, dim_size, &type, (char *)buf, sizeof(buf));
        printf("r:%d dims:%d type:%d\n", r, dims, type);
        for (int i = 0; i < dims; ++i)
        {
            printf("dim[%d]:%d\n", i, dim_size[i]);
        }
        int mat_size = mat_helper_getsize(dims, dim_size, type);
        for (int i = 0; i < mat_size; ++i)
        {
            printf("%u\n", buf[i]);
        }
    }
	return 0;
}
