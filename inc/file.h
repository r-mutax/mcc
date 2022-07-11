#ifndef FILE_INC_H
#define FILE_INC_H

#include "mcc.h"
char* read_file(char* path);
void register_include_directory(char* path);
char* get_include_path(char* path);
char* get_filename(SrcFile* src_file);

#endif
