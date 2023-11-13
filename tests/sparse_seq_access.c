#define ARRAY_SIZE 1000000
int a[ARRAY_SIZE];

int main()
{
  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (i % 10000){
        for (int j = i; j < ARRAY_SIZE && j < i + 3000; j++){
            a[j] = 30;
        }
    }
  }
  printf("Done\n");
  return 0;
}
