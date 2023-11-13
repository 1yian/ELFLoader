#define ARRAY_SIZE 100000000
int a[ARRAY_SIZE];

int main()
{
  for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = 30;
  }
  return 0;
}
