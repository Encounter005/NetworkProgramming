#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 10005
#define BUFFLEN 1024
#define BACLOG 10

// 创建新的socket，返回文件描述符
int create_socket() {
    int res = socket(AF_INET, SOCK_STREAM, 0);
    if (res < 0) {
        perror("create socket failed");
        exit(EXIT_FAILURE);
    }
    printf("create socket successfully\n");
    return res;
}

// 初始化服务器地址信息
void initialize_address(struct sockaddr_in *server_addr) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    // 监听所有可用地址
    server_addr->sin_addr.s_addr = htons(INADDR_ANY);
    server_addr->sin_port        = htons(SERVER_PORT);
}

// 绑定socket到指定的地址
void try_bind(int socket_fd, const struct sockaddr *server_addr) {
    if (bind(socket_fd, server_addr, sizeof(*server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    puts("bind successfully");
}

// 处理客户端请求
void process_client_request(int socket_client) {
    char send_buf[BUFFLEN] = {0};
    char recv_buf[BUFFLEN] = {0};


    while (1) {
        ssize_t num_read = read(socket_client, recv_buf, BUFFLEN);
        // 客户端断开连接，或读取失败
        if (num_read <= 0) {
            if (num_read == 0)
                puts("Client closed connection");
            else
                perror("read failed");
            return;
        }
        recv_buf[num_read] = '\0';   // 为接收的字符串添加结束字符

        printf("From Client: %s\n", recv_buf);

        if (strncmp(recv_buf, "exit", 4) == 0) {   // 客户端请求退出
            // 修改发送的消息
            strcpy(send_buf, "bye!!!");
            // 返回消息给客户端
            write(socket_client, send_buf, strlen(send_buf));
            printf("Me(Server)：%s\n", send_buf);
            return;
        }
        // 发送的消息，以方便日后添加或修改
        snprintf(
            send_buf,
            BUFFLEN,
            "Hello, this is the server, your message is %s",
            recv_buf);

        // 向客户端写入发送缓冲区的内容
        if (write(socket_client, send_buf, strlen(send_buf)) < 0) {
            perror("write failed");
            return;
        }
        printf("Me(Server): %s\n", send_buf);

        // 清空接收缓冲区，准备下一次读取
        memset(recv_buf, 0, BUFFLEN);
        memset(send_buf, 0, BUFFLEN);
    }
}

int main() {
    int                socket_server = create_socket();   // 创建socket
    struct sockaddr_in server_addr;                       // 声明地址结构

    initialize_address(&server_addr);   // 初始化地址结构
    try_bind(
        socket_server, (struct sockaddr *) &server_addr);   // 绑定地址到socket

    // 开始监听连接请求，设定最大等待连接数量为BACLOG
    if (listen(socket_server, BACLOG) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    puts("start listening");

    struct sockaddr_in client_addr;
    socklen_t          client_addr_size = sizeof(client_addr);

    while (1) {
        // 初始化客户端地址结构
        memset(&client_addr, 0, sizeof(client_addr));
        // 接受新的连接请求
        int socket_client = accept(
            socket_server, (struct sockaddr *) &client_addr, &client_addr_size);
        printf(
            "Client %s:%d connected\n",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));
        if (socket_client < 0) {
            perror("receive failed");
            continue;   // 尝试接受另一个连接
        }
        // 满足要求后开始处理客户端请求
        process_client_request(socket_client);
        close(socket_client);   // 关闭客户端连接
    }

    // 正常情况下不会执行到这一步
    close(socket_server);
    return 0;
}
