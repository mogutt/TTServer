package com.mogujie.ares.lib.logger;
// ziye copy from mdl-common
public class Logger {
	
	private final org.slf4j.Logger logger;
	
	public Logger(Class<?> key) {
		this.logger = org.slf4j.LoggerFactory.getLogger(key);
	}
	
	public Logger(String key) {
		this.logger = org.slf4j.LoggerFactory.getLogger(key);
	}
	
	private String appendContextMessage(String msg) {
	    return "" + msg;
	}
    
	public void debug(String msg) {
		try {
			logger.debug(appendContextMessage(msg));
		} catch (Throwable t) {
		}
	}
	
	public void debug(String msg, Throwable e) {
		try {
			logger.debug(appendContextMessage(msg), e);
		} catch (Throwable t) {
		}
	}
	
	public void info(String format, Object[] argArray){
		try {
			logger.info(format,argArray);
		} catch (Throwable t) {
		}
	}
	
	public void info(String msg) {
		try {
			logger.info(appendContextMessage(msg));
		} catch (Throwable t) {
		}
	}
	
	public void info(String msg, Throwable e) {
		try {
			logger.info(appendContextMessage(msg), e);
		} catch (Throwable t) {
		}
	}
    
	public void warn(String msg, Throwable e) {
		try {
			logger.warn(appendContextMessage(msg), e);
		} catch (Throwable t) {
		}
	}

	public void warn(String msg) {
		try {
			logger.warn(appendContextMessage(msg));
		} catch (Throwable t) {
		}
	}
	
	public void error(String msg) {
		try {
			logger.error(appendContextMessage(msg));
		} catch (Throwable t) {
		}
	}
	
	public void error(String msg, Throwable e) {
		try {
			logger.error(appendContextMessage(msg), e);
		} catch (Throwable t) {
		}
	}
	
}
