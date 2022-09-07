#include "mcpp.h"

extern bool opt_debug_output;

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
        fprintf(stderr, "invalid file path. %s\n", path);
        error("");
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

    int len = strlen(path);
    if(path[len - 1] != '/'){
        dir->dir = calloc(len + 2, sizeof(char));
        sprintf(dir->dir, "%s/", path);
    } else {
        dir->dir = strdup(path);
    }
    
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
            char* ret = calloc(strlen(path) + 2, sizeof(char));
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
            char* ret = calloc(strlen(path) + 2, sizeof(char));
            strcpy(ret, path);
            return ret;
        }
        cur = cur->next;
    }
    return NULL;
}


char* find_include_next_file(char* include_name){

    bool is_found = false;

    // step 1 serch at StdIncDir
    IncludeDir* cur = StdIncDir;
    while(cur){
        char path[256] = { 0 };
        strcpy(path, cur->dir);
        strcat(path, include_name);
        if(is_file_exist(path)){
            if(is_found){
                char* ret = calloc(strlen(path) + 2, sizeof(char));
                strcpy(ret, path);
                return ret;
            } else {
                is_found = true;
            }
        }
        cur = cur->next;
    }

    // step 2 serch at IncDir
    cur = IncDir;
    while(cur){
        char path[256] = { 0 };
        strcpy(path, cur->dir);
        strcat(path, include_name);
        if(is_file_exist(path)){
            if(is_found){
                char* ret = calloc(strlen(path) + 2, sizeof(char));
                strcpy(ret, path);
                return ret;
            } else {
                is_found = true;
            }
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

    bool is_top_line = true;

    while(tok){

        if(tok->kind != PTK_SPACE && tok->kind != PTK_NEWLINE){
            is_top_line = false;
        }

        switch(tok->kind){
            case PTK_STRING_CONST:
                fprintf(fp, "\"");
                fprintf(fp, "%s", tok->str);
                fprintf(fp, "\"");
                break;
            case PTK_CHAR_CONST:
                fprintf(fp, "'");

                switch(tok->val){
                    case 0x0a:
                        fprintf(fp, "\\n");     // LF
                        break;
                    case 0xd:
                        fprintf(fp, "\\r");      // CR
                        break;
                    case 0x00:
                        fprintf(fp, "\\0");     // null
                        break;
                    case 0x09:
                        fprintf(fp, "\\t");     // horizontal tab
                        break;
                    case 0x5c:                  //  \\ mark
                        fprintf(fp, "\\\\");
                        break;
                    default:
                        fprintf(fp, "%c", tok->val);
                        break;
                }
                fprintf(fp, "'");
                break;
            case PTK_PLACE_MAKER:
                break;
            case PTK_NEWLINE:
                fprintf(fp, "\n");
                if(opt_debug_output && !is_top_line){
                    fprintf(fp, "%s", tok->str);
                    fprintf(fp, "# %s %ld\n", tok->src->path, tok->row);
                    is_top_line = true;
                }
                break;
            case PTK_SPACE:
                if(!is_top_line){
                    // space token length convert to one character.
                    fprintf(fp, " ");
                }
                break;
            case PTK_NUM:
                fprintf(fp, "%s", tok->str);
                if(tok->is_hex){
                    int a = 0;
                }
                break;
            case PTK_NOTHING:
            case PTK_EOF:
            //case PTK_PP_KEYWORD:
            case PTK_HASH:
            case PTK_HASH_HASH:
                break;
            default:
                fprintf(fp, "%s", tok->str);
                break;
        }

        tok = tok->next;
    }
    fclose(fp);
}
