package com.mogujie.ares.configure;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

import org.dom4j.DocumentException;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;

import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.FileManager;

/**
 * 配置，单利
 * @author stone
 *
 */
public class Configure
{
	public static final Logger logger = LoggerFactory.getLogger(Configure.class);
	
	public Properties systemConfig;
	
	public Properties dbConfig;
	
	public Properties cacheConfig;
	
	public Properties commonConfig;
	
	public ConcurrentHashMap<String, HashMap<String, String>> timerConfig;
	
	public Router actionRouter;
	
	public Configure()
	{
		systemConfig = new Properties();
		dbConfig = new Properties();
		cacheConfig = new Properties();
		commonConfig = new Properties();
		timerConfig = new ConcurrentHashMap<String, HashMap<String,String>>();
		actionRouter = Router.getInstance();
	}
	
	public Properties getSystemConfig()
	{
		return systemConfig;
	}
	
	public Properties getDBConfig()
	{
		return dbConfig;
	}
	
	public Properties getCacheConfig()
	{
		return cacheConfig;
	}
	
	public Router getActionRouter() {
		return actionRouter;
	}

	public ConcurrentHashMap<String, HashMap<String, String>> getTimerConfig()
	{
		return timerConfig;
	}
	
	/**
	 * 载入所有配置
	 * @throws Exception
	 */
	public void loadConfigs() throws Exception {
		loadSystemConfig(); // 系统配置
		setCommonConfig();
		loadDBConfig(); 
		loadCacheConfig(); 
		loadActionConfig(); // action相关的路由配置
		loadTimerConfig(); // 计划任务脚本配置
	}
	
	public void loadSystemConfig()
	{
		propertyConfigLoader_sys("/system.properties", systemConfig);
	}
	
	public void setCommonConfig() {
		String path = System.getProperty("user.dir") + "/" + systemConfig.getProperty("com.mogujie.ares.config.file.common");
		propertyConfigLoader(path, commonConfig);
		FileManager.fileServerUrl = commonConfig.getProperty("com.mogujie.ares.config.file.serverurl");
	}
	
	public void loadDBConfig()
	{
		String path = System.getProperty("user.dir") + "/" + systemConfig.getProperty("com.mogujie.ares.config.file.db");
		logger.info(path);
		propertyConfigLoader(path, dbConfig);
		//propertyConfigLoader("/" + systemConfig.getProperty("com.mogujie.ares.config.file.db"), dbConfig);
	}
	
	public void loadCacheConfig()
	{
		String path = System.getProperty("user.dir") + "/" + systemConfig.getProperty("com.mogujie.ares.config.file.cache");
		logger.info(path);
		propertyConfigLoader(path, cacheConfig);
		//propertyConfigLoader(System.getProperty("user.dir") + "/" + systemConfig.getProperty("com.mogujie.ares.config.file.cache"), cacheConfig);
	}
	
	public void loadActionConfig() throws Exception {
		actionRouter.load(systemConfig.getProperty("com.mogujie.ares.config.file.route"));
	}
	
	public void loadTimerConfig()
	{
		taskXMLConfigLoader(systemConfig.getProperty("com.mogujie.ares.config.file.timer"), timerConfig);
	}

	
	public void propertyConfigLoader(String configFilePath, Properties config)
	{
		if(configFilePath.length() == 0)
		{
			return ;
		}

		File file = new File(configFilePath);
		if (file.exists()) {
			try {
				InputStream configFileStream = new FileInputStream(file);
				config.load(configFileStream);
			} catch (IOException e) {
				logger.error("", e);
			}
		}
		
		
//		InputStream configFileStream = Configure.class.getClassLoader()
//				.getResourceAsStream(configFilePath);
		//InputStream configFileStream = this.getClass().getResourceAsStream(configFilePath);
		//try {
		//	logger.info("load " + configFilePath);
		//	config.load(configFileStream);
		//} catch (IOException e) {
		//	logger.error("", e);
		//}
	}
	
	public void propertyConfigLoader_sys(String configFilePath, Properties config)
	{
		InputStream configFileStream = this.getClass().getResourceAsStream(configFilePath);
		try {
			logger.info("load " + configFilePath);
			config.load(configFileStream);
		} catch (IOException e) {
			logger.error("", e);
		}
	}

	/**
	 * 载入xml格式的计划任务脚本的配置
	 * 
	 * @param configFilePath
	 * @param config
	 */
	@SuppressWarnings({"unchecked"})
	public void taskXMLConfigLoader(String configFilePath, Map<String, HashMap<String, String>> config)
	{
		if(configFilePath.length() == 0)
		{
			return ;
		}
//		String configFileFullPath = System.getProperty("user.dir") + 
//				"/src/main/resources/" + configFilePath;
//		if( ! new File(configFileFullPath).exists())
//		{
//			return ;
//		}
		
		InputStream is = this.getClass().getResourceAsStream("/" + configFilePath);
		if(is == null) {
			return;
		}
		SAXReader reader = new SAXReader();
		try {
			org.dom4j.Document configDocument = reader.read(is);
			List<Node> nodes = configDocument.selectNodes("//tasks/task");
			for(Node node : nodes)
			{
				Node enable = node.selectSingleNode("enable");
				Node name = node.selectSingleNode("name");
				Node initdelay = node.selectSingleNode("initdelay");
				Node periodordelay = node.selectSingleNode("periodordelay");
				Node taskType = node.selectSingleNode("taskType");
				
				HashMap<String, String> theTaskConfig = new HashMap<String, String>();
				theTaskConfig.put("enable", enable.getText());
				theTaskConfig.put("initdelay", initdelay.getText());
				theTaskConfig.put("periodordelay", periodordelay.getText());
				theTaskConfig.put("taskType", taskType.getText());
				
				config.put(name.getText(), theTaskConfig);
			}
		} catch (DocumentException e) {
			logger.error("", e);
		}
	}
	
	
}
