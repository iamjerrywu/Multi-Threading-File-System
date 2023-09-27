/** @file   default/file_system.c
 *
 *  @brief  user-space project "file_system", demonstrates system call use
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

/** @brief thread user space stack size - 1KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 16
#define NUM_MUTEXES 10
#define CLOCK_FREQUENCY 1000
#define C 50
#define T 100
#define TEST_ENABLE // comment out if only focus on interaction unit test

extern void init_file_system(void);
extern int create_path(char* path, int is_file);
extern int write_file(char* path, char* data);
extern int read_file(char* path);
extern int delete_path(char* path);
extern int copy_path(char* path1, char* path2);

typedef struct {
    mutex_t* mutex_0;
    int done;
} arg_t;

typedef enum action action_e;
enum action {
    CREATE = 0,
    WRITE = 1,
    READ = 2,
    COPY = 3,
    DELETE = 4,
    TEST = 5,
    DEFAULT = 6,
};

// integration / multi-threading test script
/*  Ideally eventually should see log like followed on console:

    Create Success!
    Create Success!
    Create Success!
    Create Success!
    Create Success!
    Create Success!
    Write Success!
    Write Success!
    Write Success!
    Write Success!
    Write Success!
    Data: H
    Data: E
    Data: L
    Data: L
    Data: O
    Create Success!
    Copy Success!
    Copy Success!
    Copy Success!
    Copy Success!
    Copy Success!
    Data: H
    Data: E
    Data: L
    Data: L
    Data: O
    Delete Success!
    Data:
    Data:
    Data:
    Data:
    Data:
*/
char test_script[33][50] = {
    "create /fold1 0\r ",
    "create /fold1/file1 1\r ",
    "create /fold1/file2 1\r ",
    "create /fold1/file3 1\r ",
    "create /fold1/file4 1\r ",
    "create /fold1/file5 1\r ",
    "write /fold1/file1 H\r ",
    "write /fold1/file2 E\r ",
    "write /fold1/file3 L\r ",
    "write /fold1/file4 L\r ",
    "write /fold1/file5 O\r ",
    "read /fold1/file1\r ",
    "read /fold1/file2\r ",
    "read /fold1/file3\r ",
    "read /fold1/file4\r ",
    "read /fold1/file5\r ",
    "create /fold2 0\r ",
    "copy /fold1/file1 /fold2\r ",
    "copy /fold1/file2 /fold2\r ",
    "copy /fold1/file3 /fold2\r ",
    "copy /fold1/file4 /fold2\r ",
    "copy /fold1/file5 /fold2\r ",
    "read /fold2/file1\r ",
    "read /fold2/file2\r ",
    "read /fold2/file3\r ",
    "read /fold2/file4\r ",
    "read /fold2/file5\r ",
    "delete /fold2\r ",
    "read /fold2/file1\r ",
    "read /fold2/file2\r ",
    "read /fold2/file3\r ",
    "read /fold2/file4\r ",
    "read /fold2/file5\r ",
};

void command_parse(char* buffer, arg_t* arg);
void parse(char* buffer);
#define UNUSED __attribute__((unused))
char buf[1];
char command[100];

action_e act;
char path[100];
char third_arg[100];
int priority = 1;

void thread_spawner(UNUSED void* vargp) {
    int ptr = 0;
    mutex_t* s0 = mutex_init(0);
    if(s0 == NULL) {
        printf("Failed to create mutex 0\n");
    }

    arg_t* arg = malloc(sizeof(arg_t));
    if(arg == NULL) {
        printf("Malloc failed\n");
    }
    arg->mutex_0 = s0;
    arg->done = 0;
    // thread_create(&thread_0, 0, 100, 500,(void*)arg)

    while(1) {

        if(read(0, buf, 1) != 0) {
            if (buf[0] == '\r') {
                command[ptr] = ' ';
                ptr = 0;
                command_parse(command, arg);
            } else {
                command[ptr] = buf[0];
                ptr++;
            }
        }
    }
}

void thread_create_path(void *vargp) {
    arg_t* arg =(arg_t*)vargp;
    (void)arg;
    mutex_lock(arg->mutex_0);
    if (create_path(path, (int)(third_arg[0] - '0')) == 0) {
        printf("Create Success!\n");
    } else {
        printf("Create Failed!\n");
    }
    for (int i = 0; i < 100; i++) {
        command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }
    mutex_unlock(arg->mutex_0);

    return;
}   

void thread_write_file(void *vargp) {
    arg_t* arg =(arg_t*)vargp;
    (void)arg;
    mutex_lock(arg->mutex_0);
    if (write_file(path, third_arg) == 0) {
        printf("Write Success!\n");
    } else {
        printf("Write Failed\n");
    }
    for (int i = 0; i < 100; i++) {
        command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }
    mutex_unlock(arg->mutex_0);
    return;
}

void thread_read_file(void *vargp) {
    arg_t* arg =(arg_t*)vargp;
    (void)arg;
    mutex_lock(arg->mutex_0);
    printf("Data: %s\n", (char*)read_file(path));
    for (int i = 0; i < 100; i++) {
        command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }
    mutex_unlock(arg->mutex_0);
    return;
}

void thread_copy_path(void *vargp) {
    arg_t* arg =(arg_t*)vargp;
    (void)arg;
    mutex_lock(arg->mutex_0);
    if (copy_path(path, third_arg) == 0) {
        printf("Copy Success!\n");
    } else {
        printf("Copy Failed\n");
    }
    for (int i = 0; i < 100; i++) {
        command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }
    mutex_unlock(arg->mutex_0);
    return;        
}

void thread_delete_path(void *vargp) {
    arg_t* arg =(arg_t*)vargp;
    (void)arg;
    mutex_lock(arg->mutex_0);
    
    if (delete_path(path) == 0) {
        printf("Delete Success!\n");
    } else {
        printf("Delete Failed\n");
    }
    for (int i = 0; i < 100; i++) {
        command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }
    mutex_unlock(arg->mutex_0);
    return;   
}

void thread_spawner_for_test(UNUSED void* vargp) {
    mutex_t* s0 = mutex_init(0);
    if(s0 == NULL) {
        printf("Failed to create mutex 0\n");
    }

    arg_t* arg = malloc(sizeof(arg_t));
    if(arg == NULL) {
        printf("Malloc failed\n");
    }
    arg->mutex_0 = s0;
    arg->done = 0;

    for (int i = 0; i < 33; i++) {
        // printf("%s\n", test_script[i]);
        command_parse(test_script[i], arg);
        // command_parse("create /folder1 0\r", arg);
        for (int j = 0; j < 1000000; j++) {

        }   
    }
    while(1);   
}



int main(UNUSED int argc, char* const argv[]) {
    (void)argv;

    init_file_system();
    for (int i = 0; i < 100; i++) {
        // command[i] = ' ';
        path[i] = ' ';
        third_arg[i] = ' ';
    }

    ABORT_ON_ERROR(thread_init(NUM_THREADS, USR_STACK_WORDS, NULL, KERNEL_ONLY, NUM_MUTEXES));

    #ifdef TEST_ENABLE
    // interation testing script
    ABORT_ON_ERROR(thread_create(&thread_spawner_for_test, 0, 100, 500, NULL));
    #else 
    // unit test on interaction
    ABORT_ON_ERROR(thread_create(&thread_spawner, 0, 100, 500, NULL));
    #endif
    ABORT_ON_ERROR(scheduler_start(CLOCK_FREQUENCY));
   
    return 0;
}

void command_parse(char* buffer, arg_t* arg) {
    parse(buffer);
    switch(act) {

        case CREATE:
            thread_create((void*)&thread_create_path, priority%11 + 1, C, T, (void*)arg);
            priority++;
            break;
        case WRITE:
            thread_create((void*)&thread_write_file, priority%11 + 1, C, T, (void*)arg);
            priority++;
            break;
        case READ:
            thread_create((void*)&thread_read_file, priority%11 + 1, C, T, (void*)arg);
            priority++;
            break;
        case COPY:
            thread_create((void*)&thread_copy_path, priority%11 + 1, C, T, (void*)arg);
            priority++;
            break;
        case DELETE:
            thread_create((void*)&thread_delete_path, priority%11 + 1, C, T, (void*)arg);
            priority++;
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
    if (action[0] == 't') act = TEST;

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