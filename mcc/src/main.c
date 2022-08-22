#include "mcc.h"

char* src_file = NULL;
char* output_file = NULL;
ARG_VEC* inc_vec = NULL;

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

    // mcpp command
    int len = 0;
    for(ARG_VEC* cur = inc_vec; cur; cur = cur->next){
        len += strlen(cur->str) + 1;
    }

    char* inc_cmd = calloc(len, sizeof(char));
    for(ARG_VEC* cur = inc_vec; cur; cur = cur->next){
        strcat(inc_cmd, cur->str);
        strcat(inc_cmd, " ");
    }

    printf("mcpp -c %s -i %s -o %s.i\n", src_file, inc_cmd, output_file);

    // mcc1 command
    printf("mcc1 -c %s.i -o %s\n", output_file, output_file);

    return 0;
}