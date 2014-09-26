package com.mogujie.ares.data;

import java.util.Map;

/**
 * 
* @ClassName: Counter
* @Description: 计数器描述类
* @author shitou - shitou(at)mogujie.com
* @date 2013-7-20 下午5:52:24
*
 */
public class Counter
{

	public int userId;
	
	public Map<String, Integer> unreadCount;
	
	public Map<String, Integer> msgCount;
	
	public void setUserId(int setUserId)
	{
		userId = setUserId;
	}
	
	public int getUserId()
	{
		return userId;
	}
	
	public void setUnreadCount(Map<String, Integer>unreadMap)
	{
		unreadCount = unreadMap;
	}
	
	public Map<String, Integer> getUnreadCount()
	{
		return unreadCount;
	}
	
	public void setMsgCount(Map<String, Integer>setMsgCount)
	{
		msgCount = setMsgCount;
	}
	
	public Map<String, Integer> getMsgCount()
	{
		return msgCount;
	}

}
