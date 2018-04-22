#ifndef USER_OPEN_FILE_H
#define USER_OPEN_FILE_H
#include <copyright.h>

class UserOpenFile {
 public:
  UserOpenFile(char * name, int offset, int id);
  ~UserOpenFile();
  char * getName();
  int getCurOffset();
  int getFileId();
  void setOffset(int i);
  void setFileId(int i);
  
 private:
  char * fileName;
  int offset;
  int fileId;
};
#endif USER_OPEN_FILE_H
