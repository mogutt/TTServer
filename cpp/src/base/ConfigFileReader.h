/*
 * ConfigFileReader.h
 *
 *  Created on: 2013-7-2
 *      Author: ziteng@mogujie.com
 */

#ifndef CONFIGFILEREADER_H_
#define CONFIGFILEREADER_H_

#include "util.h"

class CConfigFileReader
{
public:
	CConfigFileReader(const char* filename);
	~CConfigFileReader();

	char* GetConfigName(const char* name);
private:
	void _LoadFile(const char* filename);
	void _ParseLine(char* line);
	char* _TrimSpace(char* name);

	bool					m_load_ok;
	map<string, string>*	m_config_map;
};



#endif /* CONFIGFILEREADER_H_ */
