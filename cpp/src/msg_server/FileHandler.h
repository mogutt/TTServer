/*
 * FileHandler.h
 *
 *  Created on: 2013-12-17
 *      Author: ziteng@mogujie.com
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include "impdu.h"

class CMsgConn;

class CFileHandler
{
public:
	virtual ~CFileHandler() {}

	static CFileHandler* getInstance();

	void HandleClientFileRequest(CMsgConn* pMsgConn, CImPduClientFileRequest* pPdu);
	void HandleClientFileHasOfflineReq(CMsgConn* pMsgConn, CImPduClientFileHasOfflineReq* pPdu);
	void HandleClientFileAddOfflineReq(CImPduClientFileAddOfflineReq* pPdu);
	void HandleClientFileDelOfflineReq(CImPduClientFileDelOfflineReq* pPdu);
	void HandleFileHasOfflineRes(CImPduFileHasOfflineRes* pPdu);
    void HandleFileNotify(CImPduFileNotify* pPdu);
private:
	CFileHandler() {}

private:
	static CFileHandler* s_handler_instance;
};


#endif /* FILEHANDLER_H_ */
