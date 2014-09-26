package com.mogujie.ares.data;

/**
 * 
 * @ClassName: Relationship
 * @Description: 好友关系
 * @author ziye - ziye(at)mogujie.com
 * @date 2013-7-20 下午5:50:10
 *
 */
public class Relationship {

	private int relateId;
	
	private int userId;
	
	private int friendUserId;
	
	private int status;
	
	private int created;
	
	private int updated;

	public int getRelateId() {
		return relateId;
	}

	public void setRelateId(int relateId) {
		this.relateId = relateId;
	}

	public int getUserId() {
		return userId;
	}

	public void setUserId(int userId) {
		this.userId = userId;
	}

	public int getFriendUserId() {
		return friendUserId;
	}

	public void setFriendUserId(int friendUserId) {
		this.friendUserId = friendUserId;
	}

	public int getStatus() {
		return status;
	}

	public void setStatus(int status) {
		this.status = status;
	}

	public int getCreated() {
		return created;
	}

	public void setCreated(int created) {
		this.created = created;
	}

	public int getUpdated() {
		return updated;
	}

	public void setUpdated(int updated) {
		this.updated = updated;
	}
	
}
