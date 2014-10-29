package com.mogujie.ares.util;

public class MoguArrayUtil {

	private static MoguArrayUtil instance;
	
	public static MoguArrayUtil getInstance()
	{
		if(instance == null)
		{
			synchronized(MoguArrayUtil.class) {
				instance = new MoguArrayUtil();
			}
		}
		return instance;
	}
	

	public int[] arrayUnique(int[] intArray) {
		return MoguUtil.distinct(intArray);
	}
}
