
int main()
{
    int i = 0, j = 0;
    int sum = 0;
    int n = 50;
    while(i < 100){
        while(j < n)
        {
            j = j + 1;
            n = n + 4;
            sum = sum + i + j;
        }
        i = i + 1;
    }
    return sum;
}