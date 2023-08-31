
int main()
{
    int i = 0, j = 0;
    int sum = 0;
    int n = 50;
    int a[100];
    while(i < 100){
        while(j < 50)
        {
            j = j + 1;
            n = n + 4;
            a[j] = n;
            sum = sum + i + j;
        }
        i = i + 1;
    }
    return sum;
}