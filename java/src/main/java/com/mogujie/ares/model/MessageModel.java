package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.data.Audio;
import com.mogujie.ares.data.GroupMessage;
import com.mogujie.ares.data.Message;
import com.mogujie.ares.data.User;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;
import com.mogujie.ares.util.MoguUtil;

/*
 * 
 * @Description: 消息的相关的操作
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-22 下午2:23:09
 * 
 */
public class MessageModel {

    private static final Logger logger = LoggerFactory
            .getLogger(MessageModel.class);

    private static MessageModel instance = new MessageModel();

    private static Map<Integer, Integer> messageTypeMap = new HashMap<Integer, Integer>();

    public static MessageModel getInstance() {
        if (instance == null) {
            instance = new MessageModel();
        }
        return instance;
    }

    private MessageModel() {
        MessageModel.messageTypeMap = new HashMap<Integer, Integer>();
        MessageModel.messageTypeMap.put(BizConstants.MESSAGE_TYPE_IM,
                BizConstants.MESSAGE_TYPE_IM);
    }

    /**
	 *
	 * @Description: 发送一段语音消息
	 * @param fromUserId
	 * @param toUserId
	 * @param byteContent
	 * @param time
	 * @param isSpam
	 * @return
	 * @throws SQLException
	 */
	public boolean sendAudioMessage(int fromUserId, int toUserId, byte[] byteContent, int time) throws SQLException {
		boolean isSuccess = false;
		if(fromUserId <= 0 || toUserId <= 0 || 
				null == byteContent || byteContent.length <= 0) {
			return isSuccess;
		}
		logger.info("发送语音:" + fromUserId + " -> " + toUserId + ": " + byteContent.length);
		int type = BizConstants.MESSAGE_TYPE_IM_AUDIO; // 语音消息
		
		AudioModel audioModel = AudioModel.getInstance();
		Audio audio = audioModel.parseAudio(byteContent);
		if(audio != null) {
			audio = audioModel.saveAudio(fromUserId, toUserId, audio, time);
			logger.info("发送语音:" + fromUserId + " -> " + toUserId + ", time = " + audio.getCostTime() + ", contentLength=" + audio.getFileSize());
			if(audio != null) { // 保存文件成功
				// ziye 这里做消息推送，不用显示调用
				sendIMMessage(fromUserId, toUserId, type, String.valueOf(audio.getId()), time);
				isSuccess = true;
			}
		} else {
			logger.error("语音文件有问题：" + fromUserId + " -> " + toUserId + ": " + byteContent.length);
		}
		
		return isSuccess;
	}
	
	public boolean sendGroupAudioMessage(int fromUserId, int groupId, byte[] byteContent, int time,int clientType,int messageType) throws SQLException {
		boolean isSuccess = false;
		if(fromUserId <= 0 || groupId <= 0 || 
				null == byteContent || byteContent.length <= 0) {
			return isSuccess;
		}
		logger.info("发送群语音:" + fromUserId + " -> " + groupId + ": " + byteContent.length);
		AudioModel audioModel = AudioModel.getInstance();
		Audio audio = audioModel.parseAudio(byteContent);
		if(audio != null) {
			audio = audioModel.saveAudio(fromUserId, groupId, audio, time);
			logger.info("发送语音:" + fromUserId + " -> " + groupId + ", time = " + audio.getCostTime() + ", contentLength=" + audio.getFileSize());
			if(audio != null) { // 保存文件成功
				sendIMGroupMessage(fromUserId, groupId, String.valueOf(audio.getId()), time, clientType, messageType);
				isSuccess = true;
			}
		} else {
			logger.error("语音文件有问题：" + fromUserId + " -> " + groupId + ": " + byteContent.length);
		}
		
		return isSuccess;
	}
	
    /*
     * @Description: 根据两个人之间的关系Id获取对话消息,双向数据
     * 
     * @param relateId
     * 
     * @param offset
     * 
     * @param count
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Message[] getDialogMessages(int relateId, int offset, int count)
            throws SQLException {
        if (relateId <= 0 || offset < 0 || count <= 0) {
            return new Message[0];
        }

        List<Message> messageList = new ArrayList<Message>();

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMMessage where relateId = ? and "
                    + "status = 0 order by created desc, id desc limit ?, ?";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, relateId);
            statement.setObject(index++, offset);
            statement.setObject(index++, count);
            rs = statement.executeQuery();

            int msgFromUserId;
            int msgToUserId;
            Message message;
            while (rs.next()) {
                message = new Message();
                message.setId(rs.getInt("id"));
                message.setRelateId(rs.getInt("relateId"));
                msgFromUserId = rs.getInt("fromUserId");
                message.setFromUserId(msgFromUserId);
                msgToUserId = rs.getInt("toUserId");
                message.setToUserId(msgToUserId);
                message.setType(rs.getInt("type"));
                message.setContent(rs.getString("content")); // 这里需要将图片等特殊占位符转译一下
                message.setIsDeleted(rs.getInt("status"));
                message.setCreated(rs.getInt("created"));
                message.setUpdated(rs.getInt("updated"));
                messageList.add(message);
            }
            // 填充用户信息
            messageList = fillUserInfoIntoMessage(messageList);
            messageList = fillAudioData(messageList);
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        Message[] message = new Message[messageList.size()];
        messageList.toArray(message);

        return message;
    }

    /*
     * @Description: 根据发送者和接收者的用户Id获取对话消息,双向数据
     * 
     * @param relateId
     * 
     * @param offset
     * 
     * @param count
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Message[] getDialogMessages(int fromUserId, int toUserId,
            int offset, int count) throws SQLException {
        if (fromUserId <= 0 || toUserId <= 0 || offset < 0 || count <= 0) {
            return new Message[0];
        }
        // 取得两个人的关系Id
        int relateId = RelationshipModel.getInstance().getRelateId(fromUserId,
                toUserId);
        if (relateId == -1) {
            return new Message[0];
        }

        return getDialogMessages(relateId, offset, count);
    }

    /*
     * @Description: 根据用户ID和其对话的用户ID删除用户(前者)的所有已读对话消息,单向数据
     * 
     * @param userId 用户ID
     * 
     * @param friendUserId 与用户对话的ID
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return Boolean
     * 
     * @throws SQLException
     */
    public Boolean deleteUserReadedDialogMessages(int userId, int friendUserId, int clientType)
            throws SQLException {
        if (userId <= 0 || friendUserId <= 0) {
            return false;
        }
        // 取得两个人的关系Id
        int relateId = RelationshipModel.getInstance().getRelateId(userId,
                friendUserId);
        if (relateId == -1) {
            return false;
        }

        // 获取两个人未读消息条数
        int fromCount = CounterModel.getInstance().getUserFriendUnreadCount(
                userId, friendUserId, clientType);
        // int ToCount =
        // CounterModel.getInstance().getUserFriendUnreadCount(friendUserId,
        // userId);
        // 设置要删除的已读消息的起始点
        // int offset = fromCount + ToCount + 10 ;
        // //删除两者未读消息条数＋10条之前的已读消息；10是个缓冲值，避免删除新到的消息
        int offset = fromCount + 3;
        return deleteDialogMessages(relateId, userId, offset, 50); // 一次最多删除50条
    }

    /*
     * @Description: 根据要删除消息的用户ID和其与另一用户对话之间的关系ID来删除该用户的已读对话消息,单向数据
     * 
     * @param userId 要删除消息的用户ID，只删除发给该用户的且该用户已读的消息
     * 
     * @param relateId 用户与某个对象之间对话的关系ID
     * 
     * @param offset 消息删除的起始点
     * 
     * @param count 一次删除的消息条数
     * 
     * @return Boolean
     * 
     * @throws SQLException
     */
    public Boolean deleteDialogMessages(int relateId, int userId, int offset,
            int count) throws SQLException {
        if (userId <= 0 || relateId <= 0 || offset < 0 || count <= 0) {
            return false;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = null;
        PreparedStatement statement = null;
        ResultSet rs = null;
        List<Integer> toDeleteMIds = new ArrayList<Integer>();
        int deleteCount = 0;
        try {
            conn = dbManager.getConnection(DBPoolName.macim_slave);
            // 查找指定便宜位置指定数量的消息ID
            String sql = "select id from IMMessage where relateId = ? and status = 0 and toUserId = ? order by created desc, id desc limit ?, ?";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setInt(index++, relateId);
            statement.setInt(index++, userId);
            statement.setInt(index++, offset);
            statement.setInt(index++, count);
            rs = statement.executeQuery();
            while (rs.next()) {
                toDeleteMIds.add(rs.getInt("id"));
            }
            rs.close();

            // 删除指定的信息
            int deleteSize = toDeleteMIds.size();
            int mid = 0;
            if (deleteSize > 0) {
                // 删除指定IDs的所有消息
                String sqlDeleteRelation = "update IMMessage set content = '*', status = 1 where id in (";
                for (int i = 0; i < deleteSize; i++) {
                    sqlDeleteRelation += "?,";
                }
                sqlDeleteRelation = sqlDeleteRelation.substring(0,
                        sqlDeleteRelation.length() - 1) + ")";
                statement = conn.prepareStatement(sqlDeleteRelation);
                int indexD = 1;
                for (int i = 0; i < deleteSize; i++) {
                    mid = toDeleteMIds.get(i);
                    statement.setInt(indexD++, mid);
                }
                deleteCount = statement.executeUpdate();
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        if (deleteCount > 0) { // 成功
            return true;
        }
        return false;
    }

    /*
     * @Description: 获取单向数据，取未读消息用的,这里只取单向的，所以不用relateId
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param offset
     * 
     * @param count
     * 
     * @return
     * 
     * @throws SQLException
     */
    public Message[] getMessages(int fromUserId, int toUserId, int offset,
            int count) throws SQLException {
        if (fromUserId <= 0 || toUserId <= 0 || offset < 0 || count <= 0) {
            return new Message[0];
        }

        List<Message> messageList = new ArrayList<Message>();

        // 取得两个人的关系Id
        int relateId = RelationshipModel.getInstance().getRelateId(fromUserId,
                toUserId);
        if (relateId == -1) {
            return new Message[0];
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;

        try {

            String sql = "select * from IMMessage where relateId=? and fromUserId = ? and "
                    + "toUserId = ? and status = 0 order by created desc, id desc limit ?, ?"; // and
                                                                                               // status
                                                                                               // =
                                                                                               // 0
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, relateId);
            statement.setObject(index++, fromUserId);
            statement.setObject(index++, toUserId);
            statement.setObject(index++, offset);
            statement.setObject(index++, count);
            rs = statement.executeQuery();

            Message message;
            while (rs.next()) {
                message = new Message();
                message.setId(rs.getInt("id"));
                message.setRelateId(rs.getInt("relateId"));
                message.setFromUserId(fromUserId);
                message.setToUserId(toUserId);
                message.setType(rs.getInt("type"));
                message.setContent(rs.getString("content")); // 这里需要特殊占位符转译一下
                message.setIsDeleted(rs.getInt("status"));
                message.setCreated(rs.getInt("created"));
                message.setUpdated(rs.getInt("updated"));
                messageList.add(message);
            }
            // 填充用户信息
            messageList = fillUserInfoIntoMessage(messageList);
			messageList = fillAudioData(messageList);
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        Message[] message = new Message[messageList.size()];
        messageList.toArray(message);

        return message;
    }

    /*
     * @Description: 获取群消息,离线消息用
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param offset
     * 
     * @param count
     * 
     * @return
     * 
     * @throws SQLException
     */
    public GroupMessage[] getGroupMessagesFromId(int groupId, int lastMsgId,
            int count) throws SQLException {
        if (groupId <= 0 || lastMsgId < 0 || count <= 0) {
            return new GroupMessage[0];
        }

        List<GroupMessage> messageList = new ArrayList<GroupMessage>();

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMGroupMessage where groupId = ? and id > ? "
                    + "order by created desc, id desc limit ?";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, groupId);
            statement.setObject(index++, lastMsgId);
            statement.setObject(index++, count);
            rs = statement.executeQuery();

            GroupMessage message;
            while (rs.next()) {
                message = new GroupMessage();
                message.setId(rs.getInt("id"));
                message.setGroupId(rs.getInt("groupId"));
                message.setUserId(rs.getInt("userId"));
                message.setContent(rs.getString("content")); // 这里需要特殊占位符转译一下
                message.setStatus(rs.getInt("status"));
                message.setCreated(rs.getInt("created"));
                message.setUpdated(rs.getInt("updated"));
                message.setMessageType(rs.getInt("messageType"));
                messageList.add(message);
            }
            
            messageList = fillGroupAudioData(messageList);
            
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        GroupMessage[] message = new GroupMessage[messageList.size()];
        messageList.toArray(message);

        return message;
    }

    /*
     * @Description: 获取群消息
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param offset
     * 
     * @param count
     * 
     * @return
     * 
     * @throws SQLException
     */
    public GroupMessage[] getGroupMessages(int groupId, int offset, int count)
            throws SQLException {
        if (groupId <= 0 || offset < 0 || count <= 0) {
            return new GroupMessage[0];
        }

        List<GroupMessage> messageList = new ArrayList<GroupMessage>();

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMGroupMessage where groupId = ? order by created desc, id desc limit ?, ?";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, groupId);
            statement.setObject(index++, offset);
            statement.setObject(index++, count);
            rs = statement.executeQuery();

            GroupMessage message;
            while (rs.next()) {
                message = new GroupMessage();
                message.setId(rs.getInt("id"));
                message.setGroupId(rs.getInt("groupId"));
                message.setUserId(rs.getInt("userId"));
                message.setContent(rs.getString("content")); // 这里需要特殊占位符转译一下
                message.setStatus(rs.getInt("status"));
                message.setCreated(rs.getInt("created"));
                message.setUpdated(rs.getInt("updated"));
                message.setMessageType(rs.getInt("messageType"));
                messageList.add(message);
            }
            
            messageList = fillGroupAudioData(messageList);
            
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
		
        GroupMessage[] message = new GroupMessage[messageList.size()];
        messageList.toArray(message);

        return message;
    }

    /*
     * 
     * @Description: 系统消息群发
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param content
     * 
     * @param time
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean sendMultiIMMessage(int fromUserId, int[] toUserIds,
            int type, String content, int time) throws SQLException {
        boolean isSuccess = false;

        if (fromUserId <= 0 || toUserIds.length <= 0 || null == content
                || content.length() <= 0) {
            return isSuccess;
        }
        Map<Integer, Integer> uidsMap = new HashMap<Integer, Integer>();
        RelationshipModel relationshipModel = RelationshipModel.getInstance();
        for (int i = 0; i < toUserIds.length; i++) {
            if (toUserIds[i] > 0) {
                relationshipModel.checkAndUpdateRelation(fromUserId,
                        toUserIds[i], true);
                int relateId = relationshipModel.getRelateId(fromUserId,
                        toUserIds[i], true);
                if (relateId <= 0) {
                    relateId = relationshipModel.addFriendship(fromUserId,
                            toUserIds[i]); // 没有好友关系,实时加一个
                }
                if (relateId > 0) {
                    uidsMap.put(toUserIds[i], relateId);
                }
            }
        }
        if (uidsMap.size() <= 0) {
            return isSuccess;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        int succCount = 0;
        List<Integer> userIdList = new ArrayList<Integer>();
        try {
            String insertClause = MoguUtil.getArgsHolder(uidsMap.size());
            String insertSql = "insert into IMMessage(`relateId`, `fromUserId`, `toUserId`, `content`, `type`, `created`, `updated`) "
                    + "values (";
            insertSql += insertClause + ")";
            Iterator<Integer> iter = uidsMap.keySet().iterator();
            while (iter.hasNext()) {
                userIdList.add(iter.next());
            }
            statement = conn.prepareStatement(insertSql);
            int index = 1;
            int relateId = 0;
            int uid = 0;
            int uidSize = userIdList.size();
            for (int i = 0; i < uidSize; i++) {
                uid = userIdList.get(i);
                relateId = uidsMap.get(uid);
                statement.setObject(index++, relateId); // TODO
                statement.setObject(index++, fromUserId);
                statement.setObject(index++, uid);
                statement.setObject(index++, content);
                statement.setObject(index++, type);
                statement.setObject(index++, time);
                statement.setObject(index++, time);
            }
            succCount = statement.executeUpdate();
            if (succCount > 0) {
                isSuccess = true;
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, null);
        }
        if (isSuccess) {
            // 增加计数
            int uidSize = userIdList.size();
            for (int i = 0; i < uidSize; i++) {
                incrCounterForNewMessage(fromUserId, userIdList.get(i),
                        succCount);
            }
        }
        return isSuccess;
    }

    /*
     * 
     * @Description: 发送一条IM消息,目前只有聊天记录，暂时不考虑原有业务中的私信，at消息等
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param content
     * 
     * @param time
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean sendIMMessage(int fromUserId, int toUserId, int type,
            String content, int time) throws SQLException {
        boolean isSuccess = false;

        if (fromUserId <= 0 || toUserId <= 0 || null == content
                || content.length() <= 0) {
            return isSuccess;
        }

        // 好友关系
        RelationshipModel relationshipModel = RelationshipModel.getInstance();
        relationshipModel.checkAndUpdateRelation(fromUserId, toUserId, true);
        int relateId = relationshipModel
                .getRelateId(fromUserId, toUserId, true);
        if (relateId <= 0) {
            relateId = relationshipModel.addFriendship(fromUserId, toUserId); // 没有好友关系,实时加一个
        }
        if (relateId <= 0) { // 加好友关系失败
            return false;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        int succCount = 0;
        try {
            String sql = "insert into IMMessage(`relateId`, `fromUserId`, `toUserId`, `content`, `type`, `created`, `updated`) "
                    + "values(?, ?, ?, ?, ?, ?, ?)";
            statement = conn.prepareStatement(sql);
            int index = 1;
            statement.setObject(index++, relateId);
            statement.setObject(index++, fromUserId);
            statement.setObject(index++, toUserId);
            statement.setObject(index++, content);
            statement.setObject(index++, type);
            statement.setObject(index++, time);
            statement.setObject(index++, time);
            succCount = statement.executeUpdate();
            if (succCount > 0) {
                isSuccess = true;
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, null);
        }
        if (isSuccess) {
            // 增加计数
            incrCounterForNewMessage(fromUserId, toUserId, succCount);
            writeNewMsgToCinfo(toUserId); // 加到用户消息中心里的未读聊天消息.
        }
        return isSuccess;
    }

    /*
     * 
     * @Description: 发送一条IM消息,目前只有聊天记录，暂时不考虑原有业务中的私信，at消息等
     * 
     * @param fromUserId
     * 
     * @param toGroupId
     * 
     * @param content
     * 
     * @param time
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     * 
     * @throws SQLException
     */
    public boolean sendIMGroupMessage(int userId, int toGroupId,
            String content, int time, int clientType,int messageType) throws SQLException {
        boolean isSuccess = false;

        if (userId <= 0 || toGroupId <= 0 || null == content
                || content.length() <= 0) {
            return isSuccess;
        }

        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_master);
        PreparedStatement statement = null;
        ResultSet rs = null;
        int succCount = 0;
        int lastMessageId = 0;
        try {
            GroupModel groupModel = GroupModel.getInstance();
            if (groupModel.isGroupMember(userId, toGroupId)) { // 是群成员
                String sql = "insert into IMGroupMessage(`groupId`, `userId`, `content`, `created`, `updated`,`messageType`) "
                        + "values(?, ?, ?, ?, ?,?)";
                statement = conn.prepareStatement(sql,
                        Statement.RETURN_GENERATED_KEYS);
                int index = 1;
                statement.setObject(index++, toGroupId);
                statement.setObject(index++, userId);
                statement.setObject(index++, content);
                statement.setObject(index++, time);
                statement.setObject(index++, time);
                statement.setObject(index++, messageType);
                succCount = statement.executeUpdate();
                if (succCount > 0) {
                    isSuccess = true;
                    rs = statement.getGeneratedKeys();
                    if (rs != null && rs.next()) {
                        lastMessageId = rs.getInt(1);
                    }
                }
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_master, conn, statement, rs);
        }
        // 加计数
        if (lastMessageId > 0) {
            CounterModel counterModel = CounterModel.getInstance();
            counterModel.incrGroupMsgCount(toGroupId, lastMessageId);
            counterModel.clearUserGroupCounter(userId, toGroupId, clientType); // 发消息的人不用计数+1，所以直接标记已读
            GroupModel groupModel = GroupModel.getInstance();
            groupModel.checkAndUpdateGroupRelation(userId, toGroupId, time); // 更新最近联系群，设置状态为2
                                                                             // 0退出
                                                                             // 1正常
                                                                             // 2正常但从最近联系人移除
        }

        return isSuccess;
    }

    /*
     * 
     * @Description: 填充消息列表里的，蛮多处需要返回一坨消息的列表，抽出来
     * 
     * @param messageList
     * 
     * @return
     * 
     * @throws SQLException
     */
    public List<Message> fillUserInfoIntoMessage(List<Message> messageList)
            throws SQLException {

        if (messageList == null || messageList.isEmpty()) {
            return new ArrayList<Message>();
        }
        Message msg = null;
        Set<Integer> uidSet = new HashSet<Integer>();
        for (int i = 0; i < messageList.size(); i++) {
            msg = messageList.get(i);
            uidSet.add(msg.getFromUserId());
            uidSet.add(msg.getToUserId());
        }
        if (uidSet.size() > 0) {
            Integer[] msgUids = new Integer[uidSet.size()];
            uidSet.toArray(msgUids);
            Map<Integer, User> usersMap = UserModel.getInstance().getUserInfo(
                    msgUids);
            for (int i = 0; i < messageList.size(); i++) {
                msg = messageList.get(i);
                msg.setFromUser(usersMap.get(msg.getFromUserId()));
                msg.setToUser(usersMap.get(msg.getToUserId()));
                messageList.set(i, msg);
            }
        }

        return messageList;
    }

    /*
     * 
     * @Description: 发送IM消息的增加计数
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param count
     */
    private void incrCounterForNewMessage(int fromUserId, int toUserId,
            int count) {

        CounterModel counterModel = CounterModel.getInstance();
        counterModel.incrUserMsgCount(fromUserId, toUserId);
        counterModel.increaseUserUnreadMsgCount(fromUserId, toUserId);
    }

    /*
     * 
     * @Description: 给客户端传过来的messagetype做一下约束，只能有现有的这几种类型
     * 
     * @param sourceType
     * 
     * @return
     */
    public int getMessageType(int sourceType) {
        if (MessageModel.messageTypeMap.containsKey(sourceType)) {
            return sourceType;
        }
        return -1;
    }

    /*
     * 写新消息到cinfo里 1.加到这个人的cinfo里面,key 为chatNew
     */
    private void writeNewMsgToCinfo(int toUserId) {
        CounterModel counterModel = CounterModel.getInstance();
        counterModel.increaseChatNewCount(toUserId);
    }
    
	public List<GroupMessage> fillGroupAudioData(List<GroupMessage> groupMessageList) throws SQLException{

		if(groupMessageList == null || groupMessageList.isEmpty()) {
			return new ArrayList<GroupMessage>();
		}
		int size = groupMessageList.size();
		int aid;
		List<Integer> audioIdList = new ArrayList<Integer>(); 
		String content = "";
		GroupMessage msg = null;
		Map<Integer, Integer> idMap = new HashMap<Integer, Integer>();
		for(int i = 0; i < size; i++) {
			msg = groupMessageList.get(i);
			if(msg.getMessageType() == BizConstants.MESSAGE_TYPE_IM_GROUP_AUDIO) {
				content = new String(msg.getContent());
				if(!StringUtils.isEmpty(content)) {
					try{
						aid = Integer.valueOf(content);
						audioIdList.add(aid);
						idMap.put(aid, i);
					} catch(Exception e) {
						logger.error(content, e); 
					}
				}
			}
		}
		Map<Integer, Audio> audioMap = new HashMap<Integer, Audio>();
		int aidSize = audioIdList.size();
		if(aidSize > 0) {
			int[] audioIds = new int[aidSize];
			for(int i = 0; i < aidSize; i++) {
				audioIds[i] = audioIdList.get(i);
			}
			audioMap = AudioModel.getInstance().readAudios(audioIds);
		}
		if(!audioMap.isEmpty()) {
			Audio audio = null;
			int index = -1;
			Iterator<Audio> iter = audioMap.values().iterator();
			while (iter.hasNext()) {
				audio = iter.next();
				index = idMap.get(audio.getId());
				groupMessageList.get(index).setAudio(audio);
			}
		}
		
		return groupMessageList;
	
	}
	
    /**
	 * 
	 * @Description: 填充语音内容
	 * @param messageList
	 * @return
	 * @throws SQLException
	 */
	public List<Message> fillAudioData(List<Message> messageList) throws SQLException {

		if(messageList == null || messageList.isEmpty()) {
			return new ArrayList<Message>();
		}
		int size = messageList.size();
		int aid;
		List<Integer> audioIdList = new ArrayList<Integer>(); 
		String content = "";
		Message msg = null;
		Map<Integer, Integer> idMap = new HashMap<Integer, Integer>();
		for(int i = 0; i < size; i++) {
			msg = messageList.get(i);
			if(msg.getType() == BizConstants.MESSAGE_TYPE_IM_AUDIO) {
				content = new String(msg.getContent());
				if(!StringUtils.isEmpty(content)) {
					try{
						aid = Integer.valueOf(content);
						audioIdList.add(aid);
						idMap.put(aid, i);
					} catch(Exception e) {
						logger.error(content, e); 
					}
				}
			}
		}
		Map<Integer, Audio> audioMap = new HashMap<Integer, Audio>();
		int aidSize = audioIdList.size();
		if(aidSize > 0) {
			int[] audioIds = new int[aidSize];
			for(int i = 0; i < aidSize; i++) {
				audioIds[i] = audioIdList.get(i);
			}
			audioMap = AudioModel.getInstance().readAudios(audioIds);
		}
		if(!audioMap.isEmpty()) {
			Audio audio = null;
			int index = -1;
			Iterator<Audio> iter = audioMap.values().iterator();
			while (iter.hasNext()) {
				audio = iter.next();
				index = idMap.get(audio.getId());
//				logger.info("audio: " + audio.getId() + "->" + index);
				messageList.get(index).setAudio(audio);
			}
		}
		
		return messageList;
	}
	
	/**
	 * 
	 * @Description: 封装语音消息体
	 * @param audio
	 * @return
	 */
	public byte[] getAudioMessageContent(Audio audio) {
		return AudioModel.getInstance().getAudioMessageContent(audio);
	}
}
