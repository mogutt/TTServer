package com.mogujie.ares;

import com.mogujie.ares.configure.SysConstants;
import com.mogujie.ares.extend.action.DelayUpdateMonitor;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.CacheManager;
import com.mogujie.ares.manager.ConfigureManager;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.ElegantStopManager;
import com.mogujie.ares.manager.NetworkManager;
import com.mogujie.ares.manager.TimerManager;

/**
 * 初始化阿瑞斯项目main类
 * @author ziye
 *
 */
public class MainServer {
	
	private static final Logger logger = LoggerFactory.getLogger(MainServer.class);
	
	public MainServer() {
		
	}
	
	public void bootup(int port) {
		try {
			initLog();
			initConfigure();
			initBase(); // 基础部分的启动，包括各种连接池
			initTimers();
			initNet(port);
			startStopChecker(port); 
		} catch (Exception e) {
			logger.error("", e);
			ElegantStopManager.getInstance(port).shutdown();
		} 
	}
	
	/**
	 * 日志初始化
	 */
	public void initLog() {
		// 这里好像没什么好做的...
	}
	
	/**
	 * 配置相关的初始化
	 * @throws Exception 
	 */
	public void initConfigure() throws Exception {
		ConfigureManager.getInstance().loadAllConfigs();
	}
	
	/**
	 * 包括lib内的连接池等的初始化
	 * @throws Exception 
	 */
	public void initBase() throws Exception {
		DBManager dbManager = DBManager.getInstance();
		CacheManager cacheManager = CacheManager.getInstance();
		if(dbManager == null || cacheManager == null) {
			throw new Exception("初始化db和cache连接池出错!");
		}
	}
	
	/**
	 * 网络初始化
	 * @throws Exception 
	 */
	public void initNet(int port) throws Exception {
		NetworkManager networkManager = NetworkManager.getInstance();
		networkManager.init(port);
	}
	
	/**
	 * 定时脚本启动
	 * @throws Exception 
	 */
	public void initTimers() throws Exception {
		TimerManager.getInstance().lanuch();
		DelayUpdateMonitor.getInstance().start();
	}
	
	/**
	 * 启动结束Check是否结束程序的线程
	 */
	public void startStopChecker(int port) {
		ElegantStopManager.getInstance(port).startCheckShutdownThread();
	}
	
    public static void main( String[] args ) {
    	int port = SysConstants.SERVER_DEFAULT_PORT;
    	if(args.length >= 1) {
    		port = Integer.parseInt(args[0]);
    	}
    	MainServer server = new MainServer();
    	server.bootup(port);
    }
    
}
