#define ARRAY_SIZE 100000000
int a[ARRAY_SIZE];

int main()
{
  for (int i = 0; i < ARRAY_SIZE; i+=50000) {

    for (int j = i; j < ARRAY_SIZE && j < i + 20000; j++){
        a[j] = 30;
    }
  }
  return 0;
}
