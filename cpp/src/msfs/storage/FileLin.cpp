/*
 *  Multimedia Small File Storage System
 *  File Operation API implement in Linux OS
 *  @author potian@mogujie.com
*/
#ifndef WIN32

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#include "FileLin.h"

namespace msfs {

File::File(const char *path) {
	assert(path);
	m_path = new char[strlen(path) + 1];
	strncpy(m_path, path, strlen(path));
	m_path[strlen(path)] = '\0';
	m_opened = false;
	m_file = -1;
	m_size = -1;
}

File::~File() {
	if (m_opened) 
		close();
	delete [] m_path;
	m_path = NULL;
}

u64 File::isExist(bool *exist) {
	*exist = access(m_path, 0) == 0;
	return 0;
}

u64 File::remove() {
	if (-1 == unlink(m_path))
		return errno;
	return 0;
}

u64 File::create(bool directIo) {
	assert(!m_opened);
	int flags = O_RDWR | O_CREAT | O_EXCL;
#if defined(__FREEBSD__) || defined (__APPLE__)
	m_file = ::open(m_path, flags, 00640);
#elif defined(__LINUX__) || defined(__linux__)
	m_file = open64(m_path, flags, 00640);
#endif	
	if (-1 == m_file)
		return errno;
#ifdef __LINUX__	
	if (directIo)
		if (-1 == fcntl(m_file, F_SETFL,O_DIRECT))
			return errno;
#endif	
	m_opened = true;
	m_size = 0;
	m_directIo = directIo;
	return 0;
}
	
u64 File::open(bool directIo) {
	assert(!m_opened);
	int flags = O_RDWR;
#ifdef __linux__	
	m_file = open64(m_path, flags);
#elif defined(__FREEBSD__) || defined(__APPLE__)
	m_file = ::open(m_path, flags);
#endif	
	if(-1 == m_file) {
		return errno;
	}
#ifdef __LINUX__
	if (directIo)
		if (-1 == fcntl(m_file, F_SETFL, O_DIRECT))
			return errno; 
#endif	
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	if(fcntl(m_file, F_SETLK, &lock) < 0) {
		::close(m_file);
		return errno;
	}

	m_opened = true;
	u64 size = 0;
	u64 code = getSize(&size);
	if (code) {
		close();
		return code;
	}
	m_size = size;
	m_directIo = directIo;
	return 0;
}	

u64 File::close() {
	if (!m_opened)
		return 0;
	u64 err = sync();
	if (err)
		return err;
	m_opened = false;
	if(0 != ::close(m_file))
		return errno;
	return 0;		
}

s64 File::getFileSize(const char *path) {
    s64 filesize = -1;
    struct stat statbuff;
	if(stat(path, &statbuff) < 0){	
		return filesize;
	} else {
		filesize = statbuff.st_size;
	}
	return filesize;
}

u64 File::getSize(u64 *size) {
	if (m_size > 0) {
		*size = m_size;
		return 0;
	}

	s64 sksize = lseek(m_file, 0, SEEK_END);
	if (-1 == sksize)
		return errno;
	else
		*size = (u64)sksize;
	m_size = *size;
	return 0;
}

u64 File::setSize(u64 size) {
	assert(m_opened);
#ifdef __linux__
	if (-1 == ftruncate64(m_file, size))
#elif defined(__FREEBSD__) || defined (__APPLE__)
	if (-1 == ftruncate(m_file, size))
#endif		
		return errno;
	m_size = size;
	return 0;
}

u64 File::read(u64 offset, u32 size, void *buffer) {
	assert(m_opened);
	if (offset + size > (u64)m_size)
		return E_EOF;
#ifdef __linux__	
	if (size != pread64(m_file, buffer, size, offset))
#elif defined(__FREEBSD__) || defined(__APPLE__)
	if (size != pread(m_file, buffer, size, offset))
#endif 		
		return errno;
	return 0;
}

u64 File::write(u64 offset, u32 size, const void *buffer) {
	assert(m_opened);
	setSize((u64)size);
	if (offset + size > (u64)m_size)
		return E_EOF;
#ifdef __linux__
	if(size != pwrite64(m_file, buffer, size, offset))
#elif defined(__FREEBSD__) || defined (__APPLE__)
	if(size != pwrite(m_file, buffer, size, offset))		
#endif
		return errno;
	return 0;
}

u64 File::sync() {
	assert(m_opened);
	if (-1 == fsync(m_file)) {
		return errno;
	}
	return 0;
}

u64 File::mkdirNoRecursion(const char *path) {
	if (::mkdir(path, 0777) != 0) 
		return errno;
	return 0;
}
	
u64 File::isDirectory(bool *isDir) {
	struct stat fileStat;
	if (stat(m_path, &fileStat) != 0)
		return errno;
	*isDir = S_ISDIR(fileStat.st_mode) != 0;
	return 0;
}
	
u64 File::getFileNum(int *fileNum) {
	DIR *dp;
	struct dirent *ep = NULL;
	string dir(m_path);
	if (dir[dir.length() - 1] != '/' && dir[dir.length() - 1] != '\\')
	dir += "/";
	dp = opendir(m_path);
	if (!dp)
		return errno;
	int files = 0;
	ep = readdir(dp);
	while (ep) {
		string subPath = dir + ep->d_name;
		File subFile(subPath.c_str());
		bool isDir = false;
		if (subFile.isDirectory(&isDir) != 0 || isDir) {
			closedir(dp);
			return errno;
		} else {
			if (strncmp(ep->d_name, ".", 1) != 0)
				files++;
		}
		ep = readdir(dp);
	}
	*fileNum = files;
	closedir(dp);
	return 0;
}

}
#endif

