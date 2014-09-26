package com.mogujie.ares.extend.action;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.mogujie.ares.data.Group;
import com.mogujie.ares.data.GroupRelation;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.GroupModel;
import com.mogujie.ares.util.MoguUtil;

public class GroupAction extends BaseAction {

    private static final Logger logger = LoggerFactory
            .getLogger(GroupAction.class);

    /*
     * 
     * @Description: 返回用户加入的群信息
     * 
     * @param userId
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer getUserGroups(int userId, DataBuffer attachment,
            int version) {

        DataBuffer buffer = null;
        if (userId <= 0) {
            buffer = new DataBuffer(12);
            buffer.writeInt(userId); // 用户Id
            buffer.writeInt(0); // 用户加入的群个数
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        logger.info("get user group list: " + userId);
        buffer = new DataBuffer();
        buffer.writeInt(userId);
        try {
            List<Group> groupList = GroupModel.getInstance().getGroupsByUserId(
                    userId, true); // 获得用户的群
            if (groupList != null && !groupList.isEmpty()) {
                int groupCount = groupList.size();
                Group group;
                List<Integer> listIds = new ArrayList<Integer>();
                Map<Integer, Group> mapGroupInfo = new HashMap<Integer, Group>();
                int groupId = 0;
                for (int i = 0; i < groupCount; i++) {
                    group = groupList.get(i);
                    // status == 0 表示群已被删除
                    if (group != null && group.getStatus() > 0) {
                        groupId = group.getGroupId();
                        mapGroupInfo.put(groupId, group);
                        listIds.add(groupId);
                    }
                }
                Map<Integer, List<GroupRelation>> mapRelations = GroupModel
                        .getInstance().getGroupMembers(listIds);
                if (mapRelations == null) {
                    mapRelations = new HashMap<Integer, List<GroupRelation>>();
                }
                DataBuffer tmpBuffer = new DataBuffer();
                groupCount = listIds.size();
                List<GroupRelation> relations = null;
                GroupRelation relation = null;
                StringBuffer logBuffer = new StringBuffer();
                int memberId = 0;
                for (int i = 0; i < groupCount; i++) {
                    groupId = listIds.get(i);
                    logBuffer.append("[" + groupId + ":");
                    group = mapGroupInfo.get(groupId);
                    relations = mapRelations.get(groupId);
                    tmpBuffer.writeInt(group.getGroupId());
                    tmpBuffer.writeString(group.getGroupName());
                    tmpBuffer.writeString(group.getAvatar());
                    tmpBuffer.writeInt(group.getGroupType());
                    tmpBuffer.writeInt(group.getCreateUserId());
                    if (relations == null || relations.isEmpty()) {
                        logBuffer.append(0);
                        tmpBuffer.writeInt(0);
                    } else {
                        int relLength = relations.size();
                        logBuffer.append(relLength);
                        tmpBuffer.writeInt(relLength);
                        for (int j = 0; j < relLength; j++) {
                            relation = relations.get(j);
                            memberId = relation.getUserId();
                            tmpBuffer.writeInt(memberId);
                        }
                    }
                    logBuffer.append("]");
                }

                buffer.writeInt(groupCount);
                if (groupCount > 0) {
                    buffer.writeDataBuffer(tmpBuffer);
                }
                logger.info("return user group list: " + userId + " - "
                        + logBuffer.toString());
            } else {
                buffer.writeInt(0);
                logger.info("return user group list: " + userId);
            }
            return MoguUtil.writeAttachments(buffer, attachment);
        } catch (SQLException e) {
            logger.error("get user group list " + userId, e);
        }
        buffer.writeInt(0);

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 返回用户加入的群信息
     * 
     * @param userId
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer getUserRecentGroups(int userId, DataBuffer attachment,
            int version) {

        DataBuffer buffer = null;
        if (userId <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(userId); // 用户Id
            buffer.writeInt(0); // 用户加入的群个数

            return MoguUtil.writeAttachments(buffer, attachment);
        }

        logger.info("get user recent group list: " + userId);
        buffer = new DataBuffer();
        buffer.writeInt(userId);
        try {
            GroupModel groupModel = GroupModel.getInstance();
            List<Group> groupList = groupModel.getRecentGroupsByUserId(userId); // 获得用户的群
            logger.info("get user recent group relation list: " + userId);
            if (groupList != null && !groupList.isEmpty()) {
                int groupIdCount = groupList.size();
                Group group;
                List<Integer> idList = new ArrayList<Integer>();
                Map<Integer, Group> groupInfoMap = new HashMap<Integer, Group>();
                int groupId = 0;
                for (int i = 0; i < groupIdCount; i++) {
                    group = groupList.get(i);
                    if (group != null && group.getStatus() > 0) { // status == 0
                                                                  // 表示群已被删除
                        groupId = group.getGroupId();
                        groupInfoMap.put(groupId, group);
                        idList.add(groupId);
                    }
                }
                Map<Integer, List<GroupRelation>> relationMap = groupModel
                        .getGroupMembers(idList);
                logger.info("get user recent group relation memberlist: "
                        + userId);
                if (relationMap == null) {
                    relationMap = new HashMap<Integer, List<GroupRelation>>();
                }
                DataBuffer tmpBuffer = new DataBuffer();
                groupIdCount = idList.size();
                List<GroupRelation> relations = null;
                GroupRelation relation = null;
                String logData = "";
                int memberId = 0;
                for (int i = 0; i < groupIdCount; i++) {
                    groupId = idList.get(i);
                    logData += "[" + groupId + ":";
                    group = groupInfoMap.get(groupId);
                    relations = relationMap.get(groupId);
                    tmpBuffer.writeInt(group.getGroupId());
                    tmpBuffer.writeString(group.getGroupName());
                    tmpBuffer.writeString(group.getAvatar());
                    tmpBuffer.writeInt(group.getGroupType());
                    tmpBuffer.writeInt(group.getCreateUserId());
                    tmpBuffer.writeInt(group.getUpdated());
		    logData += "updatedTime = " + group.getUpdated() + ",";
                    if (relations == null || relations.isEmpty()) {
                        logData += 0;
                        tmpBuffer.writeInt(0);
                    } else {
                        int relLength = relations.size();
                        logData += relLength;
                        tmpBuffer.writeInt(relLength);
                        for (int j = 0; j < relLength; j++) {
                            relation = relations.get(j);
                            memberId = relation.getUserId();
                            tmpBuffer.writeInt(memberId);
                        }
                    }
                    logData += "]";
                }

                buffer.writeInt(groupIdCount);
                if (groupIdCount > 0) {
                    buffer.writeDataBuffer(tmpBuffer);
                }
                logger.info("return user recent group list: " + userId + " - "
                        + logData);
            } else {
                buffer.writeInt(0);
                logger.info("return user recent group list: " + userId);
            }
            return MoguUtil.writeAttachments(buffer, attachment);
        } catch (SQLException e) {
            logger.error("get user group list " + userId, e);
        }
        buffer.writeInt(0);
        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 获取群详细信息
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer getGroupsDetail(int userId, int groupId,
            DataBuffer attachment, int version) {

        logger.info("get user group detail: " + userId + ", groupId = "
                + groupId);
        DataBuffer buffer = null;
        if (userId <= 0 || groupId <= 0) {
            buffer = new DataBuffer(12);
            buffer.writeInt(userId); // 用户Id
            buffer.writeInt(groupId); // 群id
            buffer.writeInt(0); // 查询结果
            return MoguUtil.writeAttachments(buffer, attachment);
        }
        buffer = new DataBuffer();
        buffer.writeInt(userId);
        buffer.writeInt(groupId);

        GroupModel groupModel = GroupModel.getInstance();
        Group groupInfo = null;
        int isSuccess = 0;
        try {
            groupInfo = groupModel.getGroupInfo(groupId);
            if (groupInfo == null) { // 群不存在
                isSuccess = 1;
                buffer.writeInt(isSuccess);
                buffer.writeInt(attachment.readableBytes());
                if (attachment.readableBytes() > 0) {
                    buffer.writeDataBuffer(attachment);
                }
                return buffer;
            }
            DataBuffer tmpDataBuffer = new DataBuffer();
            tmpDataBuffer.writeInt(isSuccess);
            tmpDataBuffer.writeString(groupInfo.getGroupName());
            tmpDataBuffer.writeString(groupInfo.getAvatar());
            tmpDataBuffer.writeInt(groupInfo.getGroupType());
            tmpDataBuffer.writeInt(groupInfo.getCreateUserId());
            List<GroupRelation> groupRelationList = groupModel
                    .getGroupMembers(groupId);
            int size = 0;
            if (groupRelationList != null) {
                size = groupRelationList.size();
            }
            GroupRelation relation = null;
            List<Integer> uidList = new ArrayList<Integer>();
            for (int i = 0; i < size; i++) {
                relation = groupRelationList.get(i);
                if (relation != null && relation.getUserId() > 0) {
                    uidList.add(relation.getUserId());
                }
            }
            tmpDataBuffer.writeIntList(uidList);
            buffer.writeDataBuffer(tmpDataBuffer);
            return MoguUtil.writeAttachments(buffer, attachment);
        } catch (SQLException e) {
            logger.error("get user group info " + userId + " - " + groupId, e);
            isSuccess = 2;
        }
        buffer.writeInt(isSuccess);
        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 创建群
     * 
     * @param userId
     * 
     * @param groupName
     * 
     * @param groupAvater
     * 
     * @param memberUserIds
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer createGroup(int userId, String groupName,
            String groupAvater, int[] memberUserIds, DataBuffer attachment,
            int version) {

        logger.info("create group userId: " + userId + ", groupName = "
                + groupName + ", groupAvater = " + groupAvater + ", members = "
                + Arrays.toString(memberUserIds));
        DataBuffer buffer = null;
        // 数据校验
        if (userId <= 0 || groupName == null || groupName.equals("")
                || memberUserIds == null || memberUserIds.length == 0) {
            buffer = new DataBuffer();
            buffer.writeInt(userId); // 用户Id
            buffer.writeInt(1); // 结果,0成功
            buffer.writeInt(0); // 群id
            buffer.writeString(""); // 查询结果
            buffer.writeIntArray(new int[0]);
            return MoguUtil.writeAttachments(buffer, attachment);
        }
        // 群成员太多
        if (memberUserIds.length > 1000) {
            logger.error("群" + groupName + "成员太多,创建失败!");
            buffer = new DataBuffer();
            buffer.writeInt(userId); // 用户Id
            buffer.writeInt(2); // 结果,0成功
            buffer.writeInt(0); // 群id
            buffer.writeString(""); // 群名字
            buffer.writeIntArray(new int[0]); // 群成员
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        buffer = new DataBuffer();
        buffer.writeInt(userId);

        Group group = null;
        try {
            group = GroupModel.getInstance().createGroup(userId, groupName, 2,
                    groupAvater, "", memberUserIds);
        } catch (SQLException e) {
            logger.error(
                    "create group error: userId = " + userId + ", groupName = "
                            + groupName + ", groupAvater =" + groupAvater
                            + ", uids = " + Arrays.toString(memberUserIds), e);
        }
        int groupId = (group != null) ? group.getGroupId() : 0;
        if (groupId > 0) {
            buffer.writeInt(0); // result
            buffer.writeInt(groupId); // 群id
            buffer.writeString(groupName); // 群名字
            buffer.writeIntArray(memberUserIds); // 群成员
        } else {
            logger.error("创建群" + groupName + "时发生异常");
            buffer = new DataBuffer();
            buffer.writeInt(userId); // 用户Id
            buffer.writeInt(1); // 结果,0成功
            buffer.writeInt(0); // 群id
            buffer.writeString(groupName); // 群名字
            buffer.writeIntArray(new int[0]); // 群成员
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 加入群
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param changeType 0 :join 1: quit
     * 
     * @param userIds
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer joinOrQuitGroup(int requestUserId, int groupId,
            int changeType, int[] userIds, DataBuffer attachment, int version) {

        if (changeType == 0) {
            return this.joinGroup(requestUserId, groupId, userIds, attachment,
                    version);
        } else {
            return this.quitGroup(requestUserId, groupId, userIds, attachment,
                    version);
        }
    }

    /*
     * 
     * @Description: 加入群
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param joinUserIds
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer joinGroup(int requestUserId, int groupId,
            int[] joinUserIds, DataBuffer attachment, int version) {

        DataBuffer buffer = null;
        // 数据校验
        joinUserIds = (joinUserIds == null) ? new int[0] : joinUserIds;
        logger.info("join group requestUserId: " + requestUserId
                + ", groupId = " + groupId + ", joinUserIds = "
                + Arrays.toString(joinUserIds));
        if (requestUserId <= 0 || groupId <= 0 || joinUserIds.length <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(requestUserId); // 用户Id
            buffer.writeInt(1); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeInt(0); // 修改类型， 加群
            buffer.writeIntArray(joinUserIds); // 加入的用户Id
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        GroupModel groupModel = GroupModel.getInstance();
        boolean isSuccess = false;
        try {

            isSuccess = groupModel.joinGroup(joinUserIds, groupId, null);
        } catch (SQLException e) {
            logger.error("1 create group error: requestUserId = "
                    + requestUserId + ", groupId = " + groupId
                    + ", joinUserIds =" + Arrays.toString(joinUserIds), e);
        }
        buffer = new DataBuffer();
        buffer.writeInt(requestUserId);
        if (isSuccess) { // 成功
            logger.info("join group requestUserId success: result=" + isSuccess
                    + ", requestUserId=" + requestUserId + ", groupId = "
                    + groupId + ", joinUserIds = "
                    + Arrays.toString(joinUserIds));
            buffer.writeInt(0); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeInt(0); // 修改类型， 加群
            buffer.writeIntArray(joinUserIds); // 加入的用户Id
        } else { // 失败
            logger.info("join group requestUserId error: result=" + isSuccess
                    + ", requestUserId=" + requestUserId + ", groupId = "
                    + groupId + ", joinUserIds = "
                    + Arrays.toString(joinUserIds));
            buffer.writeInt(1); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeInt(0); // 修改类型， 加群
            buffer.writeIntArray(joinUserIds); // 加入的用户Id
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 修改群名和头像
     * 
     * @param requestUserId
     * 
     * @param groupId
     * 
     * @param groupName
     * 
     * @param groupAvater
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer modifyGroup(int requestUserId, int groupId,
            String groupName, String groupAvater, DataBuffer attachment,
            int version) {

        logger.info("modify group requestUserId: " + requestUserId
                + ", groupId = " + groupId + ", groupName = " + groupName
                + ", groupAvater = " + groupAvater);
        DataBuffer buffer = null;
        // 数据校验
        if (requestUserId <= 0 || groupId <= 0 || groupName == null
                || groupName.equals("")) {
            buffer = new DataBuffer();
            buffer.writeInt(requestUserId); // 用户Id
            buffer.writeInt(1); // 结果,0修改成功 1数据校验失败 2修改失败
            buffer.writeInt(groupId); // 群id
            buffer.writeString(groupName); // 群名
            buffer.writeString(groupAvater); // 群头像
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        GroupModel groupModel = GroupModel.getInstance();
        boolean isSuccess = false;
        try {
            Group group = groupModel.getGroupInfo(groupId);
            if (group == null || groupId != group.getGroupId()
                    || requestUserId != group.getCreateUserId()) {
                isSuccess = false;
            } else {
                isSuccess = groupModel.modifyGroup(requestUserId, groupId,
                        groupName, groupAvater, group);
            }
        } catch (SQLException e) {
            logger.error("modify group error: requestUserId = " + requestUserId
                    + ", groupId = " + groupId + ", groupName = " + groupName
                    + ", groupAvater =" + groupAvater, e);
        }
        buffer = new DataBuffer();
        buffer.writeInt(requestUserId); // 用户Id
        if (isSuccess) { // 成功
            logger.info("modify group requestUserId success: result="
                    + isSuccess + ", requestUserId=" + requestUserId
                    + ", groupId = " + groupId + ", groupName = " + groupName
                    + ", groupAvater =" + groupAvater);
            buffer.writeInt(0); // 结果
            buffer.writeInt(groupId); // 群Id
        } else { // 失败
            logger.info("quit group requestUserId failed: result=" + isSuccess
                    + ", requestUserId=" + requestUserId + ", groupId = "
                    + groupId + ", groupName = " + groupName
                    + ", groupAvater =" + groupAvater);
            buffer.writeInt(2); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeString(groupName); // 群名
            buffer.writeString(groupAvater); // 群头像
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 退出群
     * 
     * @param requestUserId
     * 
     * @param groupId
     * 
     * @param quitUserIds
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer quitGroup(int requestUserId, int groupId,
            int[] quitUserIds, DataBuffer attachment, int version) {

        DataBuffer buffer = null;
        // 数据校验
        quitUserIds = (quitUserIds == null) ? new int[0] : quitUserIds;
        logger.info("quit group requestUserId: " + requestUserId
                + ", groupId = " + groupId + ", quitUserIds = "
                + Arrays.toString(quitUserIds));
        if (requestUserId <= 0 || groupId <= 0 || quitUserIds.length <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(requestUserId); // 用户Id
            buffer.writeInt(1); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeInt(1); // 修改类型， 退群
            buffer.writeIntArray(quitUserIds); // 退出的用户Id
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        GroupModel groupModel = GroupModel.getInstance();
        boolean isSuccess = false;
        try {
            Group group = groupModel.getGroupInfo(groupId);
            if (group == null || groupId != group.getGroupId()
                    || requestUserId != group.getCreateUserId()) {
                isSuccess = false;
            } else {
                isSuccess = groupModel.quitGroup(requestUserId, quitUserIds,
                        groupId, group);
            }
        } catch (SQLException e) {
            logger.error(
                    "1 quit group error: requestUserId = " + requestUserId
                            + ", groupId = " + groupId + ", quitUserIds ="
                            + Arrays.toString(quitUserIds), e);
        }
        buffer = new DataBuffer();
        buffer.writeInt(requestUserId); // 用户Id
        if (isSuccess) { // 成功
            logger.info("quit group requestUserId success: result=" + isSuccess
                    + ", requestUserId=" + requestUserId + ", groupId = "
                    + groupId + ", quitUserIds = "
                    + Arrays.toString(quitUserIds));
            buffer.writeInt(0); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeInt(1); // 修改类型， 退群
            buffer.writeIntArray(quitUserIds); // 退群的用户Id
        } else { // 失败
            logger.info("quit group requestUserId failed: result=" + isSuccess
                    + ", requestUserId=" + requestUserId + ", groupId = "
                    + groupId + ", quitUserIds = "
                    + Arrays.toString(quitUserIds));
            buffer.writeInt(2); // 结果
            buffer.writeInt(groupId); // 群Id
            buffer.writeInt(1); // 修改类型， 退群
            buffer.writeIntArray(quitUserIds); // 退群的用户Id
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

}
