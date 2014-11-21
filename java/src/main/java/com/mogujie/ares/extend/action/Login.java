package com.mogujie.ares.extend.action;

import com.alibaba.druid.util.StringUtils;
import com.mogujie.ares.data.User;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.LoginModel;
import com.mogujie.ares.model.UserModel;
import com.mogujie.ares.util.MoguUtil;

/*
 * @Description: 用户登陆相关的请求
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午1:28:17
 */
public class Login extends BaseAction {

    private static final Logger logger = LoggerFactory.getLogger(Login.class);

    /*
     * @Description: 用户登陆
     * 
     * @param userId 用户Id
     * 
     * @param token 登陆用户请求蘑菇街主站的http://www.mogujie.com/aresapi/login接口来获得
     * 
     * @return
     */
    public DataBuffer login(String uname, String pwd, DataBuffer attachment,
            int version) {
        logger.info("login: " + uname);
        int resultCode = 0;
        boolean isAuthed = false;
        User user = null;
        if (StringUtils.isEmpty(uname)) {
            resultCode = 1;
        }

        UserModel userModel = UserModel.getInstance();
        try {
            isAuthed = LoginModel.getInstance().auth(uname, pwd);
            if (isAuthed) {
                user = userModel.getUserInfo(uname);
            } else {
                logger.info("login failed with error password!");
            }
        } catch (Exception e) {
            logger.error("login failed with reason : ", e);
            isAuthed = false;
        }
        isAuthed = (isAuthed && (user != null)); // 用户要存在
        resultCode = (isAuthed ? 0 : 1); // 0: 成功，1: 失败
        DataBuffer buffer = new DataBuffer();
        buffer.writeString(uname);// 用户名
        buffer.writeInt(resultCode);
        if (isAuthed) {
            buffer.writeInt(user.getUserId()); // 用户ID
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
            logger.info("login success: " + uname + ", " + resultCode);
        } else {
            logger.info("login error: " + uname + ", " + resultCode);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

}
