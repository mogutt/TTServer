package com.mogujie.ares.extend.action;

import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.data.Group;
import com.mogujie.ares.data.Relationship;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.GroupModel;
import com.mogujie.ares.model.RelationshipModel;
import com.mogujie.ares.util.MoguUtil;

/*
 * @Description: 好友相关的请求
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午1:27:09
 */
public class Friendship extends BaseAction {

    private static final Logger logger = LoggerFactory
            .getLogger(Friendship.class);
    //private static User serviceUser; // 服务号小T
    private static Map<Integer, Integer> mapFilterUserIds = new HashMap<Integer, Integer>();// 要过滤的用户id

    public Friendship() {
//        try {
//            serviceUser = UserModel.getInstance().getServerUserInfo();
//            if (null == serviceUser) {
//                throw new IllegalArgumentException(
//                        "can't initialize server user info");
//            }
//            mapFilterUserIds.put(serviceUser.getUserId(), 1);
//        } catch (SQLException e) {
//            logger.error("initialize server user info error with reason : ", e);
//        }
    }

    /*
     * 
     * @Description: 获取用户的最近联系人
     * 
     * @param userId 用户id
     * 
     * @return
     */
    public DataBuffer getUserRecentContact(int userId, DataBuffer attachment,
            int version) {
        DataBuffer buffer = null;

        if (userId <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(userId);
            buffer.writeInt(0);
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        DataBuffer tmpBuffer = new DataBuffer();
        buffer = new DataBuffer();
        buffer.writeInt(userId);
        int count = 0;
        StringBuffer logBuffer = new StringBuffer();

        try {
            logger.info("get recent contact list by " + userId);
            // 取好友列表
            Relationship[] friendshipArray = RelationshipModel.getInstance()
                    .getRecentContactByUserId(userId, 100);

//            if (serviceUser != null) {
//                mapFilterUserIds.put(serviceUser.getUserId(), 1);
//                tmpBuffer.writeInt(serviceUser.getUserId());
//                tmpBuffer.writeInt(1767200461); // 2026-01-01 服务号默认置顶
//                count++;
//            }
            int fuid;
            if (friendshipArray != null && friendshipArray.length > 0) {
                for (int i = 0; i < friendshipArray.length; i++) {
                    fuid = friendshipArray[i].getFriendUserId();
                    if (mapFilterUserIds.containsKey(fuid)) {
                        continue;
                    }
                    tmpBuffer.writeInt(fuid);
                    tmpBuffer.writeInt(friendshipArray[i].getUpdated());
                    logBuffer.append(", " + fuid + "-updateTime = " + friendshipArray[i].getUpdated() ); // log
                    count++;
                }
            }
        } catch (SQLException e) {
            logger.error("get recent contact list error with reason : ", e);
        }
        logger.info("get recent contact list: userId=" + userId + ", friends:"
                + logBuffer.toString());
        if (tmpBuffer != null && tmpBuffer.readableBytes() > 0) {
            buffer.writeInt(count);
            buffer.writeDataBuffer(tmpBuffer);
        } else {
            buffer.writeInt(0);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /*
     * 
     * @Description: 删除用户的最近联系人或群
     * 
     * @param userId 用户id
     * 
     * @param friendUserId 最近联系ID
     * 
     * @param friendUserType 最近联系ID类型：1用户；2群
     * 
     * @return
     */
    public DataBuffer deleteUserRecentContact(int userId, int friendUserId,
            int friendUserType, DataBuffer attachment, int version) {
        DataBuffer buffer = null;

        if (userId <= 0 || friendUserId <= 0 || friendUserType <= 0
                || BizConstants.SYS_SERVER_USER_ID == friendUserId) {
            buffer = new DataBuffer();
            buffer.writeInt(userId);
            buffer.writeInt(0);
            buffer.writeInt(friendUserId);
            buffer.writeInt(friendUserType);
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        boolean isSuccess = false;
        try {
            if (1 == friendUserType) {
                isSuccess = RelationshipModel.getInstance()
                        .deleteRecentContactByUserId(userId, friendUserId);
            } else {
                Group group = GroupModel.getInstance().getGroupInfo(
                        friendUserId);
                // 如果群不存在则删除失败，群主也能将自己建的群从最近联系群中删除
                if (group == null || friendUserId != group.getGroupId()) {
                    isSuccess = false;
                } else {
                    isSuccess = GroupModel.getInstance()
                            .deleteRecentContactByGroupId(userId, friendUserId,
                                    group);
                }
            }
        } catch (SQLException e) {
            logger.error("delete recent contact error: userId = " + userId
                    + ", friendUserId = " + friendUserId + ", friendUserType ="
                    + friendUserType, e);
        }

        buffer = new DataBuffer();
        buffer.writeInt(userId); // 用户Id
        if (isSuccess) { // 成功
            logger.info("delete recent contact success: result=" + isSuccess
                    + ", userId=" + userId + ", friendUserId = " + friendUserId
                    + ", friendUserType = " + friendUserType);
            buffer.writeInt(0); // 结果
            buffer.writeInt(friendUserId);
            buffer.writeInt(friendUserType);
        } else { // 失败
            logger.info("delete recent contact error: result=" + isSuccess
                    + ", userId=" + userId + ", friendUserId = " + friendUserId
                    + ", friendUserType = " + friendUserType);
            buffer.writeInt(1); // 结果
            buffer.writeInt(friendUserId);
            buffer.writeInt(friendUserType);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

}
