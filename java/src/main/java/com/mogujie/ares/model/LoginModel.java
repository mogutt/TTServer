package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import com.alibaba.druid.util.StringUtils;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;
import com.mogujie.ares.util.MoguUtil;

/*
 * @Description: 登陆相关的所有操作
 * @author shuchen - shuchen[at]mogujie.com
 * @date 2014-08-04 下午3:20:01
 */
public class LoginModel {
    private static LoginModel instance = new LoginModel();
    private static final Logger logger = LoggerFactory
            .getLogger(LoginModel.class);

    public static LoginModel getInstance() {
        if (instance == null) {
            instance = new LoginModel();
        }
        return instance;
    }

    private LoginModel() {

    }

    /*
     * 
     * @Description: 登陆的验证接口，直接DB拉取已加密密码验证
     * 
     * @param mogujieSession
     * 
     * @return
     * 
     * @throws Exception
     */
    public boolean auth(String uname, String pwd) throws Exception {
        boolean isAuthed = false; // model层不做参数为空判断,会在action层做掉
        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select pwd, status from IMUsers where uname = "
                    + MoguUtil.getArgsHolder(1);
            statement = conn.prepareStatement(sql);
            statement.setString(1, uname);
            rs = statement.executeQuery();
            String passwd = "";
            int    status = 0;
            while (rs.next()) {
                passwd = rs.getString("pwd");
                status = rs.getInt("status");
                logger.info("login: " + uname);
                if (status == 0 && !StringUtils.isEmpty(passwd) && passwd.equals(pwd)) {
                    isAuthed = true;
                }
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }
        return isAuthed;
    }

}
