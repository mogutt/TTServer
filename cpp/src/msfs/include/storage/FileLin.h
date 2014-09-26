/*
 *  Multimdia Small File Storage Sytem
 *  File Operation In Linux System
 *  @author potian@mogujie.com
*/

#ifndef _FILELIN_H_
#define _FILELIN_H_

#include "Portable.h"
#include <string>
#include <assert.h>
#include <list>

using namespace std;

namespace msfs {

class File {
public:
	File(const char *path);
	~File();
	const char* getPath() const {
		return m_path;
	}
	u64 create(bool directIo = false);
	u64 remove();
	u64 open(bool directIo = false);
	u64 close();

	u64 isExist(bool *exist);
	static bool isExist(const char *path) {
		bool exist = false;
		File(path).isExist(&exist);
		return exist;
	}
	
	static u64 mkdirNoRecursion(const char *path);
	u64 isDirectory(bool *isDir);
	
	u64 getFileNum(int *fileNum);
	int getFd() { return m_opened == false ? -1 : m_file;}
	u64 read(u64 offset, u32 size, void *buffer);
	u64 write(u64 offset,u32 size, const void *buffer);
	u64 sync();
	
	static s64 getFileSize(const char *path);
	u64 getSize(u64 *size);
	u64 setSize(u64 size);

	static const char* explainErrno(u64 code) {
		switch ((u32)code) {
			case E_NO_ERROR:
				return "no error";
			case E_NOT_EXIST:
				return "file not exist";	
			case E_PERM_ERR:
				return "permission deny";
			case E_DISK_FULL:
				return "disk is full";
			case E_EXISTS:
				return "file already exist";
			case E_IN_USE:
				return "file is in use";
			case E_READ:
				return "read failed";
			case E_WRITE:
				return "write failed";
			case E_EOF:
				return "end of file exceeded";
			default:
				return "other reasons";
		}
	}

public:
	//error code
	static const u64 E_NO_ERROR = 0;
	static const u32 E_NOT_EXIST = 1;
	static const u64 E_PERM_ERR = 2;
	static const u64 E_DISK_FULL = 3;
	static const u64 E_EXISTS = 4;
	static const u64 E_IN_USE = 5;
	static const u64 E_EOF = 6;
	static const u64 E_READ = 7;
	static const u64 E_WRITE = 8;
	static const u64 E_NO_EMPTY = 9;
	static const u64 E_OTHER = 10;

private:
	char *m_path;	//file full path 
	bool m_opened;	//file has opened 
	s64 m_size;	//file size
	int m_file;	//file fd, -1 while the file no exists or opened
	bool m_directIo;//file open with directIO no OS buffer
};

}

#endif
