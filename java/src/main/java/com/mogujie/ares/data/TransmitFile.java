package com.mogujie.ares.data;

/**
 *
 * @Description: 群信息
 * @author ziye - ziye[at]mogujie.com
 * @date 2014-1-5 下午4:17:55
 *
 */
public class TransmitFile {
	
	private int id; 
	
	private int fromUserId; // 发送人
	
	private int toUserId; // 接收人

	private String taskId = ""; //任务编号
	
	private String filePath = ""; // 文件路径
	
	private int fileSize;		//文件大小.
	
	private int status; // 状态，是否已经被接收
	
	private int created; // 创建时间
	
	private int updated; // 更新时间

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getFromUserId() {
		return fromUserId;
	}

	public void setFromUserId(int fromUserId) {
		this.fromUserId = fromUserId;
	}

	public int getToUserId() {
		return toUserId;
	}

	public void setToUserId(int toUserId) {
		this.toUserId = toUserId;
	}

	public String getFilePath() {
		return filePath;
	}

	public void setFilePath(String filePath) {
		this.filePath = filePath;
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

	public int getStatus() {
		return status;
	}

	public void setStatus(int status) {
		this.status = status;
	}

	public int getFileSize() {
		return fileSize;
	}

	public void setFileSize(int fileSize) {
		this.fileSize = fileSize;
	}

	public String getTaskId() {
		return taskId;
	}

	public void setTaskId(String taskId) {
		this.taskId = taskId;
	}

	
}
