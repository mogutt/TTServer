package com.mogujie.ares.lib.net;

import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelStateEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelHandler;

import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.NetworkManager;

/**
 * 
 * @Description: netty接收数据的handler，所有请求都在这里接收并转发到dispatcher
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午3:39:34
 *
 */
public class BinaryMessageHandler extends SimpleChannelHandler  {
	
	private static final Logger logger = LoggerFactory.getLogger(BinaryMessageHandler.class);

	private IDispatcher dispatcher;
	
	public BinaryMessageHandler(IDispatcher dispatcher) {
		this.dispatcher = dispatcher;
	}
	
	/**
	 * 客户端建立连接
	 * (non-Javadoc)
	 * @see org.jboss.netty.channel.SimpleChannelHandler#channelConnected(org.jboss.netty.channel.ChannelHandlerContext, org.jboss.netty.channel.ChannelStateEvent)
	 */
	public void channelConnected(ChannelHandlerContext context, ChannelStateEvent e)
			throws Exception {
		String clientAddr = context.getChannel().getRemoteAddress().toString();
		logger.info("[LoggerFilter:Connection-Keep-alive] client " + clientAddr + " Connected");
		NetworkManager.getInstance().addClient(context); // 添加到现有客户端的列表里
		super.channelConnected(context, e);
	}
	
	/**
	 * 收到数据的事件
	 */
	@Override
	public void messageReceived(ChannelHandlerContext context, MessageEvent e)
			throws Exception {
		try{
			dispatcher.dispatch(context, e);
		} catch(Exception exception) {
			logger.error("dispatch error. ", exception);
		}
	}

	/**
	 * 客户端断开连接
	 * (non-Javadoc)
	 * @see org.jboss.netty.channel.SimpleChannelHandler#channelDisconnected(org.jboss.netty.channel.ChannelHandlerContext, org.jboss.netty.channel.ChannelStateEvent)
	 */
	public void channelDisconnected(ChannelHandlerContext context,
			ChannelStateEvent e) throws Exception {
		String clientAddr = context.getChannel().getRemoteAddress().toString();
		logger.info("[LoggerFilter:Connection-Keep-alive] client " + clientAddr + " Disconnected");
		NetworkManager.getInstance().removeClient(context); // 从现有客户端列表里移除该连接
		super.channelDisconnected(context, e);
	}

	
}
