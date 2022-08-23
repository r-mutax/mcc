#include "mcc.h"

char* cur_dir = NULL;
char* src_file = NULL;
char* output_file = NULL;
ARG_VEC* inc_vec = NULL;

char* stream_buf = NULL;
size_t stream_size = 0;

// stream function
static void init_stream(size_t sz);
static void flush_stream();
static void flow_to_stream(char* data);
static char* get_stream();
char* get_file_directory(char* filename, char* directory);

int main(int argc, char** argv){

    MCC_OPTION opt = SRC_FILE;

    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case 'c':
                case 'C':
                    opt = SRC_FILE;
                    break;
                case 'i':
                case 'I':
                    opt = INCLUDE_PATH;
                    break;
                case 'o':
                case 'O':
                    opt = OUTPUT_FILE;
                    break;
            }
            continue;
        }

        switch(opt){
            case SRC_FILE:
                src_file = argv[i];
                break;
            case INCLUDE_PATH:
                {
                    ARG_VEC* arg = calloc(1, sizeof(ARG_VEC));
                    arg->str = argv[i];
                    arg->next = inc_vec;
                    inc_vec = arg;
                }
                break;
            case OUTPUT_FILE:
                output_file = argv[i];
                break;
        }
    }

    cur_dir = calloc(256, sizeof(char));
    get_file_directory(argv[0], cur_dir);

    // ----------------------------
    // make command
    init_stream(1000);

    // mcpp command
    flow_to_stream(cur_dir);

    flow_to_stream("mcpp -c ");
    flow_to_stream(src_file);
    
    for(ARG_VEC* cur = inc_vec; cur; cur = cur->next){
        flow_to_stream(" -i ");
        flow_to_stream(cur->str);
    }

    flow_to_stream(" -o ");
    flow_to_stream(output_file);
    flow_to_stream(".i\n");

    system(get_stream());
    flush_stream();
    
    // mcc1 command
    flow_to_stream(cur_dir);
    flow_to_stream("mcc1 -c ");
    
    flow_to_stream(output_file);
    flow_to_stream(".i");

    flow_to_stream(" -o ");
    flow_to_stream(output_file);
    flow_to_stream("\n");

    //printf("%s", get_stream());
    system(get_stream());
    flush_stream();

    return 0;
}

static void init_stream(size_t sz){
    stream_size = sz;
    stream_buf = calloc(stream_size, sizeof(char));
}

static void flush_stream(){
    memset(stream_buf, 0, stream_size);
}

static void flow_to_stream(char* data){
    if(strlen(stream_buf) + strlen(data) + 1 > stream_size){
        stream_buf = realloc(stream_buf, stream_size * 2);

    }
    strcat(stream_buf, data);
}

static char* get_stream(){
    return stream_buf;
}

char* get_file_directory(char* filename, char* directory){
    int len = strrchr(filename, '/') - filename + 1;
    strncpy(directory, filename, len);
}