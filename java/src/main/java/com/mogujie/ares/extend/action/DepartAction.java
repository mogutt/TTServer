package com.mogujie.ares.extend.action;

import java.sql.SQLException;
import java.util.Iterator;
import java.util.Map;

import com.mogujie.ares.data.Department;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.DepartModel;
import com.mogujie.ares.util.MoguUtil;

public class DepartAction extends BaseAction {
    private static final Logger logger = LoggerFactory
            .getLogger(DepartAction.class);

    /*
     * 
     * @Description: 获取所有部门信息
     * 
     * @param fromUserId
     * 
     * @return
     */
    public DataBuffer getDepartmentInfo(int fromUserId, DataBuffer attachment,
            int version) {
        logger.info("get all department info by user : " + fromUserId);
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
            Map<Integer, Department> departments = DepartModel.getInstance()
                    .getDepartmentInfo();
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId); // 发请求的用户
            buffer.writeInt(departments.size()); // 查询到的部门个数
	    logger.info("departments has : " + departments.size());
            Department department;
            Iterator<Integer> it = departments.keySet().iterator();
            while (it.hasNext()) {
                department = departments.get(it.next());
                buffer.writeInt(department.getDepartId()); // 部门id
                buffer.writeString(department.getTitle()); // 部门标题
                buffer.writeString(department.getDescription()); // 部门描述
                buffer.writeInt(department.getParentDepartId()); // 上级部门ID
                buffer.writeInt(department.getLeader()); // 部门leader
                buffer.writeInt(department.getStatus()); // 部门状态
            }
        } catch (SQLException e) {
            logger.error("get all department info error with reason : ", e);
            buffer = new DataBuffer();
            buffer.writeInt(fromUserId);
            buffer.writeInt(0);
        }

        return MoguUtil.writeAttachments(buffer, attachment);
    }

    // /**
    // *
    // * @Description: 获取组织架构信息(部门信息+员工)
    // * @param fromUserId
    // * @param departIds
    // * @return
    // */
    // public DataBuffer getOrganizationalChart(int fromUserId,
    // DataBuffer attachment, int version) {
    // logger.info("get organizational chart by user : " + fromUserId);
    // DataBuffer buffer;
    // if (fromUserId <= 0) {
    // buffer = new DataBuffer();
    // buffer.writeInt(fromUserId);
    // buffer.writeInt(0);
    // buffer.writeInt(attachment.readableBytes());
    // if (attachment.readableBytes() > 0) {
    // buffer.writeDataBuffer(attachment);
    // }
    // return buffer;
    // }
    //
    // try {
    // Map<Integer, Department> departments = DepartModel.getInstance()
    // .getDepartmentInfo();
    // buffer = new DataBuffer();
    // buffer.writeInt(fromUserId); // 发请求的用户
    // buffer.writeInt(departments.size()); // 查询到详细信息的用户数
    // Department department;
    // Iterator<Integer> it = departments.keySet().iterator();
    // Set<User> setUsers = null;
    // while (it.hasNext()) {
    // department = departments.get(it.next());
    // buffer.writeInt(department.getDepartId()); // 部门id
    // buffer.writeString(department.getTitle()); // 部门标题
    // buffer.writeString(department.getDescription()); // 部门描述
    // buffer.writeInt(department.getParentDepartId()); // 上级部门ID
    // buffer.writeString(department.getLeader()); // 部门leader
    // buffer.writeInt(department.getStatus()); // 部门状态
    //
    // setUsers = UserModel.getInstance().getUserInfoByDepartId(
    // department.getDepartId());
    // buffer.writeInt(setUsers.size()); // 部门人员数
    // Iterator<User> itr = setUsers.iterator();
    // User user = null;
    // while (itr.hasNext()) {
    // user = itr.next();
    // buffer.writeInt(user.getUserId());
    // buffer.writeInt(user.getSex());
    // buffer.writeString(user.getUname());
    // buffer.writeString(user.getUnick());
    // buffer.writeString(user.getAvatar());
    // buffer.writeString(user.getTitle());
    // buffer.writeInt(user.getJobNumber());
    // buffer.writeString(user.getMail());
    // buffer.writeString(user.getTelphone());
    // }
    // }
    // } catch (SQLException e) {
    // logger.error("get organizational chart error with reason : ", e);
    // buffer = new DataBuffer();
    // buffer.writeInt(fromUserId);
    // buffer.writeInt(0);
    // }
    //
    // return MoguUtil.writeAttachments(buffer, attachment);
    // }
}
