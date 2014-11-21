package com.mogujie.ares.lib.logger;

import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.filter.Filter;
import ch.qos.logback.core.spi.FilterReply;

/**
 * 
 * @Description: heartbeat 之类和连接相关的日志过滤器
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午3:37:45
 *
 */
public class ConnectionKeepAliveFilter extends Filter<ILoggingEvent> {

	@Override
	public FilterReply decide(ILoggingEvent event) {
		
		if (event.getMessage().contains("[LoggerFilter:Connection-Keep-alive]")) {
			return FilterReply.ACCEPT;
		} else {
			return FilterReply.DENY;
		}
	}
}
