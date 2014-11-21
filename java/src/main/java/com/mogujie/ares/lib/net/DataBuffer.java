package com.mogujie.ares.lib.net;

import java.util.List;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffers;

/**
 * 
 * @Description: 数据缓冲区对象，直接封装了netty的ChannelBuffer
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午3:40:52
 *
 */
public class DataBuffer {
	
	protected ChannelBuffer buffer;
	
	public DataBuffer() {
		buffer = ChannelBuffers.dynamicBuffer();
	}
	
	public DataBuffer(ChannelBuffer binaryBuffer) {
		buffer = binaryBuffer;
	}
	
	public DataBuffer(int length) {
		buffer = ChannelBuffers.buffer(length);
	}
	
	public byte[] array() {
		return buffer.array();
	}

	public void setOrignalBuffer(ChannelBuffer buffer) {
		this.buffer = buffer;
	}
	
	public ChannelBuffer getOrignalBuffer() {
		return buffer;
	}
	
	public void writeByte(int value) {
		buffer.writeByte(value);
	}
	
	public byte readByte() {
		return buffer.readByte();
	}
	
	public void writeBytes(byte[] bytes) {
		buffer.writeBytes(bytes);
	}
	
	public byte[] readBytes(int length) {
		byte[] bytes = new byte[length];
		buffer.readBytes(bytes);
		return bytes;
	}
	
	public int readInt() {
		if(buffer.readable()){
			return buffer.readInt();
		}else{
			return 0;
		}
	}
	
	public void writeInt(int value) {
		buffer.writeInt(value);
	}
	
	public char readChar() {
		return buffer.readChar();
	}
	
	public void writeChar(char c) {
		buffer.writeChar(c);
	}
	
	public long readLong() {
		return buffer.readLong();
	}
	
	public void writeLong(long value) {
		buffer.writeLong(value);
	}
	
	public double readDouble(){
		return buffer.readDouble();
	}
	
	public void writeDouble(double value){
		buffer.writeDouble(value);
	}
	
	/**
	 * 读取一个字符串
	 * @return 
	 * 格式：前导length表示字符串的byte数
	 * length(4字节)string(length字节)
	 */
	public String readString() {
		int length = readInt();
		byte[] bytes = readBytes(length);
		
		return new String(bytes);
	}
	
	/**
	 * 写入一个字符串
	 * @param str 
	 * 数据格式见方法readString()
	 */
	public void writeString(String str) {
		byte[] bytes = str.getBytes();
		writeInt(bytes.length);
		writeBytes(bytes);
	}
	
	/**
	 * 读取int数组
	 * @return
	 * 格式：前导count表示数组中有多少个元素 
	 * count(4字节)int1(4字节)...intCount(4字节)
	 */
	public int[] readIntArray() {
		int count = readInt();
		int[] intArray = new int[count];
		for(int i = 0; i < count; i++) {
			intArray[i] = readInt();
		}
		return intArray;
	}
	
	/**
	 * 写入int数组
	 * @param intArray 格式见readIntArray()
	 */
	public void writeIntArray(int[] intArray) {
		int count = intArray.length;
		writeInt(count);
		for(int i = 0; i < count; i++) {
			writeInt(intArray[i]);
		}
	}

	/**
	 * 
	 * @Description: 写入一个int的list,list转数组太蛋疼了
	 * @param intList
	 */
	public void writeIntList(List<Integer> intList) {
		if(intList == null || intList.isEmpty()) {
			writeInt(0);
			return ;
		}
		int count = intList.size();
		writeInt(count);
		for(int i = 0; i < count; i++) {
			writeInt(intList.get(i));
		}
	}
	
	/**
	 * 读取byte数组
	 * @return
	 * 格式：前导count表示数组中有多少个元素 
	 * count(4字节)byte1(4字节)...byteCount(4字节)
	 */
	public byte[] readByteArray() {
		int length = readInt(); // 获取长度
		byte[] bytes = new byte[length];
		buffer.readBytes(bytes);
		return bytes;
	}
	
	/**
	 * 写入byte数组
	 * @param byteArray 格式见readByteArray()
	 */
	public void writeByteArray(byte[] byteArray) {
		int length = byteArray.length;
		writeInt(length);
		buffer.writeBytes(byteArray);
	}
	/**
	 * 读取String数组
	 * @return
	 * 格式：前导count表示数组中有多少个元素 
	 * count(4字节)string1(4字节)...stringsCount(4字节)
	 */
	public String[] readStringArray() {
		int count = readInt(); // 获取长度
		String[] strArray = new String[count];
		for(int i = 0; i < count; i++) {
			strArray[i] = readString();
		}
		return strArray;
	}
	
	/**
	 * 写入String数组
	 * @param byteArray 格式见readStringArray()
	 */
	public void writeStringArray(String[] strArray) {
		int count = strArray.length;
		writeInt(count);
		for(int i = 0; i < count; i++) {
			writeString(strArray[i]);
		}
	}
	
	/**
	 * 获取有效(可读取)的byte数
	 * @return
	 */
	public int readableBytes() {
		return buffer.readableBytes();
	}
	
	public DataBuffer readDataBuffer() {
		if(buffer == null || buffer.readableBytes() == 0) {
			return new DataBuffer(0);
		}
		int length = readInt();
		DataBuffer dataBuffer = new DataBuffer(0);
		dataBuffer.setOrignalBuffer(buffer.readBytes(length));
		return dataBuffer;
	}
	
	public void writeDataBuffer(DataBuffer inputBuffer) {
		if(inputBuffer == null || inputBuffer.readableBytes() == 0) {
			return ;
		}
		buffer.writeBytes(inputBuffer.buffer);
	}
}
