/*
 *
 * filename: server.c
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
#define BUFFLEN 1024
#define BACLOG 10

int create_socket() {
    int res = socket( AF_INET, SOCK_STREAM, 0 );
    if ( res < 0 ) {
        perror( "socket creation failed" );
        exit( EXIT_FAILURE );
    }
    printf( "socket created successfully\n" );
    return res;
}

void initialize_address( struct sockaddr_in *server_addr ) {
    memset( server_addr, 0, sizeof( *server_addr ) );
    server_addr->sin_family      = AF_INET;
    server_addr->sin_addr.s_addr = htons( INADDR_ANY );
    server_addr->sin_port        = htons( SERVER_PORT );
}

void try_bind( int socket_fd, const struct sockaddr *server_addr ) {
    if ( bind( socket_fd, server_addr, sizeof( *server_addr ) ) < 0 ) {
        perror( "bind failed" );
        exit( EXIT_FAILURE );
    }
    puts( "bind successful" );
}

void process_client_request( int socket_client ) {
    char send_buf[BUFFLEN] = { 0 };
    char recv_buf[BUFFLEN] = { 0 };
    snprintf( send_buf, BUFFLEN, "Hi, I am server. I received your message." );

    while ( 1 ) {
        ssize_t num_read = read( socket_client, recv_buf, BUFFLEN );
        if ( num_read <= 0 ) {
            if ( num_read == 0 )
                puts( "Client disconnected" );
            else
                perror( "read failed" );
            return;
        }
        recv_buf[num_read] = '\0';

        printf( "Client:%s\n", recv_buf );

        if ( strncmp( recv_buf, "exit", 4 ) == 0 ) {
            strcpy( send_buf, "bye!!!" );
            write( socket_client, send_buf,
                strlen(
                    send_buf ) ); // Not checking the return value for brevity
            printf( "Me(Server):%s\n", send_buf );
            return;
        }

        if ( write( socket_client, send_buf, strlen( send_buf ) ) < 0 ) {
            perror( "send failed" );
            return;
        }
        printf( "Me(Server):%s\n", send_buf );
        bzero( recv_buf, BUFFLEN );
    }
}

int main() {
    int socket_server = create_socket();
    struct sockaddr_in server_addr;

    initialize_address( &server_addr );
    try_bind( socket_server, (struct sockaddr *)&server_addr );

    if ( listen( socket_server, BACLOG ) == -1 ) {
        perror( "listen failed" );
        exit( EXIT_FAILURE );
    }
    puts( "Start to listen" );

    while ( 1 ) {
        int socket_client = accept( socket_server, NULL, NULL );
        if ( socket_client < 0 ) {
            perror( "accept failed" );
            continue; // Attempt to accept another connection
        }
        process_client_request( socket_client );
        close( socket_client );
    }

    // This point is normally not reached
    close( socket_server );
    return 0;
}
