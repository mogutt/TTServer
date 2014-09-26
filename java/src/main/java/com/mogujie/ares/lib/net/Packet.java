package com.mogujie.ares.lib.net;

/**
 * 
 * @Description: 发送和接收的数据包
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午4:14:31
 *
 */
public class Packet {

	private int length; // 数据包长度，包括包头
	
	private int version; // 版本号
		
	private int serviceId; // 服务号, 后端这边固定1000
	
	private int commandId; // 命令号, 标识服务接口, 
	
	private int reserved; // 保留，可用于如序列号等
	
	private DataBuffer contentBuffer; // 业务数据部分

	public int getLength() {
		return length;
	}

	public void setLength(int length) {
		this.length = length;
	}

	public int getVersion() {
		return version;
	}

	public void setVersion(int version) {
		this.version = version;
	}

	public int getServiceId() {
		return serviceId;
	}

	public void setServiceId(int serviceId) {
		this.serviceId = serviceId;
	}

	public int getCommandId() {
		return commandId;
	}

	public void setCommandId(int commandId) {
		this.commandId = commandId;
	}

	public int getReserved() {
		return reserved;
	}

	public void setReserved(int reserved) {
		this.reserved = reserved;
	}

	public DataBuffer getContentBuffer() {
		return contentBuffer;
	}

	public void setContentBuffer(DataBuffer contentBuffer) {
		this.contentBuffer = contentBuffer;
	}
}
