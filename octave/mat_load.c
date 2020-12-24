/*****************************************************************************
*                                                                            *
*  Copyright (C) 2020, Liu An Lin.                                           *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/

#include <stdio.h>
#include "../mat_storage/mat_helper.h"
#include "mex.h"


static int _mat_helper_to_matlab_type(int type)
{
    switch (type)
    {
        case MAT_HELPER_INT8:
            return mxINT8_CLASS;
        case MAT_HELPER_UINT8:
            return mxUINT8_CLASS;
            
        case MAT_HELPER_UINT16:
            return mxUINT16_CLASS;
        case MAT_HELPER_INT16:
            return mxINT16_CLASS;
            
        case MAT_HELPER_INT32:
            return mxINT32_CLASS;
        case MAT_HELPER_UINT32:
            return mxUINT32_CLASS;
            
        case MAT_HELPER_INT64:
            return mxINT64_CLASS;
        case MAT_HELPER_UINT64:
            return mxUINT64_CLASS;
            
        case MAT_HELPER_FLOAT32:
            return mxSINGLE_CLASS;
        case MAT_HELPER_FLOAT64:
            return mxDOUBLE_CLASS;
        default:
            return mxDOUBLE_CLASS;
    }
    return mxDOUBLE_CLASS;
}

#define _GET_MAT_IDX(row, col, n, rows, cols, ns) ((row) * (cols) * (ns) + (col) *(ns) + (n))
#define _GET_MX_IDX(row, col, n, rows, cols, ns) (((col) * (rows) + (row)) + (rows) * (cols) * (n))
static void mat_helper_copyto_mx(int dims, int *dim_size, int type, char *src, char *dst)
{
    int ndim[3] = {0, 0, 0};
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
                        ((unsigned char *)dst)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned char *)src)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
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
                        ((unsigned short *)dst)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned short *)src)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
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
                        ((unsigned int *)dst)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned int *)src)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
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
                        ((unsigned long long *)dst)[_GET_MX_IDX(i, j, k, ndim[0], ndim[1], ndim[2])] = ((unsigned long long *)src)[_GET_MAT_IDX(i, j, k, ndim[0], ndim[1], ndim[2])];
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
    if (nrhs > 2)
    {
        plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
        return;
    }
    
    if (nrhs == 1 && mxIsChar(prhs[0]))
    {
        strcpy(ip, "127.0.0.1");
        mxGetString(prhs[0], name, sizeof(name));
    }
    else if (nrhs == 2 && mxIsChar(prhs[0]) && mxIsChar(prhs[1]))
    {
        mxGetString(prhs[0], ip, sizeof(ip));
        mxGetString(prhs[1], name, sizeof(name));
    }
    else
    {
        plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
        return;
    }
    
    
    
    struct mat_helper_mat_info info;
    memset(&info, 0, sizeof(info));
    mat_helper_socket_t sock = mat_helper_read_mat_begin(ip, name, &info, 0);
    if (sock == MAT_HELPER_INVALID_SOCKET)
    {
        plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
        return;
    }
    
    int mat_size = mat_helper_getsize(info.dims, info.dim_size, info.type);
    
    char *tmp = (char *)malloc(mat_size);
    if (tmp == NULL)
    {
        mat_helper_close_socket(sock);
        plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
        return;
    }
    mwSize ndims = info.dims;
    mwSize dims_[3] = {0, 0, 0};
    for (int i = 0; i < info.dims; ++i)
    {
        dims_[i] = info.dim_size[i];
    }
    
    int r = mat_helper_read_mat_end(sock, &info, (char *)tmp, mat_size);
    if (r != 0)
    {
        free(tmp);
        plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
        return;
    }
    plhs[0] = mxCreateNumericArray(ndims, dims_, _mat_helper_to_matlab_type(info.type), mxREAL);
    if (plhs[0] == NULL)
    {
        free(tmp);
        plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
        return;
    }
    mat_helper_copyto_mx(info.dims, info.dim_size, info.type, tmp, (char *)mxGetData(plhs[0]));
    free(tmp);
    return;
}