package com.mogujie.ares.lib.storage;

import com.mogujie.ares.configure.SysConstants;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;

/**
 * 
* @Description: 缓存的连接池
* @author shitou - shitou[at]mogujie.com
* @date 2013-7-21 下午4:15:34
*
 */
public class CachePool {
	
	private JedisPool pool = null;
	
	private JedisPoolConfig poolConfig = null;
	
	private String host; // 缓存的服务器
	private int port; // 端口
	private int db; // 连接到第几个db
	
	public CachePool(String host, Integer port, Integer db)
	{
		this.host = host;
		this.port = port;
		this.db = db;
	}
	
	/**
	 * @Description: 初始化配置
	 */
	public void initialConfig()
	{
		if(poolConfig == null)
		{
			poolConfig = new JedisPoolConfig(); 
			poolConfig.setTestOnBorrow(SysConstants.TEST_ON_BORROW); // 从数据库连接池中取得连接时，对其的有效性进行检查
			poolConfig.setMaxActive(SysConstants.MAX_ACTIVE); // 最大连接数
			poolConfig.setMaxIdle(SysConstants.MAX_IDLE); // 最大闲置的连接数
			poolConfig.setMinIdle(SysConstants.MIN_IDLE); // 最少
			poolConfig.setMaxWait(SysConstants.MAX_WAIT);  // 请求最长等待时间/毫秒
			poolConfig.setTestWhileIdle(SysConstants.TEST_WHILE_IDLE); // 闲置时测试
		}
	}
	
	/**
	 * 
	 * @Description: 初始化
	 */
	public void launch()
	{
		if(pool == null)
		{
			initialConfig();
			pool = new JedisPool(poolConfig, this.host, this.port);
		}
	}
	
	/**
	 * 
	 * @Description: 销毁
	 */
	public void destory()
	{
		if(pool != null)
		{
			pool.destroy();
		}
	}
	
	/**
	 * 
	 * @Description: 获得连接
	 * @return redis的连接
	 */
	public Jedis getResource()
	{
		Jedis jedisInstance = null;
		if(pool != null)
		{
			jedisInstance = pool.getResource();
			if(db > 0)
			{
				jedisInstance.select(db);
			}
		}
		return jedisInstance;
	}
	
	/**
	 * 
	 * @Description: 释放连接
	 * @param jedisInstance
	 */
	public void returnResource(Jedis jedisInstance)
	{
		pool.returnResource(jedisInstance);
	}

}
