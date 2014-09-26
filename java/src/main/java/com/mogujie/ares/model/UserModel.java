package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.data.User;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;
import com.mogujie.ares.util.MoguUtil;

/*
 * @Description: 用户相关的model，包括获取用户信息等
 * @author ziye
 * 
 */
public class UserModel {

    private static User serverUser = null;

    private static final Logger logger = LoggerFactory
            .getLogger(UserModel.class);
    private static UserModel instance = new UserModel();

    public static UserModel getInstance() {
        if (instance == null) {
            instance = new UserModel();
        }
        return instance;
    }

    /*
     * @Description: 获取服务用户(小T)的信息
     * 
     * @param userId
     * 
     * @return User 用户的具体信息，包括用户id，用户名，头像链接 null 参数有误或取不到用户信息时返回null
     * 
     * @throws SQLException
     */
    public User getServerUserInfo() throws SQLException {
        if (serverUser == null) {
            serverUser = getUserInfo(BizConstants.SYS_SERVER_USER_ID);
        }
        return serverUser;
    }

    /*
     * @Description: 获取单个用户的信息
     * 
     * @param userId
     * 
     * @return User 用户的具体信息，包括用户id，用户名，头像链接 null 参数有误或取不到用户信息时返回null
     * 
     * @throws SQLException
     */
    public User getUserInfo(int userId) throws SQLException {
        Map<Integer, User> userInfos = getUserInfo(new int[] { userId });
        return (userInfos == null) ? null : userInfos.get(userId);
    }

    /*
     * 
     * @Description: 获取一组用户的信息,蛋疼的Integer数组
     * 
     * @param userIds
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Map<Integer, User> getUserInfo(Integer[] userIds)
            throws SQLException {
        int[] uIds = new int[userIds.length];
        for (int i = 0; i < userIds.length; i++) {
            uIds[i] = userIds[i];
        }
        return getUserInfo(uIds);
    }

    /*
     * @Description: 获取一组用户的信息
     * 
     * @param userIds
     * 
     * @return Map<Integer, User> 用户的具体信息Map，每个元素包括用户id，用户名，头像链接
     * 
     * @throws SQLException
     */
    public Map<Integer, User> getUserInfo(int[] userIds) throws SQLException {
        Map<Integer, User> mapUsers = new HashMap<Integer, User>();
        userIds = MoguUtil.distinct(userIds); // 去除重复的userId
        int countIds = userIds.length;

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String clause = MoguUtil.getArgsHolder(countIds);
            String sql = "select * from IMUsers where id in (" + clause
                    + ") and status=0";
            statement = conn.prepareStatement(sql);
            for (int i = 0; i < countIds; i++) {
                statement.setInt(i + 1, userIds[i]);
            }
            rs = statement.executeQuery();
            User user = null;
            while (rs.next()) {
                user = extractUser(rs);
                mapUsers.put(user.getUserId(), user);
            }
        } catch (SQLException e) {
            logger.error("get user info error with reason : " + e);
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        return mapUsers;
    }

    /*
     * @Description: 获取一个部门的所有用户的信息
     * 
     * @param uname
     * 
     * @return Set<User> 用户的具体信息集合，每个元素包括用户id，用户名，头像链接
     * 
     * @throws SQLException
     */
    public Set<User> getUserInfoByDepartId(int departId) throws SQLException {
        Set<User> setUsers = new HashSet<User>();
        Connection conn = DBManager.getInstance().getConnection(
                DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;

        try {
            String sql = "select * from IMUsers where departId = ? limit 5000"; // 最多取5000个用户
            statement = conn.prepareStatement(sql);
            statement.setInt(1, departId);
            rs = statement.executeQuery();
            User user = null;
            while (rs.next()) {
                user = extractUser(rs);
                setUsers.add(user);
            }
        } catch (SQLException e) {
            logger.error("get user info by department id error with reason : "
                    + e);
            throw e;
        } finally {
            DBManager.getInstance().release(DBPoolName.macim_slave, conn,
                    statement, rs);
        }

        return setUsers;
    }

    /*
     * @Description: 获取所有用户的信息;默认聊天用户上限5000人
     * 
     * @return Set<User> 所有用户的具体信息，每个元素包括用户id，用户名，头像链接
     * 
     * @throws SQLException
     */
    public Set<User> getAllUserInfo() throws SQLException {
        Set<User> setUsers = new HashSet<User>();
        Connection conn = DBManager.getInstance().getConnection(
                DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;

        try {
            String sql = "select * from IMUsers where status = 0 limit 5000"; // 最多取5000个用户
            statement = conn.prepareStatement(sql);
            rs = statement.executeQuery();
            User user = null;
            while (rs.next()) {
                user = extractUser(rs);
                setUsers.add(user);
            }
        } catch (SQLException e) {
            logger.error("get all user info error with reason : " + e);
            throw e;
        } finally {
            DBManager.getInstance().release(DBPoolName.macim_slave, conn,
                    statement, rs);
        }

        return setUsers;
    }

    /*
     * @Description: 获取一个用户的信息
     * 
     * @param uname
     * 
     * @return User 用户的具体信息，每个元素包括用户id，用户名，头像链接
     * 
     * @throws SQLException
     */
    public User getUserInfo(String uname) throws SQLException {
        Map<String, User> usersInfos = getUserInfo(new String[] { uname });
        return (usersInfos == null) ? null : usersInfos.get(uname);
    }

    /*
     * @Description: 获取一组用户的信息
     * 
     * @param unames
     * 
     * @return User Map 用户的具体信息Map，每个元素包括用户id，用户名，头像链接
     * 
     * @throws SQLException
     */
    public Map<String, User> getUserInfo(String[] unames) throws SQLException {
        Map<String, User> mapUsers = new HashMap<String, User>();

        Connection conn = DBManager.getInstance().getConnection(
                DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;

        try {
            String clause = MoguUtil.getArgsHolder(unames.length);
            String sql = "select * from IMUsers where uname in (" + clause
                    + ") limit 5000"; // 最多取5000个用户
            statement = conn.prepareStatement(sql);
            for (int i = 0; i < unames.length; i++) {
                statement.setString(i + 1, unames[i]);
            }
            rs = statement.executeQuery();
            User user = null;
            while (rs.next()) {
                user = extractUser(rs);
                mapUsers.put(user.getUname(), user);
            }
        } catch (SQLException e) {
            logger.error("get user info error with reason : " + e);
            throw e;
        } finally {
            DBManager.getInstance().release(DBPoolName.macim_slave, conn,
                    statement, rs);
        }

        return mapUsers;
    }

    // /**
    // * @Description: 修改用户头像
    // * @param userId
    // * @param avatar
    // * @return
    // * @throws SQLException
    // */
    // public int alterAvatar(int userId, String avatar) throws SQLException {
    // if (userId <= 0 || avatar == null || avatar.equals("")) {
    // throw new IllegalArgumentException("argument error: userId:"
    // + userId + "avatar: " + avatar);
    // }
    // DBManager dbManager = DBManager.getInstance();
    // Connection conn = dbManager.getConnection(DBPoolName.macim_master);
    // PreparedStatement statement = null;
    // ResultSet rs = null;
    // int time = (int) (System.currentTimeMillis() / 1000);
    // int countUpdate = 0;
    // try {
    // // 更新用户头像
    // String sqlUpdateMemberCnt =
    // "update IMUsers set avatar = ? , updated = ? where userId = ? limit 1";
    // statement = conn.prepareStatement(sqlUpdateMemberCnt);
    // int index = 1;
    // statement.setString(index++, avatar);
    // statement.setInt(index++, time);
    // statement.setInt(index++, userId);
    // countUpdate = statement.executeUpdate();
    // } catch (SQLException e) {
    // throw e;
    // } finally {
    // dbManager.release(DBPoolName.macim_master, conn, statement, rs);
    // }
    // if (0 == countUpdate) {
    // return 1;
    // }
    // return 0;
    // }

    // /**
    // *
    // * @Description: 获得用户自定义的头像
    // * @param userIds
    // * @return
    // * @throws SQLException
    // */
    // public Map<Integer, String> getUserAvatar(int[] userIds)
    // throws SQLException {
    // Map<Integer, String> mapUserAvatars = new HashMap<Integer, String>();
    // if (userIds == null || userIds.length <= 0) {
    // return mapUserAvatars;
    // }
    //
    // Connection conn = DBManager.getInstance().getConnection(
    // DBPoolName.macim_slave);
    // PreparedStatement statement = null;
    // ResultSet rs = null;
    //
    // try {
    // String clause = "";
    // for (int i = 0; i < userIds.length; i++) {
    // clause += ",?";
    // }
    // clause = clause.substring(1);
    // String sql =
    // "select userId, avatar from IMUsers where avatar <> '' and userId in ("
    // + clause + ")"; // 最多取5000个用户
    // statement = conn.prepareStatement(sql);
    // for (int i = 0; i < userIds.length; i++) {
    // statement.setInt(i + 1, userIds[i]);
    // }
    // rs = statement.executeQuery();
    // while (rs.next()) {
    // int userId = rs.getInt("userId");
    // String avatar = rs.getString("avatar");
    // if (avatar != null && !"".equals(avatar)) {
    // mapUserAvatars.put(userId, avatar);
    // }
    // }
    // } catch (SQLException e) {
    // throw e;
    // } finally {
    // DBManager.getInstance().release(DBPoolName.macim_slave, conn,
    // statement, rs);
    // }
    //
    // return mapUserAvatars;
    // }

    // /**
    // *
    // * @Description: 获得用户自定义的头像
    // * @param userIds
    // * @return
    // * @throws SQLException
    // */
    // public Map<Integer, String> getUserAvatar(String[] unames)
    // throws SQLException {
    // Map<Integer, String> mapUserAvatars = new HashMap<Integer, String>();
    // if (unames == null || unames.length <= 0) {
    // return mapUserAvatars;
    // }
    //
    // Connection conn = DBManager.getInstance().getConnection(
    // DBPoolName.macim_slave);
    // PreparedStatement statement = null;
    // ResultSet rs = null;
    //
    // try {
    // String clause = "";
    // for (int i = 0; i < unames.length; i++) {
    // clause += ",?";
    // }
    // clause = clause.substring(1);
    // String sql =
    // "select userId, avatar from IMUsers where avatar <> '' and uname in ("
    // + clause + ")"; // 最多取5000个用户
    // statement = conn.prepareStatement(sql);
    // for (int i = 0; i < unames.length; i++) {
    // statement.setString(i + 1, unames[i]);
    // }
    // rs = statement.executeQuery();
    // while (rs.next()) {
    // int userId = rs.getInt("userId");
    // String avatar = rs.getString("avatar");
    // if (avatar != null && !"".equals(avatar)) {
    // mapUserAvatars.put(userId, avatar);
    // }
    // }
    // } catch (SQLException e) {
    // throw e;
    // } finally {
    // DBManager.getInstance().release(DBPoolName.macim_slave, conn,
    // statement, rs);
    // }
    //
    // return mapUserAvatars;
    // }

    /*
     * 从ResultSet中提取User
     */
    private User extractUser(ResultSet rs) throws SQLException {
        User user = new User();
        user.setUname(rs.getString("uname"));
        user.setUserId(rs.getInt("id"));
        user.setStatus(rs.getInt("status"));
        user.setTitle((rs.getString("title")));
        user.setDepartId(((rs.getInt("departId"))));
        user.setSex(((rs.getInt("sex"))));
        user.setJobNumber(((rs.getInt("jobNumber"))));
        user.setTelphone(((rs.getString("telphone"))));
        user.setMail(rs.getString("mail"));
        user.setPosition(rs.getString("position"));
        user.setAvatar(rs.getString("avatar"));
        user.setUnick(rs.getString("nickName"));
        return user;

    }
}
