/*
 * Multimedia Small File Storage System 
 * FileManager Singleton to manager store or download file
 * author potian@mogujie.com
*/

#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include <map>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <errno.h>
#include <time.h>
#include "FileLin.h"
#include "CriticalSection.h"

using namespace std;

namespace msfs {
class CriticalSection;
class FileManager {
private:
	FileManager() {}
	FileManager(const char *host, const char *disk,
				int totFiles, int filesPerDir) {
        m_host = new char[strlen(host) + 1];
        m_disk = new char[strlen(disk) + 1];
		m_host[strlen(host)] = '\0';
		m_disk[strlen(disk)] = '\0';
		strncpy(m_host, host, strlen(host));
		strncpy(m_disk, disk, strlen(disk));
		m_totFiles = totFiles;
		m_filesPerDir = filesPerDir;
		m_map.clear();
	}
	~FileManager() {
		if (m_host)
			delete [] m_host;
		m_host = NULL;
		if (m_disk)
			delete [] m_disk;
		m_disk = NULL;
		EntryMap::iterator it = m_map.begin();
		while (it != m_map.end()) {
			delete it->second;
			m_map.erase(it++);
		}
	}

	FileManager(const FileManager &);
	FileManager operator=(const FileManager &);

public:
    static FileManager* getInstance(const char *host, const char *disk, int totFiles, int filesPerDir) {
			return (m_instance) ? m_instance : \
					(new FileManager(host, disk, totFiles, filesPerDir));
		}

	static void destroyInstance() {
		if (m_instance)
			delete m_instance;
		m_instance = NULL;
	}

	int initDir();
	u64 getFileCntCurr() {return m_totFiles;}
	int getFirstDir() {return (m_totFiles / (m_filesPerDir)) / (FIRST_DIR_MAX);}
	int getSecondDir() {return (m_totFiles % (m_filesPerDir * FIRST_DIR_MAX) ) / m_filesPerDir;}
	
	string createFileRelatePath();
	int uploadFile(const char *type, const void *content, u32 size, char *url, char *ext = NULL);
	int downloadFileByUrl(char *urlEn, void *buf, u32 *size);
	int getRelatePathByUrl(const string &url, string &path);
	int getAbsPathByUrl(const string &url, string &path);
	
protected:
	struct Entry {
		time_t m_lastAccess;
		size_t m_fileSize;
		u8* m_fileContent;
		Entry() {
			m_lastAccess = 0;
			m_fileSize = 0;
			m_fileContent = NULL;
		}
		~Entry() {
			if (m_fileContent)
				delete [] m_fileContent;
			m_fileContent = NULL;
		}
	};
	typedef std::map<std::string, Entry*> EntryMap;
	int insertEntry(const std::string& url, size_t filesize, 
					const void* content);
	Entry* getEntry(const std::string& url) const {
		return const_cast<FileManager*>(this)->getOrCreateEntry(url,false);
	}
	Entry* getOrCreateEntry(const std::string& url, bool create);
	void releaseFileCache(const std::string& url);
	void updateMapCache();

private:
	char			*m_host;	//msfs server ip or hostname
	char			*m_disk;	//storage directory of media files
	u64 			m_totFiles;	//total files has storaged
	CriticalSection	m_filesCs;
	
	int 			m_filesPerDir;	//mas file nums per dir eg. xxx/xxx
    static const u32 MAX_FILE_SIZE_PER_FILE = 5 * 1024 * 1024; 
	static const int FIRST_DIR_MAX =255;
	static const int SECOND_DIR_MAX =255;
	static FileManager * m_instance;
	EntryMap m_map;
	static const u64 MAX_FILE_IN_MAP = 10000;
	CriticalSection	m_cs;
};

}

#endif

