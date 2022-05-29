int main(){

    struct TEST_STRUCT{
        int a;
        long b;
        char ab[4];
        struct {
            long ai;
            long k;
        } abdd;
    } abd, *c;
    struct TEST_STRUCT k;

    return sizeof(abd);
}