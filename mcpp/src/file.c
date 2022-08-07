#include "mcpp.h"

static int get_file_size(char* path);

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

char* get_filename(SrcFile* src_file){
    char* yen_pos = strrchr(src_file->path, '/');
    return strndup(yen_pos + 1, strlen(src_file->path) + (yen_pos - src_file->path) - 1);
}
