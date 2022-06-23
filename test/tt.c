#define ABC 1
#ifdef ABC
static int a;
#endif
int main(){

    enum ABC_ENUM{
        ENUM1 = 0,
        ENUM2,
        ENUM3 = 5, 
        ENUM4
    };

    enum ABC_ENUM a = ENUM3, c;

    c = a;


    return a;
}