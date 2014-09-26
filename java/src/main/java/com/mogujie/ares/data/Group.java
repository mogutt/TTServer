package com.mogujie.ares.data;

import java.util.List;

/**
 *
 * @Description: 群信息
 * @author ziye - ziye[at]mogujie.com
 * @date 2014-1-5 下午4:17:55
 *
 */
public class Group {
	
	private int groupId; // 群Id
	
	private String groupName = ""; // 群名
	
	private String  avatar = "";	// 群头像
	
	private String adesc = ""; // 群描述
	
	private int createUserId; // 创建者Id
	
	private int groupType; // 群类型 1：固定群 2：临时群
	
	private int status; // 群状态 1：正常 0: 删除
	
	private int memberCnt; // 群成员个数
	
	private List<Integer> userIdList;
	
	private int updated; // 群信息更新时间
	
	private int created; // 群创建时间

	public int getGroupId() {
		return groupId;
	}

	public void setGroupId(int groupId) {
		this.groupId = groupId;
	}

	public String getGroupName() {
		return groupName;
	}

	public void setGroupName(String groupName) {
		this.groupName = groupName;
	}

	public String getAvatar() {
		return avatar;
	}

	public void setAvatar(String avatar) {
		this.avatar = avatar;
	}

	public String getAdesc() {
		return adesc;
	}

	public void setAdesc(String adesc) {
		this.adesc = adesc;
	}

	public int getCreateUserId() {
		return createUserId;
	}

	public void setCreateUserId(int createUserId) {
		this.createUserId = createUserId;
	}

	public int getGroupType() {
		return groupType;
	}

	public void setGroupType(int groupType) {
		this.groupType = groupType;
	}

	public int getMemberCnt() {
		return memberCnt;
	}

	public void setMemberCnt(int memberCnt) {
		this.memberCnt = memberCnt;
	}

	public int getStatus() {
		return status;
	}

	public void setStatus(int status) {
		this.status = status;
	}

	public int getUpdated() {
		return updated;
	}

	public void setUpdated(int updated) {
		this.updated = updated;
	}

	public int getCreated() {
		return created;
	}

	public void setCreated(int created) {
		this.created = created;
	}

	public List<Integer> getUserIdList() {
		return userIdList;
	}

	public void setUserIdList(List<Integer> userIdList) {
		this.userIdList = userIdList;
	}
}
