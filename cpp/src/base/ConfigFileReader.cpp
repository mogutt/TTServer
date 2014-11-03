/*
 * ConfigFileReader.cpp
 *
 *  Created on: 2013-7-2
 *      Author: ziteng@mogujie.com
 */


#include "ConfigFileReader.h"

CConfigFileReader::CConfigFileReader(const char* filename)
{
	m_config_map = new map<string, string>;
	_LoadFile(filename);
}

CConfigFileReader::~CConfigFileReader()
{
	delete m_config_map;
}

char* CConfigFileReader::GetConfigName(const char* name)
{
	if (!m_load_ok)
		return NULL;

	char* value = NULL;
	map<string, string>::iterator it = m_config_map->find(name);

	if(it != m_config_map->end())
	{
		value = (char*)it->second.c_str();
	}

	return value;
}

void CConfigFileReader::_LoadFile(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		log("can not open %s\n", filename);
		return;
	}

	char buf[256];
	for (;;)
	{
		char* p = fgets(buf, 256, fp);
		if (!p)
			break;

		size_t len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = 0;			// remove \n at the end

		char* ch = strchr(buf, '#');	// remove string start with #
		if (ch)
			*ch = 0;

		if (strlen(buf) == 0)
			continue;

		_ParseLine(buf);
	}

	fclose(fp);
	m_load_ok = true;
}

void CConfigFileReader::_ParseLine(char* line)
{
	char* p = strchr(line, '=');
	if (p == NULL)
		return;

	*p = 0;
	char* key =  _TrimSpace(line);
	char* value = _TrimSpace(p + 1);
	if (key && value)
	{
		m_config_map->insert(make_pair(key, value));
	}
}

char* CConfigFileReader::_TrimSpace(char* name)
{
	// remove starting space or tab
	char* start_pos = name;
	while ( (*start_pos == ' ') || (*start_pos == '\t') )
	{
		start_pos++;
	}

	if (strlen(start_pos) == 0)
		return NULL;

	// remove ending space or tab
	char* end_pos = name + strlen(name) - 1;
	while ( (*end_pos == ' ') || (*end_pos == '\t') )
	{
		*end_pos = 0;
		end_pos--;
	}

	int len = (int)(end_pos - start_pos) + 1;
	if (len <= 0)
		return NULL;

	return start_pos;
}
