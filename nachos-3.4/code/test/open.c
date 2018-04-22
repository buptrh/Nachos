int main()
{
  int i = -1;
  i = Open("after_close.txt");
  Read("", 5, i);
  Read("", 6, i);
  
  Close(i);
  Exit(i);
}
