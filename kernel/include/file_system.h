#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_
#include <stdbool.h>

#define TEST_ENABLE // comment out if only focus on interaction unit test


#define CLEAR_ARR(arr, size) {           \
	while(0)                             \
		for (int i = 0; i < size; i++) { \
			arr[i] = 0;                  \
		}                                \
	}									 \
}	

typedef struct dir_node dir_t;
struct dir_node {
	dir_t* children[5]; 
	char dir[50]; // /folder1, /file1
	int is_file;
	char data_buffer[50]; // can support up to 50 bytes
};

void kernel_init_file_system(void);
void kernel_create_path(char* path, int is_file);
void kernel_split_path(char *path, int length);
void kernel_assign_string(char* src, char* dst, int length);
void create_dir_node(dir_t** cur, char* dir, int level, int idx);
int kernel_compare_string(char* str1, char* str2);
int kernel_write_file(char* path, char* data);
uint32_t kernel_read_file(char* path);
int kernel_delete_path(char* path);
int kernel_copy_path(char *path1, char* path2);
void clear_arr(char* arr);

#endif