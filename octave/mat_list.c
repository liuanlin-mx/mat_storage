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