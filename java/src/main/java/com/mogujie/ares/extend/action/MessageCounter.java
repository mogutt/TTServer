package com.mogujie.ares.extend.action;

import java.sql.SQLException;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import com.mogujie.ares.data.Counter;
import com.mogujie.ares.data.Group;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.CounterModel;
import com.mogujie.ares.model.GroupModel;
import com.mogujie.ares.util.MoguUtil;

public class MessageCounter extends BaseAction {

    private static final Logger logger = LoggerFactory
            .getLogger(MessageCounter.class);

    /*
     * 获得未读消息计数
     * 
     * @param userId 请求的用户ID
     * @param clientType 客户端类型@see ClientType
     */
    public DataBuffer unread(int userId, int clientType, DataBuffer attachment, int version) {
        logger.info("unread count: userId=" + userId + ", clientType=" + clientType);
        DataBuffer buffer = new DataBuffer();
        buffer.writeInt(userId);

        Counter userUnreadCount = CounterModel.getInstance().getUnreadMsgCount(
                userId, clientType);
        Map<String, Integer> userUnreadInfo = userUnreadCount.getUnreadCount();
        String unreadCounterList = "";
        if (userUnreadInfo != null && userUnreadInfo.size() > 0) {
            buffer.writeInt(userUnreadInfo.size());
            Iterator<Map.Entry<String, Integer>> iter = userUnreadInfo
                    .entrySet().iterator();
            int uid;
            int count;
            while (iter.hasNext()) {
                Map.Entry<String, Integer> entry = iter.next();
                uid = Integer.valueOf(entry.getKey());
                count = Integer.valueOf(entry.getValue());
                buffer.writeInt(uid);
                buffer.writeInt(count);
                unreadCounterList += uid + "=" + count + ", ";
            }
        } else {
            buffer.writeInt(0);
        }
        logger.info("return unread count: userId = " + userId + " - "
                + unreadCounterList);

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * @auther ziye
     * 
     * @Description: 清理消息计数，已读
     * 
     * @param commandId
     * 
     * @param requestId
     * 
     * @param userId
     * 
     * @param friendUserId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public DataBuffer clear(int commandId, int requestId, int userId,
            int friendUserId, int clientType, int version) {
        logger.info("clear counter: requestId=" + requestId + ", userId=" + userId 
        		+ ", friendUserId=" + friendUserId + ", clientType=" + clientType);

        int result = 0;
        if (!CounterModel.getInstance().clearUserUnreadItemCount(userId,
                friendUserId, clientType)) {
            result = 1;
        }
        
        /*暂时去除阅后即焚的功能
        try {
            MessageModel.getInstance().deleteUserReadedDialogMessages(userId,
                    friendUserId, clientType);
        } catch (SQLException e) {
            logger.error("", e);
        }*/

        DataBuffer responseBuffer = new DataBuffer();
        responseBuffer.writeInt(requestId);
        responseBuffer.writeInt(result);
        responseBuffer.writeChar((char) commandId);
        responseBuffer.writeInt(userId);
        responseBuffer.writeInt(friendUserId);

        return responseBuffer;
    }

    /*
     * 
     * @Description: 获取群未读消息
     * 
     * @param userId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @param attachment
     * 
     * @param version
     * 
     * @return
     */
    public DataBuffer groupUnread(int userId, int clientType, DataBuffer attachment, int version) {
        logger.info("group unread count: userId=" + userId + ", clientType=" + clientType);
        DataBuffer buffer = new DataBuffer();
        buffer.writeInt(userId);

        String values = "";
        try {
            List<Group> groups = GroupModel.getInstance().getGroupsByUserId(
                    userId, false);
            if (groups != null && !groups.isEmpty()) {
                int groupCount = groups.size();
                int[] groupIds = new int[groupCount];
                Group groupInfo = null;
                for (int i = 0; i < groupCount; i++) {
                    groupInfo = groups.get(i);
                    if (groupInfo != null && groupInfo.getGroupId() > 0) {
                        groupIds[i] = groupInfo.getGroupId();
                    }
                }
                Map<Integer, Integer> userUnreadCount = CounterModel
                        .getInstance()
                        .getUserGroupUnreadCount(userId, groupIds, clientType);
                if (userUnreadCount != null && !userUnreadCount.isEmpty()) {
                    values = userUnreadCount.toString();
                    DataBuffer tempDataBuffer = new DataBuffer();
                    Iterator<Entry<Integer, Integer>> it = userUnreadCount
                            .entrySet().iterator();
                    int groupId = 0;
                    int count = 0;
                    int num = 0;
                    Entry<Integer, Integer> entry = null;
                    while (it.hasNext()) {
                        entry = it.next();
                        groupId = entry.getKey();
                        count = entry.getValue();
                        if (groupId > 0 && count > 0) {
                            tempDataBuffer.writeInt(groupId);
                            tempDataBuffer.writeInt(count);
                            num++;
                        }
                    }
                    buffer.writeInt(num);
                    if (num > 0) {
                        buffer.writeDataBuffer(tempDataBuffer);
                    }
                } else {
                    buffer.writeInt(0);
                }
            } else {
                buffer.writeInt(0);
            }
        } catch (SQLException e) {
            logger.error("group unread, userId" + userId, e);
            buffer.writeInt(0);
        }

        logger.info("return group unread count: userId = " + userId + " - "
                + values);

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * @auther ziye
     * 
     * @Description: 清除用户在这个群的未读消息计数，全部置为已读
     * 
     * @param commandId
     * 
     * @param requestId
     * 
     * @param userId
     * 
     * @param friendUserId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public DataBuffer clearUserGroup(int commandId, int userId, int groupId,
            int clientType, int version) {
        logger.info("clear counter: userId=" + userId + ", groupId=" + groupId 
        		+ ", clientType=" + clientType);

        int result = 0;
        if (!CounterModel.getInstance().clearUserGroupCounter(userId, groupId, clientType)) {
            result = 1;
        }

        DataBuffer buffer = new DataBuffer();
        buffer.writeInt(0);
        buffer.writeInt(result);
        buffer.writeChar((char) commandId);
        buffer.writeInt(userId);
        buffer.writeInt(groupId);

        return buffer;
    }

}
