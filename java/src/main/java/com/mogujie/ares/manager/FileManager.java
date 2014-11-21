package com.mogujie.ares.manager;

import org.apache.commons.lang.StringUtils;

import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.MoguHttp;

public class FileManager {

	public static String fileServerUrl = "";
	
	private static final Logger logger = LoggerFactory.getLogger(FileManager.class);
	private static FileManager instance = new FileManager();
	
	public static FileManager getInstance()
	{
		if(instance == null)
		{
			instance = new FileManager();
		}
		return instance;
	}
	
	/**
	 * 
	 * @Description: 返回音频文件
	 * @param bytes
	 * @param userId
	 * @return
	 */
	public String saveAudioBinary(byte[] bytes) {

		if(bytes == null || bytes.length == 0) {
			return "";
		}
		
		String fileName = MoguHttp.uploadAudioByteFile(FileManager.fileServerUrl, bytes);
		if(StringUtils.isEmpty(fileName)) {
			logger.info("保存语音文件失败"); 
		} else {
			logger.info("保存语音文件成功:" + fileName);
		}
		return fileName;
	}
	
	/**
	 * 
	 * @Description: 从磁盘读取一个文件
	 * @param fileName
	 * @return
	 */
	public byte[] readAudioBinary(String fileName) {
		if(StringUtils.isEmpty(fileName)) {
			return null;
		}
		String strDownloadUrl = FileManager.fileServerUrl + fileName;
		logger.info("下载文件:" + strDownloadUrl);
		byte[] bytes = MoguHttp.downloadByteFile(strDownloadUrl);
		
		return bytes;
	}
}
