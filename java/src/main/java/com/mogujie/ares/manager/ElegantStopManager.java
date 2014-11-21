package com.mogujie.ares.manager;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;

import org.jboss.netty.channel.ChannelHandlerContext;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.configure.SysConstants;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.lib.net.Packet;

/**
 * 
 * @Description: 传说中石头的优雅退出,单例
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 上午11:15:53
 *
 */
public class ElegantStopManager
{
	private int port = 0;
	
	public static final Logger logger = LoggerFactory.getLogger(ElegantStopManager.class);
	
	private static ElegantStopManager instance = null;
	
	public static ElegantStopManager getInstance(int port)
	{
		if(instance == null)
		{
			instance = new ElegantStopManager();
			instance.port = port;
			instance.initialize();
		}
		return instance;
	}
	
	public void initialize()
	{
		resetStopConfigFileContent();
		generateShutDownShellFile();
	}
	
	/**
	 * 启动的时候，重置文件内容
	 */
	public void resetStopConfigFileContent()
	{
		File stopConfigFile = new File(getShutDownFilePath());
		if(stopConfigFile.exists())
		{
			stopConfigFile.delete();
		}
	}
	
	/**
	 * 判断是否停止正在运行的实例
	 * @return
	 */
	public boolean isStopCurrentJavaInstance()
	{
		boolean isStop = false;
		File stopConfigFile = new File(getShutDownFilePath());
		// 文件不存在则新建个文件
		if( ! stopConfigFile.exists())
		{
			try {
				stopConfigFile.createNewFile();
			} catch (IOException e) {
				logger.error("", e);
			}
			return isStop;
		}
		
		BufferedReader bufferedReader = null;
		try {
			bufferedReader = new BufferedReader(new FileReader(stopConfigFile));
			String stopFileContent = bufferedReader.readLine();
			isStop = SysConstants.ELEGANT_STOP_CONTENT.equals(stopFileContent);
		} catch (FileNotFoundException e) {
			logger.error("", e);
		} catch (IOException e) {
			logger.error("", e);
		} finally {
			try {
				if(bufferedReader != null) {
					bufferedReader.close();
				}
			} catch (IOException e) {
				logger.error("", e);
			}
		}
		return isStop;
	}

	/**
	 * @Description: 包装一个停止的数据包， type=200
	 * @return
	 */
	protected Packet generateStopReceivePacket() 
	{
		Packet stopPacket = new Packet();
		stopPacket.setLength(SysConstants.PROTOCOL_HEADER_LENGTH + 4);
		stopPacket.setVersion(SysConstants.PROTOCOL_PREVIOUS_VERSION);
		stopPacket.setServiceId(BizConstants.DEFAULT_SERVICEID);
		stopPacket.setCommandId(BizConstants.COMMANDID_STOP_RECEIVE);
		stopPacket.setReserved((char)0);
		DataBuffer dataBuffer = new DataBuffer(4); // 数据部分
		dataBuffer.writeInt(1);
		stopPacket.setContentBuffer(dataBuffer);
		return stopPacket;
	}
	
	/**
	 * 
	 * @Description: netty停止接收数据
	 * 				向每个客户端都发一个即将关闭的数据包，告诉她们:“我要挂了~”
	 */
	public void nettyStopReceivePacket()
	{
		ConcurrentHashMap<String, ChannelHandlerContext> nettyClients = NetworkManager.getInstance().getClientMap();
		if(nettyClients.size() > 0)
		{
			Iterator<Map.Entry<String, ChannelHandlerContext>> iter = nettyClients.entrySet().iterator();
			while(iter.hasNext())
			{
				Map.Entry<String, ChannelHandlerContext> entry = iter.next();
				
				ChannelHandlerContext nettyClient = entry.getValue();
				nettyClient.getChannel().write(generateStopReceivePacket());
			}
		}
		
	}
	
	/**
	 * 
	 * @Description: 退出系统
	 */
	public void shutdown() {
		
		//告诉MsgServer，停止发送新的数据，并断开连接
		nettyStopReceivePacket();
		
		//断开netty
		NetworkManager networkManager = NetworkManager.getInstance();
		if(networkManager != null) {
			
			//检查是否还有work thread，如果有，sleep
			ExecutorService executor = NetworkManager.getInstance().getNettyWorkerTheadPool();
			//可能需要先暂停，再调用awaitTermination
			executor.shutdown();
			try {
				if( executor != null && !executor.awaitTermination(10, TimeUnit.SECONDS))
				{
					executor.shutdownNow();
					logger.error("Unable to completed all netty works");
				}
			} catch (InterruptedException e) {
				executor.shutdownNow();
				logger.error("", e);
			}
			
			networkManager.shutdown();
		}
		
		//关闭Timer
		TimerManager timerManager = TimerManager.getInstance();
		if(timerManager != null) {
			
			timerManager.shutDown();
			
			//检查timer是否关闭完成了，如果没有，则sleep
			timerManager.checkTasksCompleted();
			
		}
		
		//关闭DB
		DBManager dbManager = DBManager.getInstance();
		if(dbManager != null) {
			DBManager.getInstance().shutDown();
		}
		//关闭cache 
		CacheManager cacheManager = CacheManager.getInstance();
		if(cacheManager != null) {
			CacheManager.getInstance().shutDown();
		}
	}
	
	/**
	 * 获取shutdown的文件路径
	 * @return
	 */
	public String getShutDownFilePath() 
	{
		String filePath = null;
		if(port > 0)
		{
			filePath = SysConstants.ELEGANT_STOP_FILE + port;
		}
		return filePath;
	}
	
	/**
	 * 生成shutdown文件
	 */
	public void generateShutDownShellFile()
	{
		if(port > 0)
		{
			String shutDownShell = "echo " + SysConstants.ELEGANT_STOP_CONTENT + " > " + getShutDownFilePath();
			FileWriter fileWriter = null;
			try {
				File shellFile = new File(SysConstants.ELEGANT_SHUTDOWN_SHELL_SCRIPT + port + ".sh");
				if( ! shellFile.exists())
				{
					shellFile.createNewFile();
				}
				fileWriter = new FileWriter(shellFile);
				fileWriter.write(shutDownShell);
				fileWriter.flush();
			} catch (IOException e) {
				logger.error("", e);
			} finally {
				if(fileWriter != null)
				{
					try {
						fileWriter.close();
					} catch (IOException e) {
						logger.error(" ", e);
					}
				}
			}
		}
	}
	
	/**
	 * 
	 * @Description: 启动检查是否关闭storage的线程
	 */
	public void startCheckShutdownThread()
	{
		new Thread(
			new Runnable() {
				@Override
				public void run() {
					
					while(true)
					{
						if(isStopCurrentJavaInstance())	//是否需要停止
						{
							shutdown();
							//关闭该线程
							break;
						}
						try {
							Thread.sleep(SysConstants.CHECK_STOP_GAP_TIME);
						} catch (InterruptedException e) {
							logger.error("", e);
						}
					}
				}
			}
		).start();
	}

}
