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
    
    if (nrhs >= 1 && mxIsChar(prhs[0]))
    {
        mxGetString(prhs[0], ip, sizeof(ip));
    }
    else
    {
        strcpy(ip, "127.0.0.1");
    }
    
    
    
    char *buf = (char *)malloc(32 * 1024);
    if (buf == NULL)
    {
        plhs[0] = mxCreateString("");
        return;
    }
    
    buf[0] = 0;
    if (mat_helper_read_list(ip, buf, 32 * 1024) == -1)
    {
        free(buf);
        plhs[0] = mxCreateString("");
        return;
    }
    
    plhs[0] = mxCreateString(buf);
    free(buf);
    return;
}