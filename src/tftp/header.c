#include "tftp/header.h"

char *get_message(TFTPErrorCodes error_code)
{
  switch (error_code)
  {
    case UNKNOWN:
      return "Unknown error";
    case FILE_NOT_FOUND:
      return "File not found";
    case ACCESS_VIOLATION:
      return "Access violation";
    case DISK_FULL:
      return "Disk full or allocation exceeded";
    case ILLEGAL_OPERATION:
      return "Illegal TFTP operation";
    case UNKNOWN_TRANSFER_ID:
      return "Unknown transfer ID";
    case FILE_ALREADY_EXISTS:
      return "File already exists";
    case NO_SUCH_USER:
      return "No such user";
    case INVALID_OPTIONS:
      return "Invalid options specified";
    default:
      return "Unknown error";
  }
}