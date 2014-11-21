package com.mogujie.ares.lib.storage;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import com.alibaba.druid.pool.DruidDataSource;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;

/**
 * 
 * @Description: db连接池 
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-21 下午5:42:55
 *
 */
public class DBPool
{
	
	private static final Logger logger = LoggerFactory.getLogger(DBPool.class);
	
//	private BoneCP dbConnectionPool = null;
	
//	private BoneCPConfig dbBoneCPConfig = null;
	
	private DruidDataSource druid;
	
	private String jdbcUrl;
	
	private String jdbcUsername;
	
	private String jdbcPassword;
	
	private String driverClass = "com.mysql.jdbc.Driver";
	
	private int initConnectionCount = 5;
	
	private int maxActiveConnection = 18;
	
	
	public DBPool(String jdbcUrl, String username, String password)
	{
		this.jdbcUrl = jdbcUrl;
		this.jdbcUsername = username;
		this.jdbcPassword = password;
	}
	
	/**
	 * 初始化
	 */
//	public void initial()
//	{
//		//load configure
////		if(dbBoneCPConfig == null)
////		{
////			dbBoneCPConfig = new BoneCPConfig();
////			dbBoneCPConfig.setJdbcUrl(this.jdbcUrl);
////			dbBoneCPConfig.setUsername(this.jdbcUsername);
////			dbBoneCPConfig.setPassword(this.jdbcPassword);
////			dbBoneCPConfig.setMinConnectionsPerPartition(Constants.DB_MIN_CONNECTIONS_PER_PARTITION);
////			dbBoneCPConfig.setMaxConnectionsPerPartition(Constants.DB_MAX_CONNECTIONS_PER_PARTITION);
////			dbBoneCPConfig.setIdleConnectionTestPeriod(Constants.DB_IDLE_CONNECTION_TEST_PERIOD, TimeUnit.SECONDS); 
////			dbBoneCPConfig.setPartitionCount(Constants.DB_PARTITION_COUNT);
////		}
//	}
	
	/**
	 * 启动
	 * @throws SQLException 
	 */
	public void launchDBPool() throws SQLException
	{
		if(druid == null)
		{
//			initial();
			try {
//				dbConnectionPool = new BoneCP(dbBoneCPConfig);
				druid = new DruidDataSource();
				druid.setDriverClassName(driverClass);
				druid.setUsername(jdbcUsername);
				druid.setPassword(jdbcPassword);
				druid.setUrl(jdbcUrl);
				druid.setMinIdle(maxActiveConnection);
				druid.setInitialSize(initConnectionCount);
				druid.setMaxActive(maxActiveConnection);
				druid.setFilters("stat");
				druid.setTestWhileIdle(true);
				druid.setTestOnBorrow(false);
				druid.setTestOnReturn(false);
				druid.setMaxWait(60000);
				druid.setPoolPreparedStatements(false);
				druid.setValidationQuery("SELECT 'x'");
				
				druid.init();
			} catch (SQLException e) {
				logger.error("", e);
				throw e;
			}
		}
	}
	
	/**
	 * 关闭
	 */
	public void shutDownDBPool()
	{
		if(druid != null)
		{
			//dbConnectionPool.shutdown();
			druid.close();
		}
	}
	
	/**
	 * 从Pool中获取一个链接
	 */
	public Connection getConnection()
	{
		Connection connection = null;
		try {
//			connection = dbConnectionPool.getConnection();
			connection = druid.getConnection();
		} catch (SQLException e) {
			logger.error("", e);
		}
		return connection;
	}

	/**
	 * 快捷关闭
	 * @param connection
	 * @paramlogger.error("", e);
	 */
	public void shortCutClose(Connection connection, Statement stat, ResultSet rs)
	{
		try {
			if(rs != null && !rs.isClosed()) {
				rs.close();
			}
			if(stat != null && !stat.isClosed()) {
				stat.close();
			}
			if(connection != null && !connection.isClosed()) {
				connection.close();
			}
		} catch (SQLException e) {
			logger.error("", e);
		}
	}

}
