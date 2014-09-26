package com.mogujie.ares.manager;

import org.apache.commons.lang.StringUtils;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.MoguHttp;

public class FileManager {

	private static final Logger logger = LoggerFactory.getLogger(FileManager.class);
	private static FileManager instance = new FileManager();
	
	private String audioUploadUrl = BizConstants.URL_FILE_UPLOAD; // 文件上传路径
	
	private String audioDownloadUrl = BizConstants.URL_FILE_DOWNLOAD; // 文件下载路径
	
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
		
		String fileName = MoguHttp.uploadAudioByteFile(this.audioUploadUrl, bytes);
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
		String strDownloadUrl = this.audioDownloadUrl + fileName;
		logger.info("下载文件:" + strDownloadUrl);
		byte[] bytes = MoguHttp.downloadByteFile(strDownloadUrl);
		
		return bytes;
	}
}
