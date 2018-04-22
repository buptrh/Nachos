#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define LIMIT        99

int main ()
{
  int a = 0;
  int b = 0;
  int c = 0;
  
  int i;
  for (i = 0 ; i < LIMIT ; i++) {
    c = a + b;
    printf("The value of A is %d the value of B is %d.\nThe sum of both is %d",a,b,c);	  
  }

  return EXIT_SUCCESS;
}
