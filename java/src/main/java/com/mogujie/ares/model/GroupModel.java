package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.mogujie.ares.data.Group;
import com.mogujie.ares.data.GroupRelation;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;
import com.mogujie.ares.util.MoguUtil;

public class GroupModel {
    private static Logger logger = LoggerFactory.getLogger(GroupModel.class);
    private static GroupModel instance = new GroupModel();

    public static GroupModel getInstance() {
        if (instance == null) {
            instance = new GroupModel();
        }
        return instance;
    }

    private GroupModel() {
    }

    /*
     * 
     * @Description: 获取某用户加入的群个数
     * 
     * @param userId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public List<Group> getGroupsByUserId(int userId, boolean isFixedGroup)
            throws SQLException {
        List<Group> groupList = new ArrayList<Group>();
        if (userId <= 0) {
            return groupList;
        }

        List<Integer> groupIdList = new ArrayList<Integer>();
        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sqlGroupType = isFixedGroup ? " and groupType = 1 " : "";
            int limit = isFixedGroup ? 200 : 5000;
            String sql = "select groupId from IMGroupRelation where userId = ? and "
                    + "status >= 1 "
                    + sqlGroupType
                    + " order by created desc, id desc limit " + limit;
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, userId);
            rs = statement.executeQuery();

            while (rs.next()) {
                groupIdList.add(rs.getInt("groupId"));
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        Map<Integer, Group> groupMap = this.getGroupInfo(groupIdList);
        if (groupMap != null && !groupMap.isEmpty()) {
            groupList.addAll(groupMap.values());
        }
        return groupList;
    }

    /*
     * @Description: 删除用户的某个最近联系群
     * 
     * @param requestUserId
     * 
     * @param groupId 用户群Id
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Boolean deleteRecentContactByGroupId(int requestUserId, int groupId,
            Group group) throws SQLException {

        if (requestUserId <= 0 || groupId <= 0) {
            return false;
        }
        if (group == null || group.getGroupId() != groupId) { // 群不存在
            group = this.getGroupInfo(groupId);
            // 如果群不存在则删除失败，群主也能将自己建的群从最近联系群中删除
            if (group == null || group.getGroupId() != groupId) {
                return false;
            }
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int deletCount = 0;
        try {
            String sql = "update IMGroupRelation set status = 2 where userId = ? and groupId = ? and groupType = 2 limit 1";
            statement = conn.prepareStatement(sql);
            statement.setInt(1, requestUserId);
            statement.setInt(2, groupId);
            deletCount = statement.executeUpdate();

        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        if (deletCount > 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * 
     * @Description: 获取某用户加入的群个数,最多100个
     * 
     * @param userId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public List<Group> getRecentGroupsByUserId(int userId) throws SQLException {
        List<Group> groupList = new ArrayList<Group>();
        if (userId <= 0) {
            return groupList;
        }

        List<Integer> groupIdList = new ArrayList<Integer>();
        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMGroupRelation where userId = ? and "
                    + "status = 1 order by updated desc, id desc limit 100";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, userId);
            rs = statement.executeQuery();

            while (rs.next()) {
                groupIdList.add(rs.getInt("groupId"));
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        Map<Integer, Group> groupMap = this.getGroupInfo(groupIdList);
        if (groupMap != null && !groupMap.isEmpty()) {
            groupList.addAll(groupMap.values());
        }
        return groupList;
    }

    /*
     * 
     * @Description: 取得一坨群组的信息
     * 
     * @param groupIdList
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Map<Integer, Group> getGroupInfo(List<Integer> groupIdList)
            throws SQLException {
        if (groupIdList == null || groupIdList.isEmpty()) {
            return new HashMap<Integer, Group>();
        }
        int length = groupIdList.size();
        int[] groupIds = new int[length];
        for (int i = 0; i < length; i++) {
            groupIds[i] = groupIdList.get(i);
        }

        return getGroupInfo(groupIds);
    }

    /*
     * 
     * @Description: 获取单个群组的信息
     * 
     * @param groupId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Group getGroupInfo(int groupId) throws SQLException {
        if (groupId <= 0) {
            return null;
        }
        int[] groupIds = new int[] { groupId };
        Map<Integer, Group> groups = this.getGroupInfo(groupIds);
        if (null == groups || groups.isEmpty()) {
            return null;
        }
        return groups.get(groupId);
    }

    /*
     * 
     * @Description: 根据群Id获取群信息,最多100
     * 
     * @param groupIdList
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Map<Integer, Group> getGroupInfo(int[] groupIdList)
            throws SQLException {
        Map<Integer, Group> groupMap = new HashMap<Integer, Group>();
        if (groupIdList == null || groupIdList.length <= 0) {
            return groupMap;
        }

        // 用户其它信息
        String whereClause = MoguUtil.getArgsHolder(groupIdList.length);

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMGroup where groupId in ("
                    + whereClause + ") and "
                    + "status >= 1 order by groupId asc limit 5000";
            statement = conn.prepareStatement(sql);
            for (int index = 0; index < groupIdList.length; index++) {
                statement.setObject(index + 1, groupIdList[index]);
            }
            rs = statement.executeQuery();

            int groupId;
            Group group;
            while (rs.next()) {
                group = new Group();
                groupId = rs.getInt("groupId");
                group.setGroupId(groupId);
                group.setGroupName(rs.getString("groupName"));
                group.setAvatar(rs.getString("avatar"));
                group.setAdesc(rs.getString("adesc"));
                group.setCreateUserId(rs.getInt("createUserId"));
                group.setGroupType(rs.getInt("groupType"));
                group.setMemberCnt(rs.getInt("memberCnt"));
                group.setStatus(rs.getInt("status"));
                group.setCreated(rs.getInt("created"));
                group.setUpdated(rs.getInt("updated"));
                groupMap.put(groupId, group);
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        return groupMap;
    }

    /*
     * 
     * @Description: 判断用户是否是群成员
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Boolean isGroupMember(int userId, int groupId) throws SQLException {
        if (userId <= 0 || groupId <= 0) {
            return false;
        }

        List<GroupRelation> groupMembers = this.getGroupMembers(groupId);
        int length = 0;
        if (null != groupMembers && !groupMembers.isEmpty()) {
            length = groupMembers.size();
            for (int i = 0; i < length; i++) {
                if (userId == groupMembers.get(i).getUserId()) {
                    return true;
                }
            }
        }
        return false;
    }

    /*
     * 
     * @Description: 获取单个群的成员关系列表
     * 
     * @param groupId
     * 
     * @return
     * 
     * @throws SQLException
     */
    public List<GroupRelation> getGroupMembers(int groupId) throws SQLException {
        List<GroupRelation> relationList = new ArrayList<GroupRelation>();
        if (groupId <= 0) {
            return relationList;
        }
        int[] groupIds = new int[] { groupId };
        Map<Integer, List<GroupRelation>> groupsMembers = getGroupMembersByRange(groupIds);
        if (groupsMembers != null) {
            List<GroupRelation> tempList = groupsMembers.get(groupId);
            if (tempList != null && !tempList.isEmpty()) {
                relationList = tempList;
            }
        }
        return relationList;
    }

    public Map<Integer, List<GroupRelation>> getGroupMembers(
            List<Integer> groupIds) throws SQLException {
        if (null == groupIds || groupIds.isEmpty()) {
            return new HashMap<Integer, List<GroupRelation>>();
        }
        int size = groupIds.size();
        int[] ids = new int[size];
        for (int i = 0; i < size; i++) {
            ids[i] = groupIds.get(i);
        }
        return getGroupMembers(ids);
    }

    /*
     * 
     * @Description: 分批取群成员列表
     * 
     * @param groupIds
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Map<Integer, List<GroupRelation>> getGroupMembers(int[] groupIds)
            throws SQLException {
        Map<Integer, List<GroupRelation>> groupsRelations = new HashMap<Integer, List<GroupRelation>>();
        if (groupIds == null || groupIds.length <= 0) {
            return groupsRelations;
        }
        int[] ids = null;
        int length = groupIds.length;
        int pos = 0;
        int end = 0;
        int left = length - pos;
        Map<Integer, List<GroupRelation>> partGroupRelations = null;
        while (left > 0) { // 分批去取 每批10个
            end += (left > 10) ? 10 : left;
            ids = Arrays.copyOfRange(groupIds, pos, end);
            partGroupRelations = getGroupMembersByRange(ids);
            if (partGroupRelations != null && !partGroupRelations.isEmpty()) {
                groupsRelations.putAll(partGroupRelations);
            }
            pos += 10;
            left = length - pos;
        }
        return groupsRelations;
    }

    /*
     * 
     * @Description: 获取一坨群与的群成员，建议传入不要超过5个群Id
     * 
     * @param groupIds
     * 
     * @return
     * 
     * @throws SQLException
     */
    private Map<Integer, List<GroupRelation>> getGroupMembersByRange(
            int[] groupIds) throws SQLException {
        Map<Integer, List<GroupRelation>> groupsRelations = new HashMap<Integer, List<GroupRelation>>();
        if (groupIds == null || groupIds.length <= 0) {
            return groupsRelations;
        }

        String whereClause = MoguUtil.getArgsHolder(groupIds.length);

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMGroupRelation where groupId in ("
                    + whereClause + ") and "
                    + "status >= 1 order by created asc limit 10000"; // 10000够大?
            statement = conn.prepareStatement(sql);
            for (int index = 0; index < groupIds.length; index++) {
                statement.setObject(index + 1, groupIds[index]);
            }
            rs = statement.executeQuery();

            int groupId;
            GroupRelation groupRelation = null;
            List<GroupRelation> relationList = null;
            while (rs.next()) {
                groupId = rs.getInt("groupId");
                relationList = groupsRelations.get(groupId);
                if (relationList == null) {
                    relationList = new ArrayList<GroupRelation>();
                    groupsRelations.put(groupId, relationList);
                }
                groupRelation = new GroupRelation();
                groupRelation.setId(rs.getInt("id"));
                groupRelation.setGroupId(groupId);
                groupRelation.setUserId(rs.getInt("userId"));
                groupRelation.setTitle(rs.getInt("title"));
                groupRelation.setGroupType(rs.getInt("groupType"));
                groupRelation.setStatus(rs.getInt("status"));
                groupRelation.setCreated(rs.getInt("created"));
                groupRelation.setUpdated(rs.getInt("updated"));
                relationList.add(groupRelation);
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        List<GroupRelation> relationList = null;
        for (int i = 0; i < groupIds.length; i++) {
            relationList = groupsRelations.get(groupIds[i]);
            if (relationList == null) {
                groupsRelations
                        .put(groupIds[i], new ArrayList<GroupRelation>());
            }
        }
        return groupsRelations;
    }

    /*
     * 
     * @Description: 创建群
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Group createGroup(int createUserId, String groupName, int groupType,
            String avatar, String adesc, int[] memberUserIds)
            throws SQLException {

        logger.info("create group by userId : " + createUserId
                + ", groupName : " + groupName + ", groupType : " + groupType
                + ", avatar : " + avatar + ", adesc : " + adesc
                + ", memberUserIds : " + Arrays.toString(memberUserIds));
        if (createUserId <= 0 || groupName == null || groupType <= 0) {
            return null;
        }
        avatar = (avatar == null ? "" : avatar);
        adesc = (adesc == null ? "" : adesc);
        int[] uids = MoguUtil.distinct(memberUserIds); // 去重

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        int time = (int) (System.currentTimeMillis() / 1000);
        ResultSet rs = null;
        int succCount = 0;
        int groupId = 0;
        Group group = new Group();
        group.setGroupId(0);
        group.setGroupName(groupName);
        group.setAvatar(avatar);
        group.setAdesc(adesc);
        group.setCreateUserId(createUserId);
        group.setGroupType(groupType);
        group.setMemberCnt(uids.length); //
        group.setStatus(1);
        group.setCreated(time);
        group.setUpdated(time);
        try {
            // 创建群
            String sql = "insert into IMGroup(`groupName`, `avatar`,  `adesc`, `createUserId`, "
                    + "`groupType`, `memberCnt`, `created`, `updated`) "
                    + "values(?, ?, ?, ?, ?, ?, ?, ?)";
            statement = conn.prepareStatement(sql,
                    Statement.RETURN_GENERATED_KEYS);
            int index = 1;
            statement.setString(index++, group.getGroupName());
            statement.setString(index++, group.getAvatar());
            statement.setString(index++, group.getAdesc());
            statement.setInt(index++, group.getCreateUserId());
            statement.setInt(index++, group.getGroupType());
            statement.setInt(index++, 0); // uidSize初始化时设置为0，接下来会在joinGroup方法中再更新群成员数
            statement.setInt(index++, group.getCreated());
            statement.setInt(index++, group.getUpdated());
            succCount = statement.executeUpdate();
            if (succCount > 0) {
                rs = statement.getGeneratedKeys();
                if (rs != null && rs.next()) {
                    groupId = rs.getInt(1);
                    group.setGroupId(groupId);
                    logger.info("seisei create group success! groupId:"
                            + groupId);
                }
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        if (group.getGroupId() > 0) { // 成功

            logger.info("create group success! groupId : " + group.getGroupId()
                    + "createUserId : " + createUserId + ", groupName : "
                    + groupName + ", groupType : " + groupType + ", avatar : "
                    + avatar + ", adesc : " + adesc + ", memberUserIds : "
                    + Arrays.toString(memberUserIds));
            this.joinGroup(uids, groupId, group); // 将选中的人员全部加入新创建的群，并更新计群成员数
            return group;
        }

        logger.info("create group error! createUserId : " + createUserId
                + ", groupName : " + groupName + ", groupType : " + groupType
                + ", avatar : " + avatar + ", adesc : " + adesc
                + ", memberUserIds : " + Arrays.toString(memberUserIds));
        return null;
    }

    /*
     * 
     * @Description: 加入群
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean joinGroup(int[] userIds, int groupId, Group group)
            throws SQLException {

        logger.info("seisei join group! groupId:" + groupId);
        if (userIds == null || userIds.length <= 0 || groupId <= 0) {
            return false;
        }
        if (group == null || group.getGroupId() != groupId) { // 群不存在
            group = this.getGroupInfo(groupId); // 获得群信息
            if (group == null || group.getGroupId() != groupId) {
                return false;
            }
        }

        Map<Integer, Integer> mapUserIds = MoguUtil.distinctToMap(userIds); // 去重

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int time = (int) (System.currentTimeMillis() / 1000);
        int countAddedNum = 0;
        try {
            logger.info("seisei join group! select phase");
            String selectClause = MoguUtil.getArgsHolder(mapUserIds.size());
            String sqlGetRelation = "select * from IMGroupRelation where groupId = ? and userId in (";
            sqlGetRelation += selectClause + ") group by userId order by id";

            statement = conn.prepareStatement(sqlGetRelation);
            statement.setInt(1, groupId);
            Iterator<Integer> itr = mapUserIds.keySet().iterator();
            int i = 1;
            while (itr.hasNext()) {
                statement.setObject(++i, itr.next());
            }
            rs = statement.executeQuery();

            List<Integer> updateUids = new ArrayList<Integer>();
            int uid = 0;
            int status;
            while (rs.next()) {
                status = rs.getInt("status");
                uid = rs.getInt("userId");
                mapUserIds.remove(uid); // 表里已经有了就不用插入了
                if (status == 0) { // 之前退出过更新就可以了
                    updateUids.add(uid);
                }
            }
            rs.close();
            statement.close();

            // 创建关系
            int createUserId = group.getCreateUserId();
            int groupType = group.getGroupType();
            int insertSize = mapUserIds.size();
            int insertCount = 0;
            if (insertSize > 0) {
                logger.info("seisei join group! add phase");
                String addClause = MoguUtil.getArgsHolder("(?, ?, ?, ?, ?, ?)",
                        ",", insertSize);
                String sqlAddRelation = "insert into IMGroupRelation(`groupId`, `userId`, `title`, `groupType`, `created`, `updated`) "
                        + "values " + addClause;
                statement = conn.prepareStatement(sqlAddRelation);
                Iterator<Integer> iter = mapUserIds.values().iterator();
                int index = 1;
                while (iter.hasNext()) {
                    uid = iter.next();
                    statement.setInt(index++, groupId);
                    statement.setInt(index++, uid);
                    statement.setInt(index++, createUserId == uid ? 1 : 0);
                    statement.setInt(index++, groupType);
                    statement.setInt(index++, time);
                    statement.setInt(index++, time);
                }
                insertCount = statement.executeUpdate();
                statement.close();
            }

            // 更新关系
            int updateSize = updateUids.size();
            int updateCount = 0;
            if (updateSize > 0) {
                logger.info("seisei join group! update phase");
                String updateClause = MoguUtil.getArgsHolder(insertSize);
                String sqlUpdateRelation = "update IMGroupRelation set status = 1 where groupId = ? and userId in (";
                sqlUpdateRelation += updateClause + ")";
                statement = conn.prepareStatement(sqlUpdateRelation);
                int index = 1;
                statement.setInt(index++, groupId);
                for (int j = 0; j < updateSize; j++) {
                    statement.setInt(index++, updateUids.get(j));
                }
                updateCount = statement.executeUpdate();
                statement.close();
            }

            // 更新群计数
            countAddedNum = insertCount + updateCount;
            if (countAddedNum > 0) {
                logger.info("seisei join group! update count phase");
                String sqlUpdateMemberCnt = "update IMGroup set memberCnt = memberCnt + ? where groupId = ? limit 1";
                statement = conn.prepareStatement(sqlUpdateMemberCnt);
                statement.setInt(1, countAddedNum);
                statement.setInt(2, groupId);
                statement.executeUpdate();
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        if (countAddedNum > 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * 
     * @Description: 更新群关系
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean checkAndUpdateGroupRelation(int userId, int groupId, int time)
            throws SQLException {

        if (userId <= 0 || groupId <= 0 || time <= 0) {
            return false;
        }
        Group group = this.getGroupInfo(groupId);
        if (group == null || group.getGroupId() != groupId) {
            return false;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int updateCount = 0;
        try {
            // 获得所有将该群从最近联系人里删除的userId
            String sqlGetRelation = "select userId from IMGroupRelation where groupId = ? and status = 2 group by userId order by id";
            statement = conn.prepareStatement(sqlGetRelation);
            statement.setInt(1, groupId);
            rs = statement.executeQuery();

            List<Integer> updateUids = new ArrayList<Integer>();
            while (rs.next()) {
                updateUids.add(rs.getInt("userId"));
            }

            // 更新关系
            int updateSize = updateUids.size();

            if (updateSize > 0) {
                String updateClause = MoguUtil.getArgsHolder(updateSize);
                // 设置群关系状态，将所有设置成1 0：退出群 1：正常（默认值）2：正常（但从最近联系人中移除）
                String sqlUpdateRelation = "update IMGroupRelation set status = 1,updated = ? where groupId = ? and userId in (";
                sqlUpdateRelation += updateClause + ")";
                statement = conn.prepareStatement(sqlUpdateRelation);
                int index = 1;
                statement.setInt(index++, time);
                statement.setInt(index++, groupId);
                for (int i = 0; i < updateSize; i++) {
                    statement.setInt(index++, updateUids.get(i));
                }
                updateCount = statement.executeUpdate();
            }

        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        if (updateCount > 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * 
     * @Description: 修改群名字和头像
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean modifyGroup(int requestUserId, int groupId,
            String groupName, String groupAvater, Group group)
            throws SQLException {

        if (requestUserId <= 0 || groupId <= 0 || groupName == null
                || groupName.equals("")) {
            return false;
        }
        if (group == null || group.getGroupId() != groupId
                || requestUserId != group.getCreateUserId()) { // 群不存在或不是群创建者
            group = this.getGroupInfo(groupId);
            if (group == null || group.getGroupId() != groupId
                    || requestUserId != group.getCreateUserId()) {
                return false;
            }
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int time = (int) (System.currentTimeMillis() / 1000);
        int countUpdate = 0;
        try {
            // 更新群名和头像
            String sqlUpdateMemberCnt = "update IMGroup set groupName = ? , avatar = ? , updated = ? where groupId = ? limit 1";
            statement = conn.prepareStatement(sqlUpdateMemberCnt);
            int index = 1;
            statement.setString(index++, groupName);
            statement.setString(index++, groupAvater);
            statement.setInt(index++, time);
            statement.setInt(index++, groupId);
            countUpdate = statement.executeUpdate();
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }

        if (countUpdate > 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * 
     * @Description: 退出群
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean quitGroup(int requestUserId, int[] userIds, int groupId,
            Group group) throws SQLException {

        if (requestUserId <= 0 || userIds == null || userIds.length <= 0
                || groupId <= 0) {
            return false;
        }
        if (group == null || group.getGroupId() != groupId
                || requestUserId != group.getCreateUserId()) { // 群不存在或不是群创建者
            group = this.getGroupInfo(groupId);
            if (group == null || group.getGroupId() != groupId
                    || requestUserId != group.getCreateUserId()) {
                return false;
            }
        }

        Map<Integer, Integer> mapUserIds = MoguUtil.distinctToMap(userIds); // 去重

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int time = (int) (System.currentTimeMillis() / 1000);
        int countDelete = 0;
        try {
            String selectClause = MoguUtil.getArgsHolder(mapUserIds.size());
            String sqlGetRelation = "select * from IMGroupRelation where groupId = ? and userId in (";
            sqlGetRelation += selectClause + ") group by userId";
            statement = conn.prepareStatement(sqlGetRelation);
            statement.setInt(1, groupId);
            Iterator<Integer> itr = mapUserIds.keySet().iterator();
            int i = 1;
            while (itr.hasNext()) {
                statement.setInt(++i, itr.next());
            }
            rs = statement.executeQuery();

            List<Integer> updateUids = new ArrayList<Integer>();
            int status;
            int uid = 0;
            while (rs.next()) {
                status = rs.getInt("status");
                uid = rs.getInt("userId"); // 在表里，表明是该群成员
                if (status != 0) { // 在表里，而且未退出群更新状态为退出群就可以了
                    updateUids.add(uid);
                }
            }

            // 更新关系
            int updateSize = updateUids.size();
            if (updateSize > 0) {
                String updateClause = MoguUtil.getArgsHolder(updateSize);
                String sqlUpdateRelation = "update IMGroupRelation set status = 0, updated = ? where groupId = ? and userId in (";
                sqlUpdateRelation += updateClause + ")";
                statement = conn.prepareStatement(sqlUpdateRelation);
                int index = 1;
                statement.setInt(index++, time);
                statement.setInt(index++, groupId);
                for (int j = 0; j < updateSize; j++) {
                    statement.setInt(index++, updateUids.get(j));
                }
                countDelete = statement.executeUpdate();
            }

            // 更新群计数
            if (countDelete > 0) {
                String sqlUpdateMemberCnt = "update IMGroup set memberCnt = memberCnt - ? , updated = ? where groupId = ? limit 1";
                statement = conn.prepareStatement(sqlUpdateMemberCnt);
                int index = 1;
                statement.setInt(index++, countDelete);
                statement.setInt(index++, time);
                statement.setInt(index++, groupId);
                statement.executeUpdate();
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        if (countDelete > 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * 
     * @Description: 更新群的时间
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean updateGroupTime(int groupId, int time) throws SQLException {

        if (time <= 0  || groupId <= 0) {
            return false;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sqlUpdate = "update IMGroup set updated = ? where groupId = ?";
            statement = conn.prepareStatement(sqlUpdate);
            statement.setInt(1, time);
            statement.setInt(2, groupId);
            statement.executeUpdate();

        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        return true;
    }
}
