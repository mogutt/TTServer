package com.mogujie.ares.model;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;

import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;

/*
 * @Description: 统计模块处理.
 * @author zuoye@mogujie.com
 */
public class StatisticsModel {
    private static StatisticsModel instance = new StatisticsModel();

    public static StatisticsModel getInstance() {
        if (instance == null) {
            instance = new StatisticsModel();
        }
        return instance;
    }

    public void saveLog(int soure, int protocol, String ip, int type,
            int userId, String os, String userAgent, String flashVersion,
            String clientVersion) throws SQLException {
        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        try {
            String sql = " insert into IMLogging (source,protocol,ip,type,userId,os,userAgent,flash,client,created) values(?,?,?,?,?,?,?,?,?,?)";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, soure);
            statement.setObject(index++, protocol);
            statement.setObject(index++, ip2long(ip));
            statement.setObject(index++, type);
            statement.setObject(index++, userId);
            statement.setObject(index++, os);
            statement.setObject(index++, userAgent);
            statement.setObject(index++, flashVersion);
            statement.setObject(index++, clientVersion);
            statement.setObject(index++, System.currentTimeMillis() / 1000);
            statement.executeUpdate();
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, null);
        }
    }

    public long ip2long(String ip) {
        int ipNum = 0;
        try {
            ipNum = str2Ip(ip);
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }
        return int2long(ipNum);
    }

    public long int2long(int i) {
        long l = i & 0x7fffffffL;
        if (i < 0) {
            l |= 0x080000000L;
        }
        return l;
    }

    public int str2Ip(String ip) throws UnknownHostException {
        InetAddress address = InetAddress.getByName(ip);// 在给定主机名的情况下确定主机的 IP 址。
        byte[] bytes = address.getAddress();// 返回此 InetAddress 对象的原始 IP 地址
        int a, b, c, d;
        a = byte2int(bytes[0]);
        b = byte2int(bytes[1]);
        c = byte2int(bytes[2]);
        d = byte2int(bytes[3]);
        int result = (a << 24) | (b << 16) | (c << 8) | d;
        return result;
    }

    public int byte2int(byte b) {
        int l = b & 0x07f;
        if (b < 0) {
            l |= 0x80;
        }
        return l;
    }
}
