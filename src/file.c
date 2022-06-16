#include "file.h"
#include "errormsg.h"

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
