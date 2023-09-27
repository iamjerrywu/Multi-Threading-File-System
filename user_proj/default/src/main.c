/** @file   default/main.c
 *
 *  @brief  user-space project "default", demonstrates system call use
 *  @note   Not for public release, do not share
 *
 *  @date   last modified 11 October 2022
 *  @author CMU 14-642
 *
 *  @output repeatedly prints provided argument to console
**/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lib642.h>
#include <stdbool.h>
// #include <user642.h>

extern void init_file_system(void);
extern int create_path(char* path, int is_file);
extern int write_file(char* path, char* data);
extern int read_file(char* path);
extern int delete_path(char* path);
extern int copy_path(char* path1, char* path2);


typedef enum action action_e;
enum action {
    CREATE = 0,
    WRITE = 1,
    READ = 2,
    COPY = 3,
    DELETE = 4,
    DEFAULT = 5,
};

void command_parse(char* buffer);
void parse(char* buffer);
#define UNUSED __attribute__((unused))
char buf[1];
char command[100];

action_e act;
char path[100];
char third_arg[100];


int main(UNUSED int argc, char* const argv[]) {
    (void)argv;
    int ptr = 0;

    init_file_system();
    for (int i = 0; i < 100; i++) {
        // command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }
    while(1) {
        if(read(0, buf, 1) != 0) {
            if (buf[0] == '\r') {
                command[ptr] = ' ';
                // write(1, command, ptr);
                ptr = 0;
                command_parse(command);
                for (int i = 0; i < 100; i++) {
                    command[i] = ' ';
                    path[i] = ' ';
                    third_arg[i] = ' ';
                }
            } else {
                command[ptr] = buf[0];
                ptr++;
            }
        }
    }
    return 0;
}

void command_parse(char* buffer) {
    parse(buffer);
    switch(act) {

        case CREATE:
            if (create_path(path, (int)(third_arg[0] - '0')) == 0) {
                printf("Create Success!\n");
            } else {
                printf("Create Failed!\n");
            }
            break;
        case WRITE:
            if (write_file(path, third_arg) == 0) {
                printf("Write Success!\n");
            } else {
                printf("Write Failed\n");
            }
            break;
        case READ:
            printf("Data: %s\n", (char*)read_file(path));
            break;
        case COPY:
            printf("COPY\n");
            copy_path(path, third_arg);
            break;
        case DELETE:
            if (delete_path(path) == 0) {
                printf("Delete Success!\n");
            } else {
                printf("Delete Failed\n");
            }
            break;
        default:
            // printf("Error!\n");
            break;
    }
}


void parse(char* buffer) {
    int idx = 0;
    char action[6];

    // parse act
    while (*buffer != ' ') {
        action[idx] = *buffer;
        idx++;
        buffer++;
    };
    buffer++;
    if (action[0] == 'c' && action[1] == 'r') act = CREATE;
    if (action[0] == 'w') act = WRITE;
    if (action[0] == 'r') act = READ;
    if (action[0] == 'c' && action[1] == 'o') act = COPY;
    if (action[0] == 'd') act = DELETE;

    // parse path
    idx = 0;
    while (*buffer != ' ') {
        path[idx] = *buffer;
        idx++;
        buffer++;
    };
    buffer++;
    
    // parse third arg
    idx = 0;
    if (act == CREATE || act == WRITE || act == COPY) {
        while (*buffer != ' ') {
            third_arg[idx] = *buffer;
            idx++;
            buffer++;
        };
    }
}