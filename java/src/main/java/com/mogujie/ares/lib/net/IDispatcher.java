package com.mogujie.ares.lib.net;

import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.MessageEvent;

/**
 * 
 * @ClassName: IDispatcher 
 * @Description: 请求分发的接口
 * @author ziye - ziye(at)mogujie.com
 * @date 2013-7-20 下午5:55:47
 *
 */
public interface IDispatcher {
	
	public void dispatch(ChannelHandlerContext context, MessageEvent e);
	
}
