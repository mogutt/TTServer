package com.mogujie.ares.manager;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Iterator;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.storage.DBPool;

/**
 * 
 * @Description: db连接池管理类，单例
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-21 下午7:04:05
 * 
 */
public class DBManager {
    private static final Logger logger = LoggerFactory
            .getLogger(DBManager.class);

    // 连接池集合
    private ConcurrentHashMap<String, DBPool> dbPoolMap = new ConcurrentHashMap<String, DBPool>();

    // db配置
    private Properties dbProperties;

    // 是否已加载
    private boolean isLaunch = false;

    // 单例
    private static DBManager dbManagerInstance = getInstance();

    public static DBManager getInstance() {
        if (dbManagerInstance == null) {
            try {
                dbManagerInstance = new DBManager();
            } catch (SQLException e) {
                logger.error("", e);
            }
        }
        return dbManagerInstance;
    }

    private DBManager() throws SQLException {
        dbProperties = ConfigureManager.getInstance().getDBConfig();
        launch();
    }

    /**
     * 
     * @throws SQLException
     * @Description: 初始化加载
     */
    private void launch() throws SQLException {
        if (!isLaunch) {
            // shutDown();

            String shutDownInstances = dbProperties.getProperty("instances");
            String[] instances = shutDownInstances.split(",");
            for (String instanceName : instances) {
                if (instanceName.length() == 0) {
                    continue;
                }
                String jdbcUrl = dbProperties
                        .getProperty(instanceName + "_url");
                String jdbcUsername = dbProperties.getProperty(instanceName
                        + "_username");
                String jdbcPassword = dbProperties.getProperty(instanceName
                        + "_password");

                //logger.info("jdbc: " + jdbcUrl + " ,jdbcUsername: " + jdbcUsername + " ,jdbcPassword: " + jdbcPassword);

                DBPool newDBPool = new DBPool(jdbcUrl, jdbcUsername,
                        jdbcPassword);
                newDBPool.launchDBPool();

                dbPoolMap.put(instanceName, newDBPool);
            }
            isLaunch = true;
        }

    }

    /**
     * 
     * @Description: 关闭所有连接池
     */
    public void shutDown() {
        if (dbPoolMap.size() > 0) {
            Iterator<String> iterator = dbPoolMap.keySet().iterator();
            while (iterator.hasNext()) {
                DBPool pool = dbPoolMap.remove(iterator.next());
                if (pool != null) {
                    pool.shutDownDBPool();
                }
            }
            dbPoolMap.clear();
        }
    }

    /**
     * 
     * @Description: 获得DB连接
     * @return 指定的链接池中的一个连接
     */
    public Connection getConnection(DBPoolName name) {
        Connection connection = null;
        DBPool dbPool = dbPoolMap.get(name.toString());
        if (dbPool != null) {
            connection = dbPool.getConnection();
        }
        return connection;
    }

    /**
     * 
     * @Description: 释放连接
     * @param poolName
     *            连接所在的连接池
     * @param connection
     *            连接
     */
    public void closeConnection(DBPoolName poolName, Connection connection) {
        DBPool pool = dbPoolMap.get(poolName.toString());
        if (pool != null) {
            pool.shortCutClose(connection, null, null);
        }
    }

    /**
     * 
     * @Description: 集中释放各种资源
     * @param poolName
     *            连接所在的连接池的名字
     * @param connection
     *            连接
     * @param statement
     * @param resultSet
     */
    public void release(DBPoolName poolName, Connection connection,
            Statement statement, ResultSet resultSet) {
        DBPool pool = dbPoolMap.get(poolName.toString());
        if (pool != null) {
            pool.shortCutClose(connection, statement, resultSet);
        }
    }

    /**
     * 
     * @Description: 连接池的名字，定义一个枚举，以免传错
     * @author ziye - ziye[at]mogujie.com
     * @date 2013-7-22 上午11:45:27
     * 
     */
    public enum DBPoolName {
        macim_master, macim_slave
    }
}
