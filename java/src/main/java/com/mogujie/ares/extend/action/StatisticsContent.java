package com.mogujie.ares.extend.action;

import java.sql.SQLException;

import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.model.StatisticsModel;

public class StatisticsContent extends BaseAction{
	@SuppressWarnings("unused")
	private static final Logger logger = LoggerFactory.getLogger(StatisticsContent.class);
	
	//userId
	public void saveLog(int soure,int protocol,String ip,int userId,int actionType,String os,String userAgent,String flashVersion,String clientVersion,int version){
		/*
		 * C++ =>Java
		 *    source: web1.
   			  protocol: 2 
       		  ip: 3707387924
   			  userId: 1000000
       		  os: Mac ox x
			  userAgent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.76 Safari/537.36
    		  flash: 9.0.1 flashplayer 版本号
   		   	client: 3.14 as或客户端息的版本号.
		 */

	/*	logger.info("saveLog soure=" + soure
				 + ", protocol=" + protocol + ", ip=" + ip
				+ ", userId=" + userId+", actionType=" + actionType+ ", os=" + os+ ", userAgent=" + userAgent+ ", flashVersion=" + flashVersion+ ", clientVersion=" + clientVersion);
				*/
		try {
			StatisticsModel.getInstance().saveLog(soure, protocol, ip,actionType, userId, os, userAgent, flashVersion, clientVersion);
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}

}
