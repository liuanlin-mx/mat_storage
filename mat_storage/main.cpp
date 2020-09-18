#include <stdio.h>
#include <map>
#include <string>
#include <thread>
#include <functional>
#include <mutex>
#include <list>
#include "mat_helper.h"

struct node
{
    node()
    {
        data = NULL;
    }
    
    node(char *data_, struct mat_helper_mat_info info_)
    {
        data = data_;
        info = info_;
    }
    
    node(const node &node_)
    {
        int mat_size = mat_helper_getsize(node_.info.dims, node_.info.dim_size, node_.info.type);
        data = (char *)malloc(mat_size);
        if (data)
        {
            memcpy(data, node_.data, mat_size);
            info = node_.info;
        }
    }
    
    node(node &&node_)
    {
        data = node_.data;
        info = node_.info;
        node_.data = NULL;
    }
    
    ~node()
    {
        if (data)
        {
            free(data);
        }
    }
    
    node& operator=(node&& node_)
    {
        if (data)
        {
            free(data);
        }
        data = node_.data;
        info = node_.info;
        node_.data = NULL;
        return *this;
    }
    
    node& operator=(const node& node_)
    {
        if (data)
        {
            free(data);
        }
        
        int mat_size = mat_helper_getsize(node_.info.dims, node_.info.dim_size, node_.info.type);
        data = (char *)malloc(mat_size);
        if (data)
        {
            memcpy(data, node_.data, mat_size);
            info = node_.info;
        }
        return *this;
    }
    

    mat_helper_mat_info info;
    char *data;
};


struct sem_node
{
    std::uint8_t max;
    std::uint8_t count;
    std::list<mat_helper_socket_t> wait_list;
    sem_node()
    {
        max = 255;
        count = 0;
    }
    sem_node(std::uint8_t max_)
    {
        max = max_;
        count = 0;
    }
    ~sem_node()
    {
        mat_helper_sem_wait_res res;
        memset(&res, 0, sizeof(res));
        res.type = MAT_HELPER_TYPE_SEM_WAIT_REQ;
        res.result = MAT_HELPER_ERR;
        for (auto i: wait_list)
        {
            mat_helper_write(i, (char *)&res, sizeof(res));
            mat_helper_close_socket(i);
        }
    }
};

std::map<std::string, node> mat_map;
std::map<std::string, sem_node> sem_map;
std::mutex mtx;

void write_req(mat_helper_socket_t c)
{
    mat_helper_write_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_write_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_WRITE_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
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
    
    node node_(data, req.info);
    
    if (mat_helper_read(c, data, mat_size) == -1)
    {
        free(data);
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    
    mtx.lock();
    mat_map[std::string(req.name)] = std::move(node_);
    mtx.unlock();
    
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
        return;
    }
    
    mtx.lock();
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        mtx.unlock();
        return;
    }
    
    node node_ = mat_map[std::string(req.name)];
    mtx.unlock();
    
    if (node_.data == NULL)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    
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
    if (-1 != mat_helper_write(c, (char *)node_.data, mat_size))
    {
        if (req.del)
        {
            mtx.lock();
            mat_map.erase(std::string(req.name));
            mtx.unlock();
        }
    }
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
        return;
    }
    
    mtx.lock();
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        mtx.unlock();
        return;
    }
    struct mat_helper_mat_info info = mat_map[std::string(req.name)].info;
    mtx.unlock();
    
    
    res.info.dims = info.dims;
    for (int i = 0; i < info.dims; ++i)
    {
        res.info.dim_size[i] = htonl(info.dim_size[i]);
    }
    res.info.type = info.type;
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
    mtx.lock();
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
    mtx.unlock();
    
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
        return;
    }
    
    mtx.lock();
    if (mat_map.count(std::string(req.name)) == 0)
    {
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        mtx.unlock();
        return;
    }
    mat_map.erase(std::string(req.name));
    mtx.unlock();
    
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
}






void sem_init_req(mat_helper_socket_t c)
{
    mat_helper_sem_init_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_sem_init_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_SEM_INIT_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        return;
    }
    
    mtx.lock();
    if (sem_map.count(std::string(req.name)) > 0)
    {
        mtx.unlock();
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    struct sem_node node(req.max);
    sem_map[std::string(req.name)] = node;
    mtx.unlock();
    
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
}

void sem_destroy_req(mat_helper_socket_t c)
{
    mat_helper_sem_destroy_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_sem_destroy_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_SEM_DESTROY_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        return;
    }
    
    mtx.lock();
    sem_map.erase(std::string(req.name));
    mtx.unlock();
    
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
}

int sem_wait_req(mat_helper_socket_t c)
{
    mat_helper_sem_wait_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_sem_wait_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_SEM_WAIT_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        return 0;
    }
    
    mtx.lock();
    if (sem_map.count(std::string(req.name)) == 0)
    {
        mtx.unlock();
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return 0;
    }
    if (sem_map[std::string(req.name)].count > 0)
    {
        sem_map[std::string(req.name)].count--;
        mtx.unlock();
        res.result = MAT_HELPER_OK;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return 0;
    }
    
    sem_map[std::string(req.name)].wait_list.push_back(c);
    mtx.unlock();
    return 1;
}


void sem_post_req(mat_helper_socket_t c)
{
    mat_helper_sem_post_req req;
    memset(&req, 0, sizeof(req));
    
    mat_helper_sem_post_res res;
    memset(&res, 0, sizeof(res));
    res.type = MAT_HELPER_TYPE_SEM_POST_RES;
    
    if (-1 == mat_helper_read(c, ((char *)&req) + 1, sizeof(req) - 1))
    {
        return;
    }
    
    mtx.lock();
    if (sem_map.count(std::string(req.name)) == 0)
    {
        mtx.unlock();
        res.result = MAT_HELPER_ERR;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    
    sem_node& node = sem_map[std::string(req.name)];
    if (node.wait_list.size() > 0)
    {
        mat_helper_socket_t wait_c = node.wait_list.front();
        node.wait_list.pop_front();
        mtx.unlock();
        
        mat_helper_sem_wait_res wait_res;
        memset(&wait_res, 0, sizeof(wait_res));
        wait_res.type = MAT_HELPER_TYPE_SEM_WAIT_REQ;
        wait_res.result = MAT_HELPER_ERR;
        mat_helper_write(wait_c, (char *)&wait_res, sizeof(wait_res));
        mat_helper_close_socket(wait_c);
        
        res.result = MAT_HELPER_OK;
        mat_helper_write(c, (char *)&res, sizeof(res));
        return;
    }
    if (node.count < node.max)
    {
        node.count++;
    }
    mtx.unlock();
    res.result = MAT_HELPER_OK;
    mat_helper_write(c, (char *)&res, sizeof(res));
}

static void session_thread(mat_helper_socket_t c)
{
    char buf[512];
    if (-1 == mat_helper_read(c, buf, 1))
    {
        return;
    }
        
    switch (buf[0])
    {
        case MAT_HELPER_TYPE_WRITE_REQ:
            write_req(c);
            break;
        case MAT_HELPER_TYPE_READ_REQ:
            read_req(c);
        case MAT_HELPER_TYPE_READ_INFO_REQ:
            read_info_req(c);
            break;
        case MAT_HELPER_TYPE_LIST_REQ:
            list_req(c);
            break;
        case MAT_HELPER_TYPE_DEL_REQ:
            del_req(c);
            break;
        case MAT_HELPER_TYPE_SEM_INIT_REQ:
            sem_init_req(c);
            break;
        case MAT_HELPER_TYPE_SEM_DESTROY_REQ:
            sem_destroy_req(c);
            break;
        case MAT_HELPER_TYPE_SEM_WAIT_REQ:
            if (sem_wait_req(c))
            {
                return;
            }
            break;
        case MAT_HELPER_TYPE_SEM_POST_REQ:
            sem_post_req(c);
            break;
    }
    mat_helper_close_socket(c);
    
    mtx.lock();
    for (auto i : sem_map)
    {
        printf("sem name: %s\n", i.first.c_str());
        printf("sem max%d\n", i.second.max);
        printf("sem count%d\n", i.second.count);
    }
    mtx.unlock();
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
