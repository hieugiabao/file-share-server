
#ifndef _MODEL_H_
#define _MODEL_H_

enum FType
{
  FDIRECTORY, // 0
  FFILE       // 1
};

enum Permission
{
  READ = 1,   // 0001
  WRITE = 2,  // 0010
  EXECUTE = 4 // 0100
};

#endif // _MODEL_H_