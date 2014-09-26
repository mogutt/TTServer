package com.mogujie.ares.data;

public class GroupCounterItem {
	
	private int userId;
	
	private int groupId;
	
	private int groupTotalCount; // 群总消息数
	
	private int userUnreadCount; // 用户在该群中的未读消息数
	
	private int lastMessageId;

	public int getUserId() {
		return userId;
	}

	public void setUserId(int userId) {
		this.userId = userId;
	}

	public int getGroupId() {
		return groupId;
	}

	public void setGroupId(int groupId) {
		this.groupId = groupId;
	}

	public int getGroupTotalCount() {
		return groupTotalCount;
	}

	public void setGroupTotalCount(int groupTotalCount) {
		this.groupTotalCount = groupTotalCount;
	}

	public int getUserUnreadCount() {
		return userUnreadCount;
	}

	public void setUserUnreadCount(int userUnreadCount) {
		this.userUnreadCount = userUnreadCount;
	}

	public int getLastMessageId() {
		return lastMessageId;
	}

	public void setLastMessageId(int lastMessageId) {
		this.lastMessageId = lastMessageId;
	}
}
