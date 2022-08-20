#include "mcpp.h"

static IncludeDir* IncDir = NULL;
static IncludeDir* IncTail = NULL;
static IncludeDir* StdIncDir = NULL;
static IncludeDir* StdIncTail = NULL;
static int get_file_size(char* path);
static bool is_file_exist(char* path);

char* read_file(char* path){
    
    int file_size = get_file_size(path);

    FILE *fp = fopen(path, "r");
    if(!fp){
        error("invalid file path.");
    }

    char *buf = calloc(1, file_size + 2);
    fread(buf, file_size, 1, fp);

    if (file_size == 0 || buf[file_size - 1] != '\n')
    buf[file_size++] = '\n';
    fclose(fp);

    return buf;
}

static int get_file_size(char* path){
    
    FILE* fp = fopen(path, "r");
    if(!fp){
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    
    fclose(fp);
    return size;
}

static bool is_file_exist(char* path){
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

void add_include_path(char* path){

    IncludeDir* dir = calloc(1, sizeof(IncludeDir));
    dir->dir = strdup(path);
    
    if(IncDir){
        IncTail = IncTail->next = dir;
    } else {
        IncDir = IncTail = dir;
    }
}

void add_std_include_path(char* path){

    IncludeDir* dir = calloc(1, sizeof(IncludeDir));
    dir->dir = strdup(path);
    
    if(StdIncDir){
        StdIncTail = StdIncTail->next = dir;
    } else {
        StdIncDir = StdIncTail = dir;
    }
}

char* find_include_file(char* include_name){
    IncludeDir* cur = IncDir;
    while(cur){
        char path[256] = { 0 };
        strcpy(path, cur->dir);
        strcat(path, include_name);
        if(is_file_exist(path)){
            char* ret = calloc(strlen(path), sizeof(char));
            strcpy(ret, path);
            return ret;
        }
        cur = cur->next;
    }
    return NULL;
}

char* find_std_include_file(char* include_name){
    IncludeDir* cur = StdIncDir;
    while(cur){
        char path[256] = { 0 };
        strcpy(path, cur->dir);
        strcat(path, include_name);
        if(is_file_exist(path)){
            char* ret = calloc(strlen(path), sizeof(char));
            strcpy(ret, path);
            return ret;
        }
        cur = cur->next;
    }
    return NULL;
}

char* get_file_directory(char* filename, char* directory){
    int len = strrchr(filename, '/') - filename + 1;
    strncpy(directory, filename, len);
}

void output_preprocessed_file(PP_Token* tok, FILE* fp){
    while(tok){
        if(tok->kind == PTK_STRING_CONST){
            fprintf(fp, "\"");
        }
        fprintf(fp, "%s", tok->str);
        if(tok->kind == PTK_STRING_CONST){
            fprintf(fp, "\"");
        }
        tok = tok->next;
    }
    fclose(fp);
}
