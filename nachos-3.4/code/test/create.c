void procs()
{
  Exec("../test/open");
}

int main()
{
  int k = -1;
  int j = -1;
  int p = -1;
  int f = -1;
  int lim = 20;
  int pid = -1;
  
  Create("newfile.txt");
  Create("bobsuncle.txt");

  j = Open("newfile.txt");
  k = Open("bobsuncle.txt");

  //char buf1[4] = {'t','e','s','t'};
  Write("Test: Print to stdout", 21, 1);


  
  //char buf2[4] = {'f','r','e','d'};
  Write("fred & bob!", 11, k);

  
  Read("", 4, 0);
  Read("", 10, k);

  Close(j);
  Close(k);

  Create("dummy.txt");
  Create("after_close.txt");
  f = Open ("dummy.txt");
  p = Open("after_close.txt");
  Write("HelloWorld!", 11, p);

  Read("", 5, p);
  Read("", 6, p);

  
  Write("Goodbye World", 13, p);
  Read("", 13, p);
  while (lim > 0) {
    lim--;
  }
  pid = Fork(procs);
  Yield();
  
  Close(p);
  Close(f);
  
  Exit(p); 
}
