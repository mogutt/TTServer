package com.mogujie.ares.manager;

import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

import com.mogujie.ares.lib.storage.CachePool;

import redis.clients.jedis.Jedis;

/**
 * 
* @Description: 缓存连接相关的管理类，针对不同业务会有多个连接池,单例
* @author shitou - shitou[at]mogujie.com
* @date 2013-7-21 下午5:45:18
*
 */
public class CacheManager
{
	private ConcurrentHashMap<String, CachePool> cachePool = new ConcurrentHashMap<String, CachePool>(); // 多个池子
	
	private static CacheManager cacheManagerInstance = new CacheManager();
	
	private Properties cacheProperties;
	
	private boolean isLanuch = false;
	
	public static CacheManager getInstance()
	{
		if(cacheManagerInstance == null)
		{
			cacheManagerInstance = new CacheManager();
		}
		return cacheManagerInstance;
	}
	
	private CacheManager()
	{
		this.cacheProperties = ConfigureManager.getInstance().getCacheConfig();
		launch();
	}
	
	/**
	 * @Description: 初始化
	 */
	private void launch()
	{
		if( ! isLanuch)
		{
			//shutDown();
			
			String needLaunchPoolInstance = cacheProperties.getProperty("instances");
			String[] needLaunchPools = needLaunchPoolInstance.split(",");
			for(String instance : needLaunchPools)
			{
				//获取配置
				String redisHost = cacheProperties.getProperty(instance + "_host");
				Integer redisPort = Integer.valueOf(cacheProperties.getProperty(instance + "_port"));
				Integer redisDB = Integer.valueOf(cacheProperties.getProperty(instance + "_db"));
				
				//实例化，启动，并塞入Hashmap中
				CachePool cachePoolInstance = new CachePool(redisHost, redisPort, redisDB);
				cachePoolInstance.launch();
				cachePool.put(instance, cachePoolInstance);
			}
			isLanuch = true;
		}
	}
	
	/**
	 * @Description: 关闭所有连接池
	 */
	public void shutDown()
	{
		if(this.cachePool.size() > 0)
		{
			Iterator<Entry<String, CachePool>> iterator = this.cachePool.entrySet().iterator();
			while(iterator.hasNext())
			{
				Map.Entry<String, CachePool> hashEntry = iterator.next();
				CachePool poolInstance = hashEntry.getValue();
				poolInstance.destory();
				
				cachePool.remove(hashEntry.getKey());
			}
			
			cachePool.clear();
		}
	}
	
	/**
	 * @Description: 从指定的链接池中获取一个链接
	 * @param poolName 连接池的名字
	 * @return
	 */
	public Jedis getResource(CachePoolName poolName) {
		String strName = poolName.toString();
		
		return getJedisResource(strName);
	}
	
	/**
	 * @Description: 释放一个链接
	 * @param poolName 连接所在的连接池
	 * @param jedis 连接
	 */
	public void returnResource(CachePoolName poolName,  Jedis jedis) {
		if(jedis == null) {
			return ;
		}
		
		String strName = poolName.toString();
		returnJedisResource(strName, jedis);
	}
	
	/**
	 * 
	 * @Description: 具体释放连接操作
	 * @param poolName
	 * @param jedisInstance
	 */
	private void returnJedisResource(String poolName, Jedis jedisInstance) {
		CachePool pool = cachePool.get(poolName);
		if(pool != null) {
			pool.returnResource(jedisInstance);
		}
	}
	
	/**
	 * 
	 * @Description: 具体获取连接操作
	 * @param instanceName
	 * @return
	 */
	private Jedis getJedisResource(String instanceName) {
		Jedis jedisResource = null;
		CachePool pool = cachePool.get(instanceName);
		if(pool != null) {
			jedisResource = pool.getResource();
		}
		return jedisResource;
	}
	
	/**
	 * 
	 * @Description: 连接池的名字
	 * @author ziye - ziye[at]mogujie.com
	 * @date 2013-7-21 下午6:37:06
	 *
	 */
	public enum CachePoolName {
		counter, // 所有计数器
		unread, // 未读计数器
//		cinfo,	//用户信息(消息通知用的).
//		cinfo_push, // 推送cinfo
		group_counter
	}
}
