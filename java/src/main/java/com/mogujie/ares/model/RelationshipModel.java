package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.mogujie.ares.data.Relationship;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;
import com.mogujie.ares.util.MoguUtil;

/*
 * 
 * @Description: 好友关系相关的操作model
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-22 下午2:35:34
 *
 */
public class RelationshipModel {

    private static RelationshipModel instance = new RelationshipModel();

    public static RelationshipModel getInstance() {
        if (instance == null) {
            instance = new RelationshipModel();
        }
        return instance;
    }

    /*
     * @Description: 这个接口是默认不走master的
     * 
     * @param userAId
     * 
     * @param userBId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public int getRelateId(int userAId, int userBId) throws SQLException {
        return getRelateId(userAId, userBId, false);
    }

    /*
     * @Description: 获取两个用户之间的关系Id,加为好友的时候会有两条来回的好友关系,
     * 以小的用户Id在前的记录的relateId作为两个人之间的关系Id,查询对话的时候根据关系Id查询
     * 
     * @param userAId
     * 
     * @param userBId
     * 
     * @param isFromMaster 添加关系之前的check要走master
     * 
     * @return
     * 
     * @throws SQLException
     */
    public int getRelateId(int userAId, int userBId, boolean isFromMaster)
            throws SQLException {

        int relateId = -1;

        if (userAId <= 0 || userBId <= 0) {
            return relateId;
        }
        int smallUserId;
        int bigUserId;
        // 取用户Id小的在前的relateId作为两个人的relateId
        if (userAId > userBId) {
            smallUserId = userBId;
            bigUserId = userAId;
        } else {
            smallUserId = userAId;
            bigUserId = userBId;
        }

        DBManager dbManager = DBManager.getInstance();
        DBPoolName dbPoolName = isFromMaster ? DBPoolName.macim_master
                : DBPoolName.macim_slave;
        Connection conn = dbManager.getConnection(dbPoolName);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select relateId from IMRecentContact where userId = ? "
                    + "and friendUserId = ? limit 1";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, smallUserId);
            statement.setObject(index++, bigUserId);
            rs = statement.executeQuery();

            if (rs.next()) {
                relateId = rs.getInt("relateId");
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(dbPoolName, conn, statement, rs);
        }

        return relateId;
    }

    /*
     * @Description: 获取用户们的关系Id
     * 
     * @param userAId
     * 
     * @param userBIds
     * 
     * @return
     * 
     * @throws SQLException
     */
    public int[] getRelateIds(int userAId, int[] userBIds) throws SQLException {

        if (userAId <= 0 || userBIds.length <= 0) {
            return new int[0];
        }
        List<Integer> smallUserIds = new ArrayList<Integer>();
        List<Integer> bigUserIds = new ArrayList<Integer>();
        String smallWhereClause = "";
        String bigWhereClause = "";
        for (int i = 0; i < userBIds.length; i++) { // 分开两批查。。。
            if (userBIds[i] <= 0)
                continue;
            // 取用户Id小的在前的relateId作为两个人的relateId
            if (userAId > userBIds[i]) {
                smallUserIds.add(userBIds[i]);
                smallWhereClause += ",?";
            } else {
                bigUserIds.add(userBIds[i]);
                bigWhereClause += ",?";
            }
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        List<Integer> relateIdList = new ArrayList<Integer>();
        try {
            int smallLength = smallUserIds.size();
            if (smallLength > 0) { // 用户Id比当前用户Id小的一批
                String sql = "select relateId from IMRecentContact where userId in ("
                        + smallWhereClause.substring(1)
                        + ") and friendUserId = ?";
                statement = conn.prepareStatement(sql);
                int index = 1;
                for (int i = 0; i < smallLength; i++) {
                    statement.setObject(index++, smallUserIds.get(i));
                }
                statement.setObject(index++, userAId);
                rs = statement.executeQuery();
                while (rs.next()) {
                    relateIdList.add(rs.getInt("relateId"));
                }
                rs.close();
            }
            int bigLength = bigUserIds.size();
            if (bigLength > 0) { // 用户Id比当前用户Id大的一批
                String sql = "select relateId from IMRecentContact where userId = ? "
                        + " and friendUserId in ("
                        + bigWhereClause.substring(1) + ")";
                statement = conn.prepareStatement(sql);
                int index = 1;
                statement.setObject(index++, userAId);
                for (int i = 0; i < bigLength; i++) {
                    statement.setObject(index++, bigUserIds.get(i));
                }
                rs = statement.executeQuery();
                while (rs.next()) {
                    relateIdList.add(rs.getInt("relateId"));
                }
                rs.close();
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        int size = relateIdList.size();
        int[] relateIds = new int[size];
        for (int i = 0; i < size; i++) {
            relateIds[i] = relateIdList.get(i);
        }

        return relateIds;
    }

    /*
     * 
     * @Description: 获取用户的所有最近联系人列表，默认最多1000个
     * 
     * @param userId 用户Id
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Relationship[] getRecentContactByUserId(int userId, int limit)
            throws SQLException {
        List<Relationship> relateList = new ArrayList<Relationship>();
        Relationship[] relations;
        if (userId <= 0 || limit <= 0) {
            relations = new Relationship[relateList.size()];
            relateList.toArray(relations);
            return relations;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMRecentContact where userId = ? and status = 0 order by updated desc, relateId desc limit "
                    + limit;
            statement = conn.prepareStatement(sql);
            statement.setObject(1, userId);
            rs = statement.executeQuery();

            Relationship relate;
            while (rs.next()) {
                relate = new Relationship();
                relate.setRelateId(rs.getInt("relateId"));
                relate.setUserId(userId);
                relate.setFriendUserId(rs.getInt("friendUserId"));
                relate.setStatus(rs.getInt("status"));
                relate.setCreated(rs.getInt("created"));
                relate.setUpdated(rs.getInt("updated"));
                relateList.add(relate);
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        relations = new Relationship[relateList.size()];
        relateList.toArray(relations);
        return relations;
    }

    /*
     * @Description: 删除用户的某个最近联系人
     * 
     * @param userId 用户Id
     * 
     * @param friendUserId 好友用户Id
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Boolean deleteRecentContactByUserId(int userId, int friendUserId)
            throws SQLException {

        if (userId <= 0 || friendUserId <= 0) {
            return false;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int deletCount = 0;
        try {
            String sql = "update IMRecentContact set status = 1 where userId = ? and friendUserId = ? limit 1";
            statement = conn.prepareStatement(sql);
            statement.setInt(1, userId);
            statement.setInt(2, friendUserId);
            deletCount = statement.executeUpdate();

        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        if (deletCount >= 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * @Description: 加为好友，互相关系，一次插两条
     * 
     * @param userAId
     * 
     * @param userBId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public int addFriendship(int userAId, int userBId) throws SQLException {

        if (userAId <= 0 || userBId <= 0) {
            return -1;
        }
        // 已存在关系
        int relateId = getRelateId(userAId, userBId);
        if (relateId > 0) {
            return relateId;
        }

        int mid;
        // 取用户Id小的在前的relateId作为两个人的relateId,轻度洁癖君表示尽量保证这个关系Id比另一条记录小
        if (userAId > userBId) {
            mid = userAId;
            userAId = userBId;
            userBId = mid;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int ctime = (int) (new Date().getTime() / 1000);
        int defaultStatus = 0;
        int id = -1;
        try {
            // 一次插入2条记录
            if (userAId != userBId) {
                String sql = "insert into IMRecentContact(`userId`, `friendUserId`, `status`, `created`, `updated`) "
                        + "values(?, ?, ?, ?, ?),(?, ?, ?, ?, ?)";
                int index = 1;
                statement = conn.prepareStatement(sql,
                        Statement.RETURN_GENERATED_KEYS);
                statement.setObject(index++, userAId);
                statement.setObject(index++, userBId);
                statement.setObject(index++, defaultStatus);
                statement.setObject(index++, ctime);
                statement.setObject(index++, ctime);
                statement.setObject(index++, userBId);
                statement.setObject(index++, userAId);
                statement.setObject(index++, defaultStatus);
                statement.setObject(index++, ctime);
                statement.setObject(index++, ctime);
            } else { // 自己和自己
                String sql = "insert into IMRecentContact(`userId`, `friendUserId`, `status`, `created`, `updated`) "
                        + "values(?, ?, ?, ?, ?)";
                int index = 1;
                statement = conn.prepareStatement(sql,
                        Statement.RETURN_GENERATED_KEYS);
                statement.setObject(index++, userAId);
                statement.setObject(index++, userBId);
                statement.setObject(index++, defaultStatus);
                statement.setObject(index++, ctime);
                statement.setObject(index++, ctime);
            }
            // 测试
            int count = statement.executeUpdate();
            if (count > 0) {
                rs = statement.getGeneratedKeys();
                if (rs.next()) {
                    id = rs.getInt(1);
                }
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        return id;
    }

    public boolean checkAndUpdateRelation(int userAId, int userBId,
            boolean isFromMaster) throws SQLException {
        if (userAId <= 0 || userBId <= 0) {
            return true;
        }
        DBManager dbManager = DBManager.getInstance();
        DBPoolName dbPoolName = isFromMaster ? DBPoolName.macim_master
                : DBPoolName.macim_slave;
        Connection conn = dbManager.getConnection(dbPoolName);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int relateId = 0;
        List<Integer> updateRelateIds = new ArrayList<Integer>();
        try {
            String sql = "select * from IMRecentContact where (userId = ? "
                    + "and friendUserId = ?) or (userId = ? "
                    + "and friendUserId = ?) limit 2";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, userAId);
            statement.setObject(index++, userBId);
            statement.setObject(index++, userBId);
            statement.setObject(index++, userAId);
            rs = statement.executeQuery();
            while (rs.next()) {
                relateId = rs.getInt("relateId");
                if (rs.getInt("status") != 0) {
                    updateRelateIds.add(relateId);
                }
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(dbPoolName, conn, statement, rs);
        }
        int size = updateRelateIds.size();
        int succCount = 0;
        if (size > 0) {
            dbPoolName = DBPoolName.macim_master;
            conn = dbManager.getConnection(dbPoolName);
            statement = null;
            try {
                String sql = "update IMRecentContact set status = 0 where relateId in (";
                for (int i = 0; i < size; i++) {
                    sql += "?,";
                }
                sql = sql.substring(0, sql.length() - 1) + ") limit 4";
                statement = conn.prepareStatement(sql);
                for (int i = 0; i < size; i++) {
                    statement.setInt(i + 1, updateRelateIds.get(i));
                }
                succCount = statement.executeUpdate();
            } catch (SQLException e) {
                throw e;
            } finally {
                dbManager.release(dbPoolName, conn, statement, null);
            }
        }
        return succCount > 0;
    }

    /*
     * 判断是不是好友
     * 
     * @param userAId
     * 
     * @param userBId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean isRecentContact(int userAId, int userBId)
            throws SQLException {
        int relateId = getRelateId(userAId, userBId);
        return relateId > 0;
    }

    /*
     * 
     * @Description: 获取最后服务这个用户的客服，只在一周之内，如果联系超过一周就忽略了...
     * 
     * @return
     * 
     * @throws Exception
     */
    public int getLastServiceUser(int userId, int[] shopUserId)
            throws Exception {
        if (userId <= 0 || shopUserId == null || shopUserId.length == 0) {
            return 0;
        }

        List<Integer> friendUserIdList = new ArrayList<Integer>();
        String smallWhereClause = MoguUtil.getArgsHolder(shopUserId.length);
        // 一周之内的有效
        int oneWeekAgo = (int) (System.currentTimeMillis() / 1000) - 604800; // 86400
                                                                             // *
                                                                             // 7
        String sql = "select friendUserId from IMRecentContact where userId = ? "
                + "and updated > ? and friendUserId in ("
                + smallWhereClause
                + ") order by updated desc limit 1";

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setInt(index++, userId);
            statement.setInt(index++, oneWeekAgo);
            for (int i = 0; i < friendUserIdList.size(); i++) {
                statement.setObject(index++, friendUserIdList.get(i));
            }
            rs = statement.executeQuery();

            if (rs.next()) {
                return rs.getInt("friendUserId");
            }
        } catch (Exception e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        return 0;
    }

    public boolean updateRelationShip(int fromUserId, int toUserId, int updated)
            throws SQLException {
        if (fromUserId <= 0 || updated <= 0) {
            return false;
        }

        String sql = "update IMRecentContact set updated=? where "
                + "(userId=? and friendUserId=?) or (userId=? and friendUserId=?)";

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        try {
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, updated);
            statement.setObject(index++, fromUserId);
            statement.setObject(index++, toUserId);
            statement.setObject(index++, toUserId);
            statement.setObject(index++, fromUserId);
            statement.executeUpdate();
            return true;
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, null);
        }
    }
}
