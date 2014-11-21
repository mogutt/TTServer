package com.mogujie.ares.timer;

import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;

/**
 * 
 * @Description: 配置实时更新的类
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 下午2:54:08
 *
 */
public class ConfigureReloadTask implements Runnable
{
	private Logger logger = LoggerFactory.getLogger(ConfigureReloadTask.class);
	@Override
	public void run()
	{
		//重载所有的配置
		//ConfigureManager.getInstance().reloadAllConfigs();
		logger.info("Hello World");
	}


}
