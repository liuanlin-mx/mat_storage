#include <stdio.h>
#include "../mat_storage/mat_helper.h"
#include "mex.h"


static int _matlab_type_to_mat_helper(int type)
{
    switch (type)
    {
        case mxINT8_CLASS:
            return MAT_HELPER_INT8;
        case mxUINT8_CLASS:
            return MAT_HELPER_UINT8;
            
        case mxUINT16_CLASS:
            return MAT_HELPER_UINT16;
        case mxINT16_CLASS:
            return MAT_HELPER_INT16;
            
        case mxINT32_CLASS:
            return MAT_HELPER_INT32;
        case mxUINT32_CLASS:
            return MAT_HELPER_UINT32;
            
        case mxINT64_CLASS:
            return MAT_HELPER_INT64;
        case mxUINT64_CLASS:
            return MAT_HELPER_UINT64;
            
        case mxSINGLE_CLASS:
            return MAT_HELPER_FLOAT32;
        case mxDOUBLE_CLASS:
            return MAT_HELPER_FLOAT64;
        default:
            return MAT_HELPER_UINT8;
    }
    return MAT_HELPER_UINT8;
}

#define _GET_MAT_IDX(row, col, n, rows, cols, ns) ((row) * (cols) * (ns) + (col) *(ns) + (n))
#define _GET_MX_IDX(row, col, n, rows, cols, ns) (((col) * (rows) + (row)) + (rows) * (cols) * (n))
static void mx_copyto_mat_helper(int dims, int *dim_size, int type, char *src, char *dst)
{
    int ndim[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < dims; ++i)
    {
        ndim[i] = dim_size[i];
    }
    
    if (dims < 2)
    {
        ndim[1] = 1;
    }
    
    if (dims < 3)
    {
        ndim[2] = 1;
    }
    
    switch (type)
    {
        case MAT_HELPER_INT8:
        case MAT_HELPER_UINT8:
        {
            for (int k = 0; k < ndim[2]; ++k)
            {
                for (int i = 0; i < ndim[0]; ++i)
                {
                    for (int j = 0; j < ndim[1]; ++j)
                    {
                        ((unsigned char *)dst)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned char *)src)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
                    }
                }
            }
            break;
        }
            
        case MAT_HELPER_UINT16:
        case MAT_HELPER_INT16:
        {
            for (int k = 0; k < ndim[2]; ++k)
            {
                for (int i = 0; i < ndim[0]; ++i)
                {
                    for (int j = 0; j < ndim[1]; ++j)
                    {
                        ((unsigned short *)dst)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned short *)src)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
                    }
                }
            }
            break;
        }
            
        case MAT_HELPER_INT32:
        case MAT_HELPER_UINT32:
        case MAT_HELPER_FLOAT32:
        {
            
            for (int k = 0; k < ndim[2]; ++k)
            {
                for (int i = 0; i < ndim[0]; ++i)
                {
                    for (int j = 0; j < ndim[1]; ++j)
                    {
                        ((unsigned int *)dst)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned int *)src)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
                    }
                }
            }
            break;
        }
            
        case MAT_HELPER_INT64:
        case MAT_HELPER_UINT64:
        case MAT_HELPER_FLOAT64:
        {
            for (int k = 0; k < ndim[2]; ++k)
            {
                for (int i = 0; i < ndim[0]; ++i)
                {
                    for (int j = 0; j < ndim[1]; ++j)
                    {
                        ((unsigned long long *)dst)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned long long *)src)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
                    }
                }
            }
            break;
        }
    }
}

/* nlhs 输出参数个数 plhs输出参数列表 */
/* nrhs 输入参数个数 prhs输入参数列表 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    char ip[64] = {0};
    char name[64] = {0};
    const mxArray *in = NULL;
    if (nrhs > 3)
    {
        plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
        ((char *)mxGetData(plhs[0]))[0] = 0;
        return;
    }
    
    if (nrhs == 2 && mxIsChar(prhs[0]))
    {
        strcpy(ip, "127.0.0.1");
        mxGetString(prhs[0], name, sizeof(name));
        in = prhs[1];
    }
    else if (nrhs == 3 && mxIsChar(prhs[0]) && mxIsChar(prhs[1]))
    {
        mxGetString(prhs[0], ip, sizeof(ip));
        mxGetString(prhs[1], name, sizeof(name));
        in = prhs[2];
    }
    else
    {
        plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
        ((char *)mxGetData(plhs[0]))[0] = 0;
        return;
    }
    
    
    switch (mxGetClassID(in))
    {
        case mxDOUBLE_CLASS:
        case mxSINGLE_CLASS:
        case mxINT8_CLASS:
        case mxUINT8_CLASS:
        case mxINT16_CLASS:
        case mxUINT16_CLASS:
        case mxINT32_CLASS:
        case mxUINT32_CLASS:
        case mxINT64_CLASS:
        case mxUINT64_CLASS:
            break;
        default:
            plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
            ((char *)mxGetData(plhs[0]))[0] = 0;
            return;
    }
    
    int dims = 0;
    int dim_size[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int type = _matlab_type_to_mat_helper(mxGetClassID(in));
    
    dims = mxGetNumberOfDimensions(in);
    for (int i = 0; i < dims; ++i)
    {
        dim_size[i] = mxGetDimensions(in)[i];
    }

    int mat_size = mat_helper_getsize(dims, dim_size, type);
    char *tmp = (char *)malloc(mat_size);
    if (tmp == NULL)
    {
        plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
        ((char *)mxGetData(plhs[0]))[0] = 0;
        return;
    }
    mx_copyto_mat_helper(dims, dim_size, type, (char *)mxGetData(in), tmp);
    if (-1 == mat_helper_write_mat(ip, name, dims, dim_size, type, tmp))
    {
        free(tmp);
        plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
        ((char *)mxGetData(plhs[0]))[0] = 0;
        return;
    }
    free(tmp);
    plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
    ((char *)mxGetData(plhs[0]))[0] = 1;
    return;
}