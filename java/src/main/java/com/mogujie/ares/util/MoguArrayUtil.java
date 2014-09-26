package com.mogujie.ares.util;

import java.util.HashSet;
import java.util.Set;

public class MoguArrayUtil {

	private static MoguArrayUtil instance = new MoguArrayUtil();
	
	public static MoguArrayUtil getInstance()
	{
		if(instance == null)
		{
			synchronized(instance) {
				instance = new MoguArrayUtil();
			}
		}
		return instance;
	}
	

	public int[] arrayUnique(int[] intArray) {
		Set<Integer> intSet = new HashSet<Integer>();
		int intVal;
		int length = intArray.length;  
		for(int i = 0; i < length; i++) {
			intVal = intArray[i];
			if(!intSet.contains(intVal)) {
				intSet.add(intVal);
			}
		}
		Integer[] uniqArray = new Integer[intSet.size()];
		intSet.toArray(uniqArray);
		int[] ints = new int[uniqArray.length];
		for(int i = 0; i < uniqArray.length; i++) {
			ints[i] = uniqArray[i];
		}
		return ints;
	}
}
