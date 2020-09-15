#include <stdio.h>
#include <map>
#include <string>
#include <thread>
#include <functional>
#include "mat_helper.h"

struct node
{
    mat_helper_mat_info info;
    char *data;
};

std::map<std::string, node> mat_map;

void write_req(mat_helper_socket_t c)
{
    mat_helper_write_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_write_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_WRITE_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        mat_helper_close_socket(c);
        return;
    }
    for (int i = 0; i < req.info.dims; ++i)
    {
        req.info.dim_size[i] = ntohl(req.info.dim_size[i]);
    }
    
    int mat_size = mat_helper_getsize(req.info.dims, req.info.dim_size, req.info.type);
    char *data = (char *)malloc(mat_size);
    if (data == NULL)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    node node_;
    node_.info = req.info;
    node_.data = data;
    
    if (mat_helper_read(c, data, mat_size) == -1)
    {
        free(data);
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    
    if (mat_map.count(std::string(req.name)))
    {
        free(mat_map[std::string(req.name)].data);
    }
    mat_map[std::string(req.name)] = node_;
    
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
}



void read_req(mat_helper_socket_t c)
{
    mat_helper_read_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_read_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_READ_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        mat_helper_close_socket(c);
        return;
    }
    
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
    }
    
    node& node_ = mat_map[std::string(req.name)];
    res.info.dims = node_.info.dims;
    for (int i = 0; i < node_.info.dims; ++i)
    {
        res.info.dim_size[i] = htonl(node_.info.dim_size[i]);
    }
    res.info.type = node_.info.type;
    res.result = MAT_HELPER_OK;
    
    if (-1 == mat_helper_write(c, (char *)&res, sizeof(res)))
    {
        return;
    }
    int mat_size = mat_helper_getsize(node_.info.dims, node_.info.dim_size, node_.info.type);
    mat_helper_write(c, (char *)node_.data, mat_size);
}


void read_del_req(mat_helper_socket_t c)
{
    mat_helper_read_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_read_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_READ_DEL_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        mat_helper_close_socket(c);
        return;
    }
    
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
    }
    
    node& node_ = mat_map[std::string(req.name)];
    res.info.dims = node_.info.dims;
    for (int i = 0; i < node_.info.dims; ++i)
    {
        res.info.dim_size[i] = htonl(node_.info.dim_size[i]);
    }
    res.info.type = node_.info.type;
    res.result = MAT_HELPER_OK;
    
    if (-1 == mat_helper_write(c, (char *)&res, sizeof(res)))
    {
        return;
    }
    int mat_size = mat_helper_getsize(node_.info.dims, node_.info.dim_size, node_.info.type);
    mat_helper_write(c, (char *)node_.data, mat_size);
    mat_map.erase(std::string(req.name));
}

void read_info_req(mat_helper_socket_t c)
{
    mat_helper_read_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_read_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_READ_INFO_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        mat_helper_close_socket(c);
        return;
    }
    
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
    }
    
    node& node_ = mat_map[std::string(req.name)];
    res.info.dims = node_.info.dims;
    for (int i = 0; i < node_.info.dims; ++i)
    {
        res.info.dim_size[i] = htonl(node_.info.dim_size[i]);
    }
    res.info.type = node_.info.type;
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
}



static const char *__mat_helper_get_type_str(int type)
{
    switch (type)
    {
        case MAT_HELPER_INT8:
            return "int8";
        case MAT_HELPER_UINT8:
            return "uint8";
        case MAT_HELPER_INT16:
            return "int16";
        case MAT_HELPER_UINT16:
            return "uint16";
        case MAT_HELPER_INT32:
            return "int32";
        case MAT_HELPER_UINT32:
            return "uint32";
        case MAT_HELPER_INT64:
            return "int64";
        case MAT_HELPER_UINT64:
            return "uint64";
            
        case MAT_HELPER_FLOAT32:
            return "float32";
        case MAT_HELPER_FLOAT64:
            return "float64";
        default:
            return "???";
    }
    return "???";
}

void list_req(mat_helper_socket_t c)
{
    std::string str;
    for (std::map<std::string, node>::iterator it = mat_map.begin(); it != mat_map.end(); ++it)
    {
        char buf[128] = {0};
        
        str += it->first + ": ";
        str += buf;
        str += "(";
        
        for (int i = 0; i < it->second.info.dims; ++i)
        {
            sprintf(buf, "%d", it->second.info.dim_size[i]);
            str += buf;
            if (i + 1 < it->second.info.dims)
            {
                str += "x";
            }
        }
        str += ") " + std::string(__mat_helper_get_type_str(it->second.info.type)) + "\n";
    }
    
    mat_helper_list_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_LIST_RES;
    res.len = htonl(str.length());
    
    if (-1 == mat_helper_write(c, (char *)&res, sizeof(res)))
    {
        return;
    }
    if (str.length() > 0)
    {
        mat_helper_write(c, (char *)str.c_str(), str.length());
    }
}




void del_req(mat_helper_socket_t c)
{
    mat_helper_del_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_del_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_DEL_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        mat_helper_close_socket(c);
        return;
    }
    
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
    }
    
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
    mat_map.erase(std::string(req.name));
}



static void session_thread(mat_helper_socket_t c)
{
    char buf[512];
    if (-1 == mat_helper_read(c, buf, 1))
    {
        mat_helper_close_socket(c);
        return;
    }
        
    switch (buf[0])
    {
        case MAT_HELPER_TYPE_WRITE_REQ:
            write_req(c);
            break;
        case MAT_HELPER_TYPE_READ_REQ:
            read_req(c);
        case MAT_HELPER_TYPE_READ_DEL_REQ:
            read_del_req(c);
            break;
        case MAT_HELPER_TYPE_READ_INFO_REQ:
            read_info_req(c);
            break;
        case MAT_HELPER_TYPE_LIST_REQ:
            list_req(c);
            break;
        case MAT_HELPER_TYPE_DEL_REQ:
            del_req(c);
            break;
    }
    mat_helper_close_socket(c);
}

int main(int argc, char **argv)
{
    
#ifdef MAT_STORAGE_OS_WINDOWS
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    mat_helper_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == MAT_HELPER_INVALID_SOCKET)
    {
        return -1;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MAT_HELPER_PORT);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    
    int value = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&value, (mat_helper_socklen_t)sizeof(value));
    if (0 != bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
    {
        mat_helper_close_socket(sock);
        return -1;
    }
    listen(sock, 10);
    
    while (1)
    {
        struct sockaddr_in in_addr;
        mat_helper_socklen_t addr_len = sizeof(in_addr);
        mat_helper_socket_t c = accept(sock, (struct sockaddr *)&in_addr, &addr_len);
        if (c == MAT_HELPER_INVALID_SOCKET)
        {
            break;
        }
        std::thread th(std::bind(session_thread, c));
        th.detach();
    }
    mat_helper_close_socket(sock);
	return 0;
}
