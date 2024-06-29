/*
 *
 * filename: client.c
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 10005
#define CONNECT_NUM 5
#define BUFFLEN 1024

int create_socket() {
    int res = socket( AF_INET, SOCK_STREAM, 0 );
    if ( res < 0 ) {
        printf( "socket creations failed\n" );
        exit( 1 );
    }

    printf( "socket created successfully\n" );
    return res;
}

void initialize_address( struct sockaddr_in *server_addr ) {
    memset( server_addr, 0, sizeof( *server_addr ) );
    server_addr->sin_family      = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr("192.168.5.120");
    server_addr->sin_port        = htons( SERVER_PORT );
}

void try_connect( int socket_fd, const struct sockaddr *server_addr ) {
    int connect_res = connect( socket_fd, server_addr, sizeof( *server_addr ) );
    if ( connect_res < 0 ) {
        puts( "connection failed" );
        exit( 1 );
    }

    puts( "connection successful" );
}

int main() {
    int socket_fd = -1;
    socket_fd     = create_socket();

    struct sockaddr_in server_addr;

    initialize_address( &server_addr );
    try_connect( socket_fd, (struct sockaddr *)&server_addr );

    char send_buf[BUFFLEN] = { 0 };
    char recv_buf[BUFFLEN] = { 0 };

    while ( fgets( send_buf, BUFFLEN, stdin ) ) {
        if ( write( socket_fd, send_buf, BUFFLEN ) == -1 ) {
            puts( "write failed" );
        }
        printf( "Me(Client):%s\n", send_buf );
        bzero( send_buf, BUFFLEN );

        if ( read( socket_fd, recv_buf, BUFFLEN ) == -1 ) {
            puts( "receive failed" );
        }
        printf( "Server:%s\n", recv_buf );
        if ( strcmp( recv_buf, "bye!!!" ) == 0 ) {
            break;
        }
        bzero( recv_buf, BUFFLEN );
    }

    close( socket_fd );

    return 0;
}
