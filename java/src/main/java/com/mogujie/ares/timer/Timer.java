package com.mogujie.ares.timer;

import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import com.mogujie.ares.manager.ConfigureManager;

/**
 * 
 * @Description: 定时器，实际上就是一个ScheduledExecutorService的封装类
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 下午2:55:21
 *
 */
public class Timer
{
	private Properties systemProperties;
	
	private ScheduledExecutorService schedulerService; // 线程池

	private boolean isLaunch = false;
	
	private static Timer timerInstance = new Timer();
	
	public static Timer getInstance()
	{
		if(timerInstance == null)
		{
			timerInstance = new Timer();
		}
		return timerInstance;
	}
	
	private Timer()
	{
		systemProperties = ConfigureManager.getInstance().getSystemConfig();
		launch();
	}
	
	// 初始化配置 
	private void launch()
	{
		if( ! isLaunch && schedulerService == null)
		{
			schedulerService = Executors.newScheduledThreadPool(
					Integer.valueOf(systemProperties.getProperty("task_initialize_pool")));
		}
	}
	
	/**
	 * 
	 * @Description: 启动一个只运行一次的任务
	 * @param task 
	 * @param delay 等待delay时间之后再执行
	 * @param unit
	 * @return
	 */
	public ScheduledFuture<?> submitOneShotTask(Runnable task, long delay, TimeUnit unit)
	{
		ScheduledFuture<?> scheduler = schedulerService.schedule(task, delay, unit);
		return scheduler;
	}
	
	/**
	 * 
	 * @Description: 启动一个固定延迟周期执行的脚本,
	 * 				指上一次执行结束之后延迟period时间之后再执行
	 * 如initialDelay = 1; delay = 3; unit = second; 执行时间固定需要2秒
	 * 从time=0开始执行的时间应该是1, 6, 11, 16....
	 * @param task
	 * @param initialDelay
	 * @param period
	 * @param unit
	 * @return
	 */
	public ScheduledFuture<?> submitFixedRateTask(Runnable task, long initialDelay, long period, TimeUnit unit)
	{
		ScheduledFuture<?> scheduler = schedulerService.scheduleAtFixedRate(task, initialDelay, period, unit);
		return scheduler;
	}
	
	/**
	 * 
	 * @Description: 启动一个固定周期执行的脚本,不管上一个任务执行多久，
	 * 				下一个任务都是在上一个开始执行的delay时间之后执行
	 * 如initialDelay = 1; delay = 3; unit = second; 执行时间固定需要2秒
	 * 从time=0开始执行的时间应该是1, 4, 7, 10....
	 * @param task
	 * @param initialDelay
	 * @param delay
	 * @param unit
	 * @return
	 */
	public ScheduledFuture<?> submitFixedDelayTask(Runnable task, long initialDelay, long delay, TimeUnit unit)
	{
		ScheduledFuture<?> scheduler = schedulerService.scheduleWithFixedDelay(task, initialDelay, delay, unit);
		return scheduler;
	}

	/**
	 * @Description: 关闭所有任务
	 */
	public void shutDown()
	{
		if(schedulerService != null)
		{
			schedulerService.shutdown();
		}
	}
	
	/**
	 * @Description: 关闭所有任务
	 */
	public void shutDownNow()
	{
		if(schedulerService != null)
		{
			schedulerService.shutdownNow();
		}
	}
	
	
}