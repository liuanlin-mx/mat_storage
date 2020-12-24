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
    
    
    int r = mat_helper_sem_destroy(ip, name);
    if (r != 0)
    {
        plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
        ((char *)mxGetData(plhs[0]))[0] = 0;
        return;
    }
    plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
    ((char *)mxGetData(plhs[0]))[0] = 1;
    return;
}