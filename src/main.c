#include "utils/headers/server.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments.\n");
        fprintf(stderr, "Usage: %s <ip> <port> <logfile>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *ip_str = argv[1];
    int port = atoi(argv[2]);
    // const char *log_path = argv[3];
    
    srand(time(NULL));
    Server *server = create_server(ip_str, port);
    
    if (!server) {
        fprintf(stderr, "Something went wrong starting the server.\n");
        return EXIT_FAILURE;
    }

    run_server(server);
    stop_server(server);
    return EXIT_SUCCESS;
}