package com.mogujie.ares.lib.logger;

import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.filter.Filter;
import ch.qos.logback.core.spi.FilterReply;

/**
 * 
* @Description: 为过滤的一些通用日志的过滤器
* @author ziye - ziye[at]mogujie.com
* @date 2013-7-21 下午3:38:53
*
 */
public class NoneFilteredLoggerFilter extends Filter<ILoggingEvent> {

	@Override
	public FilterReply decide(ILoggingEvent event) {
		
		if (event.getMessage().contains("[LoggerFilter:")) {
			return FilterReply.DENY;
		} else {
			return FilterReply.ACCEPT;
		}
	}
}
