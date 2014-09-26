package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;

import com.mogujie.ares.data.Department;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;

/*
 * @Description: 部门相关的model
 * @author ziye
 * 
 */
public class DepartModel {

    private static DepartModel instance = new DepartModel();
    @SuppressWarnings("unused")
    private final Logger logger = LoggerFactory.getLogger(DepartModel.class);

    public static DepartModel getInstance() {
        if (instance == null) {
            instance = new DepartModel();
        }
        return instance;
    }

    /*
     * @Description: 获取所有部门信息
     * 
     * @return Map<Integer, Department> 部门的具体信息Map
     * 
     * @throws SQLException
     */
    public Map<Integer, Department> getDepartmentInfo() throws SQLException {
        Map<Integer, Department> departInfos = new HashMap<Integer, Department>();
        DBManager dbManager = DBManager.getInstance();
        Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
        PreparedStatement statement = null;
        ResultSet rs = null;
        try {
            String sql = "select * from IMDepartment where status = 0";
            statement = conn.prepareStatement(sql);
            rs = statement.executeQuery();
            Department department = null;
            int departId = 0;
            while (rs.next()) {
                department = new Department();
                departId = rs.getInt("id");
                department.setDepartId(departId);
                department.setTitle(rs.getString("title"));
                department.setLeader(rs.getInt("leader"));
                department.setParentDepartId(rs.getInt("pid"));
                department.setStatus(rs.getInt("status"));
                department.setDescription(rs.getString("desc"));
                departInfos.put(departId, department);
            }
        } catch (SQLException e) {
            throw e;
        } finally {
            dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
        }

        return departInfos;
    }

}
