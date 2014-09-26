package com.mogujie.ares.extend.action;

import java.sql.SQLException;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import com.mogujie.ares.data.User;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.UserModel;
import com.mogujie.ares.util.MoguUtil;

/**
 * 
 * @Description: 用户相关的类
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-8-12 下午4:55:45
 * 
 */
public class UserAction extends BaseAction {

    private static final Logger logger = LoggerFactory
            .getLogger(UserAction.class);

    /**
     * 
     * @Description: 获取用户信息
     * @param fromUserId
     * @param userIds
     * @return
     */
    public DataBuffer getUsersInfo(int fromUserId, int[] userIds,
            DataBuffer attachment, int version) {
        logger.info("get users info : " + Arrays.toString(userIds));
        DataBuffer buffer;
        if (userIds == null || userIds.length <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId);
            buffer.writeInt(0);
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        try {
            Map<Integer, User> users = UserModel.getInstance().getUserInfo(
                    userIds);
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId); // 发请求的用户
            buffer.writeInt(users.size()); // 查询到详细信息的用户数
            User user;
            Iterator<Integer> it = users.keySet().iterator();
            String logText = "response userInfo: ";
            while (it.hasNext()) {
                user = users.get(it.next());
                buffer.writeInt(user.getUserId()); // 用户ID
                buffer.writeString(user.getUname()); // 用户名
                buffer.writeString(user.getUnick()); // 用户昵称
                buffer.writeString(user.getAvatar()); // 用户头像
                buffer.writeString(user.getTitle()); // 用户职称
                buffer.writeString(user.getPosition()); // 用户地址
                buffer.writeInt(user.getStatus()); // 用户在职等状态
                buffer.writeInt(user.getSex()); // 用户性别
                buffer.writeInt(user.getDepartId()); // 用户所在部门ID
                buffer.writeInt(user.getJobNumber()); // 用户工号
                buffer.writeString(user.getTelphone()); // 用户电话
                buffer.writeString(user.getMail()); // 用户邮箱
                logText += "userId=" + user.getUserId() + ", ";
            }
            logger.info(logText);
        } catch (SQLException e) {
            logger.error("get users info error with reason : ", e);
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId);
            buffer.writeInt(0);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    /**
     * 
     * @Description: 获取所有用户信息
     * @param fromUserId
     * @return
     */
    public DataBuffer getAllUsersInfo(int fromUserId, DataBuffer attachment,
            int version) {
        logger.info("get all users info by : " + fromUserId);
        DataBuffer buffer;
        if (fromUserId <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId);
            buffer.writeInt(0);
            buffer.writeInt(attachment.readableBytes());
            if (attachment.readableBytes() > 0) {
                buffer.writeDataBuffer(attachment);
            }
            return buffer;
        }

        try {
            Set<User> users = UserModel.getInstance().getAllUserInfo();
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId); // 发请求的用户
            buffer.writeInt(users.size()); // 查询到详细信息的用户数
            User user;
            Iterator<User> it = users.iterator();
            while (it.hasNext()) {
                user = it.next();
                buffer.writeInt(user.getUserId()); // 用户ID
                buffer.writeString(user.getUname()); // 用户名
                buffer.writeString(user.getUnick()); // 用户昵称
                buffer.writeString(user.getAvatar()); // 用户头像
                buffer.writeString(user.getTitle()); // 用户职称
                buffer.writeString(user.getPosition()); // 用户地址
                buffer.writeInt(user.getStatus()); // 用户在职等状态
                buffer.writeInt(user.getSex()); // 用户性别
                buffer.writeInt(user.getDepartId()); // 用户所在部门ID
                buffer.writeInt(user.getJobNumber()); // 用户工号
                buffer.writeString(user.getTelphone()); // 用户电话
                buffer.writeString(user.getMail()); // 用户邮箱
            }
        } catch (SQLException e) {
            logger.error("get all user info error with reason", e);
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId);
            buffer.writeInt(0);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    // /*
    // * @Description: 修改用户头像
    // * @param userId
    // * 用户Id
    // * @param avatar
    // * 用户头像url
    // * @return
    // */
    // public DataBuffer alterAvatar(int userId, String avatar,
    // DataBuffer attachment, int version) {
    // // logger.info("login: " + userId + ", new avatar: " + avatar);
    // int resultCode = 0;
    // // 数据校验扔到model层,这里不需要判断
    // try {
    // resultCode = UserModel.getInstance().alterAvatar(userId, avatar);
    // } catch (Exception e) {
    // resultCode = 1;
    // logger.error("error: update user's avatar failed! userId: "
    // + userId + " avatar: " + avatar + e.toString());
    // }
    // DataBuffer buffer = new DataBuffer();
    // buffer.writeInt(userId);
    // buffer.writeInt(resultCode);
    //
    // return MoguUtil.writeAttachments(buffer, attachment);
    // }

}
