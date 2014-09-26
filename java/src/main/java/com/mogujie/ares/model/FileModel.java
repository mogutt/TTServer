package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.mogujie.ares.data.TransmitFile;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;

/**
 * @Description: 用户相关的model，包括获取用户信息等
 * @author ziye
 * 
 */
public class FileModel {

    private static final Logger logger = LoggerFactory
            .getLogger(FileModel.class);
    private static FileModel instance = new FileModel();

    public static FileModel getInstance() {
        if (instance == null) {
            instance = new FileModel();
        }
        return instance;
    }

    /*
     * 
     * @Description: 保存一个未接收的离线文件
     * 
     * @param userId
     * 
     * @param toUserId
     * 
     * @param filePath
     * 
     * @return
     */
    public boolean saveFileRecord(int userId, int toUserId, String taskId, String filePath,
            int fileSize) {
        if (userId <= 0 || toUserId <= 0 || filePath == null
                || "".equals(filePath)) {
            return false;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        try {
            int time = (int) (System.currentTimeMillis() / 1000);
            String sql = " insert into IMTransmitFile (userId, toUserId, taskId, filePath, created, updated,fsize) "
                    + "values(?,?,?,?,?,?,?)";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setInt(index++, userId);
            statement.setInt(index++, toUserId);
            statement.setString(index++, taskId);
            statement.setString(index++, filePath);
            statement.setInt(index++, time);
            statement.setInt(index++, time);
            statement.setInt(index++, fileSize);
            statement.executeUpdate();
        } catch (SQLException e) {
            logger.error(userId + " : " + toUserId + " : " + filePath, e);
            return false;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, null);
        }

        return true;
    }

    /*
     * 
     * @Description: 设置一个文件已经被对方接收
     * 
     * @param id
     * 
     * @return
     */
    public boolean deleteFileRecord(String taskId) {

        if (null == taskId || taskId.isEmpty()) {
            return false;
        }
        
        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        try {
            int time = (int) (System.currentTimeMillis() / 1000);
            String sql = "update IMTransmitFile set status = 0, updated = ? where "
                    + "taskId = ? limit 1";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setInt(index++, time);
            statement.setString(index++, taskId);
            statement.executeUpdate();
        } catch (SQLException e) {
            logger.error("taskId : " + taskId, e);
            return false;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, null);
        }

        return true;
    }

    /*
     * 
     * @Description: 获取一个用户未接收的所有文件列表
     * 
     * @param toUserId
     * 
     * @return
     */
    public List<TransmitFile> getUserFiles(int toUserId) {

        List<TransmitFile> fileList = new ArrayList<TransmitFile>();
        if (toUserId <= 0) {
            return fileList;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        TransmitFile file = null;
        try {
            int time = (int) (System.currentTimeMillis() / 1000) - 7 * 24 * 3600;
            String sql = "select * from IMTransmitFile where "
                    + "toUserId = ? and created > ? and status = 1 order by created desc limit 50";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setInt(index++, toUserId);
            statement.setInt(index++, time);
            rs = statement.executeQuery();
            while (rs.next()) {
                file = new TransmitFile();
                file.setId(rs.getInt("id"));
                file.setFromUserId(rs.getInt("userId"));
                file.setToUserId(rs.getInt("toUserId"));
                file.setTaskId(rs.getString("taskId"));
                file.setFilePath(rs.getString("filePath"));
                file.setStatus(rs.getInt("status"));
                file.setCreated(rs.getInt("created"));
                file.setUpdated(rs.getInt("updated"));
                file.setFileSize(rs.getInt("fsize"));
                fileList.add(file);
            }
        } catch (SQLException e) {
            logger.error("userId : " + toUserId, e);
            return new ArrayList<TransmitFile>();
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        return fileList;
    }

    /*
     * 
     * @Description: 获取一个发送的文件的记录
     * 
     * @param toUserId
     * 
     * @return
     */
    public TransmitFile getFileRecord(String taskId) {

        if (null == taskId || taskId.isEmpty()) {
            return null;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        TransmitFile file = null;
        try {
            String sql = "select * from IMTransmitFile where "
                    + "taskId = ? limit 1";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setString(index++, taskId);
            rs = statement.executeQuery();
            if (rs.next()) {
                file = new TransmitFile();
                file.setId(rs.getInt("id"));
                file.setFromUserId(rs.getInt("userId"));
                file.setToUserId(rs.getInt("toUserId"));
                file.setTaskId(rs.getString("taskId"));
                file.setFilePath(rs.getString("filePath"));
                file.setStatus(rs.getInt("status"));
                file.setCreated(rs.getInt("created"));
                file.setUpdated(rs.getInt("updated"));
                file.setFileSize(rs.getInt("fsize"));
            }
        } catch (SQLException e) {
            logger.error("taskId : " + taskId, e);
            return null;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        return file;
    }
}