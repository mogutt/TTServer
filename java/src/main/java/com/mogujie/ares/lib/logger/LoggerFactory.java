package com.mogujie.ares.lib.logger;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
// ziye copy from mdl-common
public class LoggerFactory {
	
	private static final ConcurrentMap<String, Logger> loggers = new ConcurrentHashMap<String, Logger>();

	public static Logger getLogger(Class<?> key) {
		Logger logger = loggers.get(key.getName());
		if (logger == null) {
			loggers.putIfAbsent(key.getName(), new Logger(key));
			logger = loggers.get(key.getName());
		}
		return logger;
	}
	
	public static Logger getLogger(String key) {
		Logger logger = loggers.get(key);
		if (logger == null) {
			loggers.putIfAbsent(key, new Logger(key));
			logger = loggers.get(key);
		}
		return logger;
	}
	
}
