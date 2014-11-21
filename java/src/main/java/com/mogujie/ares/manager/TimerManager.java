package com.mogujie.ares.manager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import com.mogujie.ares.configure.SysConstants;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.timer.Timer;

/**
 * 
 * @Description: 定时脚本管理 
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-22 下午1:03:05
 * 
 * @param <E>
 */
public class TimerManager
{
	private static final Logger logger = LoggerFactory.getLogger(TimerManager.class);
	
	ConcurrentHashMap<String, HashMap<String, String>> timerConfigure = new ConcurrentHashMap<String, HashMap<String,String>>(); 
	
	private List<Runnable> tasks = new ArrayList<Runnable>();
	
	ConcurrentHashMap<String, ScheduledFuture<?>> taskFutures = new ConcurrentHashMap<String, ScheduledFuture<?>>();
	
	private boolean isLaunch = false;
	
	private Timer timer; // 一个线程池
	
	private static TimerManager timerManagerInstance;
	
	public static TimerManager getInstance()
	{
		if(timerManagerInstance == null)
		{
			timerManagerInstance = new TimerManager();
		}
		return timerManagerInstance;
	}
	
	private TimerManager() {
		initialize();
	} 
	
	/**
	 * 
	 * @Description:  初始化
	 */
	private void initialize()
	{
		timerConfigure = ConfigureManager.getInstance().getTimerConfig();
		
		timer = Timer.getInstance();
	}
	
	/**
	 * 
	 * @Description: 创建所有初始化的任务
	 */
	private void createInitializeAllTasks()
	{
		Iterator<String> iter = timerConfigure.keySet().iterator();
		while(iter.hasNext())
		{
			String taskName = iter.next();
			try {
				Class<?> taskClass;
				taskClass = Class.forName(taskName);
				Object taskInstance = taskClass.newInstance();
				if(taskInstance != null) {
					Runnable taskRunnable = (Runnable) taskInstance;
					tasks.add(taskRunnable);
				}
			} catch (ClassNotFoundException e) {
				logger.error("", e);
			} catch (InstantiationException e) {
				logger.error("", e);
			} catch (IllegalAccessException e) {
				logger.error("", e);
			}
		}
	}
	
	/**
	 *  
	 * @throws Exception 
	 * @Description: 提交一个初始化的任务
	 */
	protected void submitInitialTasks() throws Exception
	{
		if(tasks.size() > 0)
		{
			Iterator<Runnable> iter = tasks.iterator();
			while(iter.hasNext())
			{
				try{
					Runnable initialTask = iter.next();
					HashMap<String, String> taskConfig = timerConfigure.get(initialTask.getClass().getName()); 
					if( ! taskConfig.get("enable").equals(SysConstants.TASK_ENABLE_RUN)) {
						continue;
					}
					
					String taskType = taskConfig.get("taskType");
					if(taskType.equals("rateTask")) {
						submitFixedRateTask(initialTask);
					} else if(taskType.equals("delayTask")) {
						submitFixedDelayTask(initialTask);
					} else if(taskType.equals("oneShot")) {
						submitOneShotTask(initialTask);
					}
				} catch (Exception e) {
					throw e;
				}
			}
		}
	}
	
	/**
	 * 
	 * @Description: 初始化加载
	 * @throws Exception
	 */
	public void lanuch() throws Exception
	{
		if( ! isLaunch)
		{
			createInitializeAllTasks();
			
			submitInitialTasks();
		}
	}
	
	/**
	 * 提交定时类task
	 */
	public void submitFixedRateTask(Runnable task)
	{
		logger.info("submit fixed rate task");
		String taskName = task.getClass().getName();
		long initialDelay = Integer.valueOf(timerConfigure.get(taskName).get("initdelay"));
		long period = Integer.valueOf(timerConfigure.get(taskName).get("periodordelay"));
		taskFutures.put(taskName, timer.submitFixedRateTask(task, initialDelay, period, TimeUnit.SECONDS));
	}
	
	/**
	 * 提交delay循环执行的task
	 */
	public void submitFixedDelayTask(Runnable task)
	{
		String taskName = task.getClass().getName();
		long initialDelay = Integer.valueOf(timerConfigure.get(taskName).get("initialdelay"));
		long delay = Integer.valueOf(timerConfigure.get(taskName).get("periodordelay"));
		taskFutures.put(taskName, timer.submitFixedDelayTask(task, initialDelay, delay, TimeUnit.SECONDS));
	}
	
	/**
	 * 提交仅执行一次的task
	 */
	public void submitOneShotTask(Runnable task)
	{
		String taskName = task.getClass().getName();
		long initialDelay = Integer.valueOf(timerConfigure.get(taskName).get("initaildelay"));
		taskFutures.put(taskName, timer.submitOneShotTask(task, initialDelay, TimeUnit.SECONDS));
	}
	
	/**
	 * 
	 * @Description: 关闭定时任务
	 */
	public void shutDown()
	{
		if(timer != null)
		{
			timer.shutDown();
			
			clean();
		}
	}
	
	/**
	 * 
	 * @Description: 关闭定时任务,不等待当前自然结束
	 */
	public void shutDownNow()
	{
		if(timer != null)
		{
			timer.shutDownNow();
			
			clean();
		}
	}
	
	/**
	 * 
	 * @Description: 擦屁股
	 */
	private void clean()
	{
		if(taskFutures.size() > 0)
		{
			taskFutures.clear();
		}
		
		if(tasks.size() > 0)
		{
			tasks.clear();
		}
	}
	
	/**
	 * 检查task是否都完成了，如果没有，则堵塞调用的线程
	 */
	public void checkTasksCompleted()
	{
		if(taskFutures.size() > 0)
		{
			Iterator<Map.Entry<String, ScheduledFuture<?>>> iter = taskFutures.entrySet().iterator();
			while(iter.hasNext())
			{
				Map.Entry<String, ScheduledFuture<?>> entry = iter.next();
				try {
					entry.getValue().get(); // 这里会wait
				} catch (InterruptedException e) {
					logger.error("", e);
				} catch (ExecutionException e) {
					logger.error("", e);
				}
			}
		}
	}
}
