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
    
    
    int r = mat_helper_sem_wait(ip, name);
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