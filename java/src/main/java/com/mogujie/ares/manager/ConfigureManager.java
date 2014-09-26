package com.mogujie.ares.manager;

import java.util.HashMap;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

import com.mogujie.ares.configure.Configure;
import com.mogujie.ares.configure.Router;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
/**
 * 
 * @Description: 配置管理类, 单例
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 上午11:15:23
 *
 */
public class ConfigureManager
{

	public static final Logger logger = LoggerFactory.getLogger(ConfigureManager.class);

	private static ConfigureManager _configureManagerInstance = null;
	
	public static ConfigureManager getInstance()
	{
		if(_configureManagerInstance == null)
		{
			_configureManagerInstance = new ConfigureManager();
		}
		return _configureManagerInstance;
	}
	
	private Configure configure;
	
	private ConfigureManager()
	{
		configure = new Configure();
	}
	
	/**
	 * 初始化配置，只在启动时调用
	 */
	public void initial() throws Exception
	{
		reloadAllConfigs();
	}
	
	/**
	 * db的配置
	 * @return
	 */
	public Properties getDBConfig()
	{
		return configure.getDBConfig();
	}
	
	/**
	 * cache(redis)配置
	 * @return
	 */
	public Properties getCacheConfig()
	{
		return configure.getCacheConfig();
	}
	
	/**
	 * 阿瑞斯系统配置
	 * @return
	 */
	public Properties getSystemConfig()
	{
		return configure.getSystemConfig();
	}
	
	public Router getActionRouter() {
		return configure.getActionRouter();
	}
	
	/**
	 * 计划任务脚本配置
	 * @return
	 */
	public ConcurrentHashMap<String, HashMap<String, String>> getTimerConfig()
	{
		return configure.getTimerConfig();
	}
	
	/**
	 * 装载数据
	 * @throws Exception
	 */
	public void loadAllConfigs() throws Exception
	{
		configure.loadConfigs();
	}
	
	/**
	 * 重新装载数据
	 * @throws Exception
	 */
	public void reloadAllConfigs() throws Exception {
		loadAllConfigs();
	}

}
