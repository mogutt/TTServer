package com.mogujie.ares.extend.action;

import java.sql.SQLException;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.data.GroupCounterItem;
import com.mogujie.ares.data.GroupMessage;
import com.mogujie.ares.data.Message;
import com.mogujie.ares.data.User;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.CounterModel;
import com.mogujie.ares.model.MessageModel;
import com.mogujie.ares.util.MoguUtil;

public class MessageContent extends BaseAction {

    private static final Logger logger = LoggerFactory
            .getLogger(MessageContent.class);

    /**
     * 
     * @param commandId
     * @param userId
     * @param groupId
     * @param clientType 客户端类型@see ClientType
     * @param attachment
     * @param version
     * @return
     */
    public DataBuffer getGroupUnreadMessage(int commandId, int userId,
            int groupId, int clientType, DataBuffer attachment, int version) {
        logger.info("query group unread message: fromUserId=" + userId
                + ", groupId =" + groupId + ", clientType=" + clientType);
        if (userId <= 0 || groupId <= 0) {
            DataBuffer buffer = new DataBuffer();
            buffer.writeInt(userId);
            buffer.writeInt(commandId);
            buffer.writeInt(groupId);
            buffer.writeInt(0);
            logger.info("response group unread message: fromUserId=" + userId
                    + ", groupId=" + groupId + ", count=" + 0);
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        DataBuffer buffer = new DataBuffer();
        buffer.writeInt(userId);
        buffer.writeInt(commandId);
        buffer.writeInt(groupId);

        try {
            GroupCounterItem counterItem = CounterModel.getInstance()
                    .getUserGroupCount(userId, groupId, clientType);
            int lastMsgId = counterItem.getLastMessageId();
            int count = counterItem.getUserUnreadCount(); // 这里的计数最大是
                                                          // GROUP_UNREAD_MAX_COUNTER
                                                          // = 100
            if (count > 0) {
                GroupMessage[] messages = MessageModel.getInstance()
                        .getGroupMessagesFromId(groupId, lastMsgId, count);
                logger.info("response group unread message: userId=" + userId
                        + ", groupId=" + groupId + ", count=" + messages.length);
                buffer = packageGroupMessages(buffer, messages);
            } else {
                buffer.writeInt(0);
                logger.info("response group unread message: userId=" + userId
                        + ", groupId=" + groupId + ", count=" + 0);
            }
        } catch (Exception e) {
            logger.error("userId=" + userId + ", groupId=" + groupId, e);
            buffer.writeInt(0);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 获得群历史消息
     * 
     * @param userId 请求用户ID
     * 
     * @param groupId 请求群的ID
     */
    public DataBuffer getGroupHistoryMessage(int commandId, int userId,
            int groupId, int offset, int count, DataBuffer attachment,
            int version) {
        logger.info("query group history message: fromUserId=" + userId
                + ", groupId =" + groupId + ", offset=" + offset + ", count="
                + count);
        if (userId <= 0 || groupId <= 0) {
            DataBuffer buffer = new DataBuffer();
            buffer.writeInt(userId);
            buffer.writeInt(commandId);
            buffer.writeInt(groupId);
            buffer.writeInt(0);
            logger.info("response param error group history message: fromUserId="
                    + userId + ", groupId=" + groupId + ", count=" + 0);

            return MoguUtil.writeAttachments(buffer, attachment);
        }

        DataBuffer buffer = new DataBuffer();
        buffer.writeInt(userId);
        buffer.writeInt(commandId);
        buffer.writeInt(groupId);

        try {
            if (count > 0) {
                GroupMessage[] messages = MessageModel.getInstance()
                        .getGroupMessages(groupId, offset, count);
                logger.info("response normal group history message: userId="
                        + userId + ", groupId=" + groupId + ", count="
                        + messages.length);
                buffer = packageGroupMessages(buffer, messages);
            } else {
                buffer.writeInt(0);
                logger.info("response empty group history message: userId="
                        + userId + ", groupId=" + groupId + ", count=" + 0);
            }
        } catch (Exception e) {
            logger.error("userId=" + userId + ", groupId=" + groupId, e);
            buffer.writeInt(0);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 读取未读消息, 一次读取最多50条
     * 
     * @param userId 发送者
     * 
     * @param friendUserId 接收者
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public DataBuffer getUnreadMessage(int commandId, int userId,
            int friendUserId, int clientType, DataBuffer attachment, int version) {
        logger.info("query unread message: fromUserId=" + userId
                + ", friendUserId=" + friendUserId + ", clientType=" + clientType);
        if (userId <= 0 || friendUserId <= 0) {
            DataBuffer buffer = new DataBuffer();
            buffer.writeInt(commandId);
            buffer.writeInt(userId);
            buffer.writeInt(friendUserId);
            buffer.writeInt(0);
            logger.info("response unread message: fromUserId=" + userId
                    + ", friendUserId=" + friendUserId + ", count=" + 0);

            return MoguUtil.writeAttachments(buffer, attachment);
        }

        DataBuffer buffer = new DataBuffer();
        buffer.writeInt(commandId);
        buffer.writeInt(userId);
        buffer.writeInt(friendUserId);

        try {
            int count = CounterModel.getInstance().getUserFriendUnreadCount(
                    userId, friendUserId, clientType);
            count = count > BizConstants.UNREAD_MAX_COUNTER ? BizConstants.UNREAD_MAX_COUNTER
                    : count; // 一次读取不超过500条
            if (count > 0) {
                Message[] messages = MessageModel.getInstance().getMessages(
                        friendUserId, userId, 0, count);
                logger.info("response unread message: userId=" + userId
                        + ", friendUserId=" + friendUserId + ", count="
                        + messages.length);
                buffer = packageMessages(buffer, messages);
            } else {
                buffer.writeInt(0);
                logger.info("response unread message: userId=" + userId
                        + ", friendUserId=" + friendUserId + ", count=" + 0);
            }
        } catch (SQLException e) {
            logger.error("userId=" + userId + ", friendUserId=" + friendUserId,
                    e);
            buffer.writeInt(0);
        }
        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /**
     * 读取历史消息,一次读取最多20条
     * 
     * @param userId
     *            发送者
     * @param friendUserId
     *            接收者
     * @param offset
     * @param count 条数
     * @param clientType 客户端类型@see ClientType
     * @return
     */
    public DataBuffer getHistoryMessage(int commandId, int userId,
            int friendUserId, int offset, int count, int clientType, DataBuffer attachment,
            int version) {
        logger.info("query history message: userId=" + userId
                + ", friendUserId=" + friendUserId + ", offset=" + offset
                + ", count=" + count + ", clientType=" + clientType);
        if (userId <= 0 || friendUserId < 0 || offset < 0 || count <= 0) {
            logger.info("获取历史消息数据校验出错from:" + userId + ", to: " + friendUserId
                    + ", offset: " + offset + ", count: " + count);
            DataBuffer dataBuffer = new DataBuffer();
            dataBuffer.writeInt(commandId);
            dataBuffer.writeInt(userId);
            dataBuffer.writeInt(friendUserId);
            dataBuffer.writeInt(0);
            dataBuffer.writeInt(attachment.readableBytes());
            if (attachment.readableBytes() > 0) {
                dataBuffer.writeDataBuffer(attachment);
            }
            return dataBuffer;
        }
        count = count > 50 ? 50 : count; // 一次读取不超过50条

        DataBuffer dataBuffer = new DataBuffer();
        dataBuffer.writeInt(commandId);
        dataBuffer.writeInt(userId);
        dataBuffer.writeInt(friendUserId);

        try {
            Message[] messages = MessageModel.getInstance().getDialogMessages(
                    userId, friendUserId, offset, count);
            dataBuffer = packageMessages(dataBuffer, messages);
        } catch (SQLException e) {
            logger.error("userId=" + userId + ", friendUserId=" + friendUserId,
                    e);
            dataBuffer.writeInt(0);
        }
        dataBuffer.writeInt(attachment.readableBytes());
        if (attachment.readableBytes() > 0) {
            dataBuffer.writeDataBuffer(attachment);
        }

        return dataBuffer;
    }

    /*
     * 发送消息(点对点、群)
     * 
     * @param requestId 请求的唯一id
     * 
     * @param fromUserId 消息发送者
     * 
     * @param toId 消息接收者, 发送的目的地，messageType = 1时,这个id是userId， messageType =
     * 2时，这个id是groupId
     * 
     * @param created 消息创建时间
     * 
     * @param messageType 消息类型
     * 
     * @param content 消息内容
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public DataBuffer sendMessage(int requestType, int requestId,
            int fromUserId, int toId, int created, byte biteMessageType,
            byte[] content, int clientType, String attach, int version) {
        logger.info("发送消息 - requestId=" + requestId + ", fromUserId="
                + fromUserId + ", toUserId=" + toId + ", created=" + created
                + ", content=*" + ", clientType=" + clientType + ", attach=" + attach);
        MessageModel msgModel = MessageModel.getInstance();
        int messageType = biteMessageType;
        DataBuffer dataBuffer = new DataBuffer();
        if (fromUserId <= 0 || toId <= 0 || messageType <= 0 || content == null
                || content.length <= 0 ||
                ( (biteMessageType == 1 || biteMessageType == 17) && content.length > 1536) || // 一般的文本消息
        		( (biteMessageType == 2 || biteMessageType == 18) && content.length > 102400)) {
            logger.info("消息数据校验不通过 - from:" + fromUserId + ", to: " + toId
                    + ", messageType: " + messageType + ", content: *");
            dataBuffer.writeInt(requestId);
            dataBuffer.writeInt(1);
            dataBuffer.writeChar((char) requestType);
            dataBuffer.writeInt(fromUserId);
            dataBuffer.writeInt(toId);
            return dataBuffer;
        }

        boolean isSuccess = false;
        boolean isGroupMessage = false;
        try {
            // 内容为空的校验在MessageServer就做掉，直接发送失败
            int msgType = biteMessageType;
            if (msgType == BizConstants.MESSAGE_TYPE_IM ) {
            	String strContent = new String(content);
                isSuccess = msgModel.sendIMMessage(fromUserId, toId, msgType,strContent, created);
			} else if(msgType == BizConstants.MESSAGE_TYPE_IM_AUDIO){
				isSuccess = msgModel.sendAudioMessage(fromUserId, toId,content,created);
			}else if(msgType == BizConstants.MESSAGE_TYPE_IM_GROUP) {
				String strContent = new String(content);
				isSuccess = msgModel.sendIMGroupMessage(fromUserId, toId, strContent, created,clientType,msgType);
            }else if( msgType == BizConstants.MESSAGE_TYPE_IM_GROUP_AUDIO){
				isSuccess = msgModel.sendGroupAudioMessage(fromUserId, toId,
						content,created,clientType,msgType);
            }
            
            if (!isSuccess) { // 发送失败，记一把日志
                logger.error("发送消息失败 - fromUserId=" + fromUserId
                        + ", toUserId=" + toId + ", created=" + created
                        + ", messageType=" + messageType + ", content=*"
                        + ", created=" + created);
            } else {
            	if(!isGroupMessage) {
            		DelayUpdateMonitor.getInstance().AddPersonalUpdate(fromUserId, toId, created);
            	} else {
            		DelayUpdateMonitor.getInstance().AddGroupUpdate(toId, created);
            	}
                logger.info("发送消息成功 - from:" + fromUserId + ", to: " + toId
                        + ", messageType: " + messageType + ", content: *");
            }
        } catch (SQLException e) {
            logger.error("发送消息失败 - fromUserId=" + fromUserId + ", toUserId="
                    + toId + ", created=" + created + ", messageType="
                    + messageType + ", content=*" + ", created=" + created, e);
        }
        dataBuffer.writeInt(requestId);
        int result = (isSuccess ? 0 : 1);
        dataBuffer.writeInt(result);
        dataBuffer.writeChar((char) requestType);
        dataBuffer.writeInt(fromUserId);
        dataBuffer.writeInt(toId);
        return dataBuffer;
    }

    /**
     * 
     * @Description: 打包一坨消息到数据包，很多接口都需要，所以抽出来做一个方法
     *               传入前请确定message和dataBuffer不是null，否则报错
     * @param dataBuffer
     * @param messages
     * @return
     */
    private DataBuffer packageMessages(DataBuffer dataBuffer, Message[] messages) {
        int msgCount = messages.length;
        dataBuffer.writeInt(msgCount);
        for (int i = 0; i < msgCount; i++) {
            dataBuffer = packageMessage(dataBuffer, messages[i]);
        }
        return dataBuffer;
    }

    /**
     * 
     * @Description: 打包一条消息到数据包，很多接口都需要，所以抽出来做一个方法
     *               传入前请确定message和dataBuffer不是null，否则报错
     * @param dataBuffer
     * @param messages
     * @return
     */
    private DataBuffer packageMessage(DataBuffer dataBuffer, Message message) {
        dataBuffer.writeInt(message.getFromUserId());
        User user = message.getFromUser();
        if (user != null) {
            dataBuffer.writeString(user.getUname());
            dataBuffer.writeString(user.getUnick());
            dataBuffer.writeString(user.getAvatar());
        } else {
            dataBuffer.writeString("");
            dataBuffer.writeString("");
            dataBuffer.writeString(""); // TODO 默认头像
        }
        dataBuffer.writeInt(message.getCreated());
        dataBuffer.writeByte(message.getType());
        if(message.getType()==BizConstants.MESSAGE_TYPE_IM_AUDIO){
        	 dataBuffer.writeByteArray(MessageModel.getInstance().getAudioMessageContent(message.getAudio()));
        }else{
        	 dataBuffer.writeString(message.getContent());
        }
       
        return dataBuffer;
    }

    /**
     * 
     * @Description:
     * @param dataBuffer
     * @param messages
     * @return
     */
    private DataBuffer packageGroupMessages(DataBuffer dataBuffer,
            GroupMessage[] groupMessages) {
        int msgCount = groupMessages.length;
        dataBuffer.writeInt(msgCount);
        for (int i = 0; i < msgCount; i++) {
            dataBuffer = packageGroupMessage(dataBuffer, groupMessages[i]);
        }
        return dataBuffer;
    }

    /*
     * 
     * @Description: 打包一条消息到数据包，很多接口都需要，所以抽出来做一个方法
     * 传入前请确定message和dataBuffer不是null，否则报错
     * 
     * @param dataBuffer
     * 
     * @param messages
     * 
     * @return
     */
    private DataBuffer packageGroupMessage(DataBuffer dataBuffer,
            GroupMessage groupMessage) {
        dataBuffer.writeInt(groupMessage.getUserId());
        dataBuffer.writeInt(groupMessage.getCreated());
        dataBuffer.writeByte(groupMessage.getMessageType());
		if(groupMessage.getMessageType()==BizConstants.MESSAGE_TYPE_IM_GROUP_AUDIO){
			dataBuffer.writeByteArray(MessageModel.getInstance().getAudioMessageContent(groupMessage.getAudio()));
		}else{
			dataBuffer.writeString(groupMessage.getContent());
		}

        return dataBuffer;
    }
}
