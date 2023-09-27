#include <stdio.h>
#include <file_system.h>
#include <stdlib.h>
#include <printk.h>
#include <arm.h>

dir_t* root;
dir_t level0[1];
dir_t level1[5];
dir_t level2[25];
char dir_list[2][50];


#define FOLDER_LENGTH 5
#define FILE_LENGTH 5
#define DATA_LENGTH 50

// kernel api to support assign string to another string
void kernel_assign_string(char* src, char* dst, int length) {
	
	for (int i = 0; i < length; i++) {
		if (dst[i] != ' ') src[i] = dst[i];
		else break;
	}
}

// kernel api to support comparing two string
int kernel_compare_string(char* str1, char* str2) {
	for (int i = 0; i < FOLDER_LENGTH; i++) {
		if (*str1 != *str2) return false;
		str1++;
		str2++;
	}
	return true;
}

// kernel api to support splitting path to several dir
void kernel_split_path(char *path, int length) {
	// breakpoint();
	int idx = 0;
	int first_dir_length = 0;
	for (int i = 0; i < length; i++) {
		if (path[i] == '/') {
			if (i == 0) continue;
			else {
				first_dir_length = i + 1;
				idx++;
			}
		} else {
			if (first_dir_length == 0) { 
				dir_list[idx][i - 1] = path[i];				
			} else {
				dir_list[idx][i%first_dir_length] = path[i];
			}
		}
	}
}

// kernel api to support initializing dir list and splitting path
void analysis_path(char* path) {
	// clear split dir
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 50; j++) {
			dir_list[i][j] = 0;
		}
	}
	int length = 0;
	char* tmp = path;
	while(*tmp != ' ') {
		length++;
		tmp++;
	}
	
	kernel_split_path(path, length);
}

// kernel api to support creating path (folder or file)
void kernel_create_path(char* path, int is_file) {
	analysis_path(path);
	// breakpoint();
	dir_t *cur = root;
	for (int i = 0 ; i < 2; i++) {
		bool find_folder = false;
		if (!is_file && i == 1) break;
		for (int j = 0; j < 5; j++) {
			// breakpoint();
			if (kernel_compare_string(cur->children[j]->dir, dir_list[i])) {
				cur = cur->children[j];
				find_folder = true;
				break;
			}
		}
		if (find_folder) continue;
		// create 
		for (int j = 0; j < 5; j++) {
			if (cur->children[j] == NULL) {
				create_dir_node(&cur->children[j], dir_list[i], i, j);
				cur = cur->children[j];
				break;
			}
		}
	}
}

// kernel api to support writing to file
int kernel_write_file(char *path, char* data) {
	analysis_path(path);
	// breakpoint();
	dir_t *cur = root;
	for (int i = 0 ; i < 2; i++) {
		bool find_folder = false;
		for (int j = 0; j < 5; j++) {
			// breakpoint();
			if (kernel_compare_string(cur->children[j]->dir, dir_list[i])) {
				cur = cur->children[j];
				if (i == 1) {
					kernel_assign_string(cur->data_buffer, data, DATA_LENGTH);
					return 0;
				}
				find_folder = true;
				break;
			}
		}
		if (!find_folder) return -1;
	}
	return -1;
}

// kernel api to support reading from file
uint32_t kernel_read_file(char* path) {
	analysis_path(path);
	dir_t *cur = root;
	// breakpoint();
	for (int i = 0 ; i < 2; i++) {
		bool find_folder = false;
		for (int j = 0; j < 5; j++) {
			if (kernel_compare_string(cur->children[j]->dir, dir_list[i])) {
				cur = cur->children[j];
				if (i == 1) {
					// breakpoint();
					return (uint32_t)cur->data_buffer;
				}
				find_folder = true;
				break;
			}
		}
		if (!find_folder) return 0;
	}
	return 0;
}

// kernel api to support deleting path (folder or path)
int kernel_delete_path(char* path) {
	analysis_path(path);
	if (dir_list[1][0] != 0) {
		dir_t *cur = root;
		for (int i = 0 ; i < 2; i++) {
			bool find_folder = false;
			for (int j = 0; j < 5; j++) {
				if (kernel_compare_string(cur->children[j]->dir, dir_list[i])) {
					cur = cur->children[j];
					if (i == 1) {
						cur->is_file = 0;
						clear_arr(cur->dir);
						clear_arr(cur->data_buffer);
						cur = 0;
						// breakpoint();
						return 0;
					}
					find_folder = true;
					break;
				}
			}
			if (!find_folder) return -1;
		}
	} else {
		dir_t *cur = root;
		for (int i = 0; i < 5; i++) {
			if (kernel_compare_string(cur->children[i]->dir, dir_list[0])) {
				cur = cur->children[i];
				for (int j = 0; j < 5; j++) {
					dir_t* child = cur->children[j];
					child->is_file = 0;
					clear_arr(child->dir);
					clear_arr(child->data_buffer);
					child = 0;
				}
				cur->is_file = 0;
				clear_arr(cur->dir);
				clear_arr(cur->data_buffer);
				cur = 0;
				return 0;
			}
		}
		return -1;
	}
	return 0;
}

// kernel api to support copying path
int kernel_copy_path(char *path1, char* path2) {
	uint32_t data_addr = kernel_read_file(path1);
	if (data_addr == 0) return -1;

	char new_path2[50];
	for (int i = 0 ; i < 6; i++) {
		new_path2[i] = path2[i];
	}
	new_path2[6] = '/';
	
	for (int i = 0; i < 5 ; i++) {
		new_path2[i + 7] = dir_list[1][i];
	}
	new_path2[12] = ' ';
	kernel_create_path(new_path2, 1);
	kernel_write_file(new_path2, (char*)data_addr);
	return 0;
}

// api support create dir node
void create_dir_node(dir_t** cur, char* dir, int level, int idx) {
	// breakpoint();
	if (level == 0) {
		*cur = &level1[idx];
		kernel_assign_string((*cur)->dir, dir, FOLDER_LENGTH);
		(*cur)->is_file = 0;
	} else {
		*cur = &level2[idx];
		kernel_assign_string((*cur)->dir, dir, FILE_LENGTH);
		(*cur)->is_file = 1;
	}
}

//api support clear arr
void clear_arr(char* arr) {
	for (int i = 0; i < 50; i++) {
		arr[i] = 0;
	}
}

//api support init file system
void kernel_init_file_system(void) {

	// init root dir
	root = &level0[0];
	kernel_assign_string(root->dir, "/", 1);
	root->is_file = false;
}