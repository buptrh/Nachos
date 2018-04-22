void checkFiles()
{
  int k = -1;
  k = Open("triangle.txt");
  Write("Adios", 5, k);
  Read("", 5, k);
  Close(k);
  Exit(k);
}

int main ()
{
  int i = -1;
  int j = -1;

  Create("box.txt");
  Create("triangle.txt");

  i = Open("box.txt");
  j = Open("triangle.txt");

  Fork(checkFiles);
  Yield();
  
  Write("Hello", 5, i);

  Read("", 5, i);
  Read("", 5, j);

  Close(i);
  Close(j);

  Exit(j);
}
