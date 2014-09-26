package com.mogujie.ares.timer;

/**
 * 
 * @Description: 配置实时更新的类
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 下午2:54:08
 * 
 */
//public class WorkerInfoReloader implements Runnable {
// private Logger logger =
// LoggerFactory.getLogger(WorkerInfoReloader.class);
// @Override
// public void run()
// {
// logger.info("reload worker info");
// //重载所有的配置
// boolean isSuccess = InternalDataModel.getInstance().refreshUserList();
// if(!isSuccess) {
// logger.error("reload worker info list fail");
// }
// logger.info("reload worker info list success");
// }

// public void recorectData() {
// List<GroupRelation> groupList = new ArrayList<GroupRelation>();
// DBManager dbManager = DBManager.getInstance();
// Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
// PreparedStatement statement = null;
// ResultSet rs = null;
// CounterModel cm = CounterModel.getInstance();
// try {
// String sql = "select * from IMGroupRelation where " +
// "status = 1 and groupType = 2 order by updated desc, id desc";
// statement = conn.prepareStatement(sql);
// rs = statement.executeQuery();
// GroupRelation gr = null;
// while(rs.next()) {
// gr = new GroupRelation();
// gr.setGroupId(rs.getInt("groupId"));
// gr.setUserId(rs.getInt("userId"));
// groupList.add(gr);
// }
// int size = groupList.size();
// logger.info("size: " + size);
// for(int i = 0; i < size; i++) {
// gr = groupList.get(i);
// Map<Integer, Integer> grcnt = cm.getUserGroupUnreadCount(gr.getUserId(),
// new int[]{gr.getGroupId()});
// GroupMessage[] gm =
// MessageModel.getInstance().getGroupMessages(gr.getGroupId(), 0, 1);
// logger.info("crt: " + gm[0].getCreated() + ", grcnt: " + grcnt);
// if(gr.getUserId() == 9822376 && grcnt.get(gr.getGroupId()) > 0 &&
// gm[0].getCreated() > 1400752800) {
// // cm.clearUserGroupCounter(gr.getUserId(), gr.getGroupId());
// logger.info("清除消息: userId = " + gr.getUserId()
// + " , groupId = " + gr.getGroupId() + ", cnt=" +
// grcnt.get(gr.getGroupId()));
// }
// }
// } catch (SQLException e) {
// logger.error("", e);
// } finally {
// dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
// }
// }
// }
