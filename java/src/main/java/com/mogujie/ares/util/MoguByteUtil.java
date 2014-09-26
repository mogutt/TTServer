package com.mogujie.ares.util;

public class MoguByteUtil {

	private static MoguByteUtil instance = new MoguByteUtil();
	
	public static MoguByteUtil getInstance()
	{
		if(instance == null)
		{
			synchronized(instance) {
				instance = new MoguByteUtil();
			}
		}
		return instance;
	}
	
	public int convert2Int(byte[] bytes) {
		 return (((int)bytes[0]) << 24) + (((int)bytes[1]) << 16) + (((int)bytes[2]) << 8) + bytes[3];
	}
	
	public byte[] getBytes(int value) {
		byte[] bytes = new byte[4];
		bytes[0] = (byte)(value >>> 24);//取最高8位放到0下标
		bytes[1] = (byte)(value >>> 16);//取次高8为放到1下标
		bytes[2] = (byte)(value >>> 8); //取次低8位放到2下标
		bytes[3] = (byte)(value );      //取最低8位放到3下标
		return bytes;
	}
}
