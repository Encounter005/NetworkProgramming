#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 10005
#define BUFFLEN 1024

/*
 * 创建套接字并返回其文件描述符
 * 如果创建失败，程序将以状态1退出
 */
int create_socket() {
    int sock_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock_fd < 0 ) {
        perror( "socket创建失败" ); // 使用perror提供更多的错误信息
        exit( EXIT_FAILURE );
    }
    printf( "socket create successfully\n" );
    return sock_fd;
}

/*
 * 初始化服务器地址结构的函数
 */
void initialize_address( struct sockaddr_in *server_addr ) {
    memset( server_addr, 0, sizeof( *server_addr ) );
    server_addr->sin_family      = AF_INET;
    server_addr->sin_port        = htons( SERVER_PORT );
    server_addr->sin_addr.s_addr = inet_addr( "192.168.5.120" ); // 服务器IP
}

/*
 * 建立到服务器的连接
 * 如果连接失败，程序将以状态1退出
 */
void connect_to_server( int socket_fd, const struct sockaddr *server_addr ) {
    if ( connect( socket_fd, server_addr, sizeof( *server_addr ) ) < 0 ) {
        perror( "connection failed" );
        exit( EXIT_FAILURE );
    }
    printf( "connection successful\n" );
}

/*
 * 客户端程序开始的主函数
 */
int main() {
    int socket_fd = create_socket(); // 创建socket
    struct sockaddr_in server_addr;
    initialize_address( &server_addr ); // 初始化服务器详细信息

    connect_to_server(
        socket_fd, (struct sockaddr *)&server_addr ); // 连接到服务器

    char send_buf[BUFFLEN] = { 0 }; // 用于发送数据的缓冲区
    char recv_buf[BUFFLEN] = { 0 }; // 用于接收数据的缓冲区

    // 持续从用户获取输入并发送到服务器，直到从服务器接收到"bye!!!"
    while ( fgets( send_buf, BUFFLEN, stdin ) ) {
        // 向服务器写入用户输入
        if ( write( socket_fd, send_buf, strnlen( send_buf, BUFFLEN ) ) ==
             -1 ) {
            perror( "write failed" );
            break;
        }
        printf( "Me(Client):%s", send_buf ); // 打印用户输入
        memset( send_buf, 0, BUFFLEN );      // 清除发送缓冲区

        // 读取服务器的响应
        if ( read( socket_fd, recv_buf, BUFFLEN ) == -1 ) {
            perror( "receive failed" );
            break;
        }
        printf( "Server:%s", recv_buf ); // 打印服务器的响应
        if ( strcmp( recv_buf, "bye!!!" ) == 0 ) {
            break;
        }
        memset( recv_buf, 0, BUFFLEN ); // 清除接收缓冲区
    }

    close( socket_fd ); // 关闭套接字
    return 0;
}
