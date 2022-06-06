int main(){

    int a = 12;
    switch(a){
        case 1:
        case 2:
            a = 13;
            goto label;
        case 12:
            a = 15;
    }
    label:

    return a;
}