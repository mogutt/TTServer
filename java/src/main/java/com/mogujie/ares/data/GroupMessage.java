package com.mogujie.ares.data;

public class GroupMessage {


	// 消息id
	private int id;
	
	private int userId;
	
	private User userInfo;
	
	private int groupId;
	
	private Group groupInfo;
	
	private String content;
	
	/**
	 * @return the audio
	 */
	public Audio getAudio() {
		return audio;
	}

	/**
	 * @param audio the audio to set
	 */
	public void setAudio(Audio audio) {
		this.audio = audio;
	}

	private Audio audio;
	
	private int status;
	
	private int updated;
	
	/**
	 * @return the messageType
	 */
	public int getMessageType() {
		return messageType;
	}

	/**
	 * @param messageType the messageType to set
	 */
	public void setMessageType(int messageType) {
		this.messageType = messageType;
	}

	private int created;
	
	private int messageType;

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getUserId() {
		return userId;
	}

	public void setUserId(int userId) {
		this.userId = userId;
	}

	public User getUserInfo() {
		return userInfo;
	}

	public void setUserInfo(User userInfo) {
		this.userInfo = userInfo;
	}

	public int getGroupId() {
		return groupId;
	}

	public void setGroupId(int groupId) {
		this.groupId = groupId;
	}

	public Group getGroupInfo() {
		return groupInfo;
	}

	public void setGroupInfo(Group groupInfo) {
		this.groupInfo = groupInfo;
	}

	public String getContent() {
		return content;
	}

	public void setContent(String content) {
		this.content = content;
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
	
}
