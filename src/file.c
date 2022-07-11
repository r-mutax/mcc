#include "mcc.h"
#include "file.h"
#include "errormsg.h"
#include "utility.h"

IncDir* inc_dir_dict;

static bool is_exist_path(char* path);

char* read_file(char* path){
    
    FILE *fp = fopen(path, "r");
    if(!fp){
        error("invalid file path.");
    }

    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

void register_include_directory(char* path){

    int len = strlen(path);
    int is_slash = path[len - 1] != '/';

    IncDir* inc = calloc(1, sizeof(IncDir));
    inc->path = calloc(1, strlen(path) + is_slash);
    strcpy(inc->path, path);

    if(is_slash)
        strcat(inc->path, "/");

    inc->next = inc_dir_dict;
    inc_dir_dict = inc;    
}

char* get_include_path(char* path){
    
    IncDir* cur = inc_dir_dict;
    int path_len = strlen(path);

    while(cur){
        char* inc_path = calloc(1, strlen(cur->path) + path_len);
        sprintf(inc_path, "%s%s", cur->path, path);
        if(is_exist_path(inc_path)){
            return inc_path;
        }

        free(inc_path);
        cur = cur->next;
    }

    error("can not find include file.\n");
    return NULL;
}

static bool is_exist_path(char* path){
    FILE* fp = fopen(path, "r");

    if(fp == NULL){
        return false;
    }
    fclose(fp);
    
    return true;
}

char* get_filename(SrcFile* src_file){
    char* yen_pos = strrchr(src_file->path, '/');
    return strndup(yen_pos + 1, strlen(src_file->path) + (yen_pos - src_file->path) - 1);
}