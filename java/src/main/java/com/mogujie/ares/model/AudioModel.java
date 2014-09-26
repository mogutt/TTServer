/**
 * unread construct
 * userId => {
 *   firendUserId => count
 * }
 * key construct : u + userId
 * 
 * markread construct
 * userId => {
 * 	 firendUserId => count
 *   ... 	
 * }
 * key construct :  m + userId
 * 
 */
package com.mogujie.ares.model;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.lang.StringUtils;

import com.mogujie.ares.util.MoguArrayUtil;
import com.mogujie.ares.util.MoguByteUtil;
import com.mogujie.ares.data.Audio;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.DBManager;
import com.mogujie.ares.manager.DBManager.DBPoolName;
import com.mogujie.ares.manager.FileManager;

/**
 * 
 * @Description: 计数器相关操作
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 下午2:19:39
 *
 */
public class AudioModel
{
	
	private static AudioModel instance;
	private static final Logger logger = LoggerFactory.getLogger(AudioModel.class);
	
	
	public static AudioModel getInstance()
	{
		if(instance == null)
		{
			instance = new AudioModel();
		}
		return instance;
	}
	
	private AudioModel(){}
	
	/**
	 * 
	 * @Description: 保持一个语音文件并返回语音路径
	 * @param bytes
	 * @param userId
	 * @param toUserId
	 * @return 保持的audioid
	 */
	public Audio saveAudio(int userId, int toUserId, Audio audio, int created) {

		if(audio == null || audio.getData() == null 
				|| audio.getData().length <= 0 || audio.getCostTime() <= 0) {
			return null;
		}
		
		byte[] data = audio.getData();
		String path = FileManager.getInstance().saveAudioBinary(data);
		if(StringUtils.isEmpty(path)) {
			return null;
		}
		audio.setCreated(created);
		audio.setFileSize(data.length);
		audio.setPath(path);
		audio.setUserId(userId);
		audio.setToUserId(toUserId);
		int id = saveAudioInfo(audio);
		audio.setId(id);
		return audio;
	}
	
	public Audio parseAudio(byte[] bytes) {
		if(bytes.length <= 4) { return null; }
		Audio audio = null;
		try {
			byte[] costByte = Arrays.copyOfRange(bytes, 0, 4);
			byte[] data = Arrays.copyOfRange(bytes, 4, bytes.length);
			int cost = MoguByteUtil.getInstance().convert2Int(costByte);
			if(cost <= 0) {
				return null;
			}
			audio = new Audio();
			audio.setCostTime(cost);
			audio.setData(data);
		} catch(Exception e) {
			return null;
		}
		return audio;
	}
	
	/**
	 * 
	 * @Description: 数据库记录语音文件的路径
	 * @param userId
	 * @param toUserId
	 * @param path
	 * @param size
	 * @param created
	 * @return
	 */
	private int saveAudioInfo(Audio audio) {
		DBManager dbManager = DBManager.getInstance();
		Connection conn = dbManager.getConnection(DBPoolName.macim_master);
		PreparedStatement statement = null;
		ResultSet rs = null;
		int succCount = 0;
		int id = 0;
		try {
			String sql = "insert into IMAudio(`userId`, `toUserId`, `path`, `fileSize`, `costTime`, `created`) " +
					"values(?, ?, ?, ?, ?, ?)";
			statement = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS);
			int index = 1;
			statement.setInt(index++, audio.getUserId());
			statement.setInt(index++, audio.getToUserId());
			statement.setString(index++, audio.getPath());
			statement.setInt(index++, audio.getFileSize());
			statement.setInt(index++, audio.getCostTime());
			statement.setInt(index++, audio.getCreated());
			succCount = statement.executeUpdate();
			if(succCount > 0) {
				rs = statement.getGeneratedKeys();
				if(rs.next()) {
					id = rs.getInt(1);
				}
			}
		} catch (SQLException e) {
			logger.error(audio.getUserId() + " - " + audio.getToUserId() + " - " + audio.getPath(), e);
		} finally {
			dbManager.release(DBPoolName.macim_master, conn, statement, null);
		}
		
		return id;
	}
	
	/**
	 * 
	 * @Description: 读取语音额外信息，除语音内容外
	 * @param audioIds
	 * @return
	 */
	public Map<Integer, Audio> getAudiosInfo(int[] audioIds) {
		Map<Integer, Audio> audioMap = new HashMap<Integer, Audio>();
		if(audioIds == null || audioIds.length == 0) {
			return audioMap;
		}
		DBManager dbManager = DBManager.getInstance();
		Connection conn = dbManager.getConnection(DBPoolName.macim_slave);
		PreparedStatement statement = null;
		ResultSet rs = null;
		try {
			StringBuilder sb = new StringBuilder("select * from IMAudio where id in (");
			int length = audioIds.length;
			int cnt = 0;
			for(int i = 0; i < length; i++) {
				if(audioIds[i] <= 0) {continue;}
				sb.append("?,");
				cnt++;
			}
			if(cnt <= 0) {
				return audioMap;
			}
			String sql = sb.subSequence(0, sb.length() - 1) + ")";
			statement = conn.prepareStatement(sql);
			int index = 1;
			for(int i = 0; i < length; i++) {
				if(audioIds[i] <= 0) {continue;}
				statement.setInt(index++, audioIds[i]);
			}
			rs = statement.executeQuery();

			Audio audio;
			while(rs.next()) {
				audio = new Audio();
				audio.setId(rs.getInt("id"));
				audio.setUserId(rs.getInt("userId"));
				audio.setToUserId(rs.getInt("toUserId"));
				audio.setPath(rs.getString("path"));
				audio.setFileSize(rs.getInt("fileSize"));
				audio.setCostTime(rs.getInt("costTime"));
				audio.setCreated(rs.getInt("created"));
				audioMap.put(audio.getId(), audio);
			}
		} catch (SQLException e) {
			logger.error(Arrays.toString(audioIds), e);
		} finally {
			dbManager.release(DBPoolName.macim_slave, conn, statement, rs);
		}
		
		return audioMap;
	}
	
	/**
	 * 
	 * @Description: 读取一段语音的详细信息，包括语音文件内容
	 * @param audioIds
	 * @return
	 */
	public Map<Integer, Audio> readAudios(int[] audioIds) {
		Map<Integer, Audio> audioMap = new HashMap<Integer, Audio>();
		if(audioIds == null || audioIds.length == 0) {
			return audioMap;
		}
		audioIds = MoguArrayUtil.getInstance().arrayUnique(audioIds);
		audioMap = getAudiosInfo(audioIds);
		logger.info("read audios:" + Arrays.toString(audioIds));
		if(!audioMap.isEmpty()) {
			Iterator<Audio> iter = audioMap.values().iterator();
			Audio audio = null;
			byte[] data = null;
			while(iter.hasNext()) {
				audio = iter.next();
				data = FileManager.getInstance().readAudioBinary(audio.getPath());
				if(data != null) {
					audio.setData(data);
					audioMap.put(audio.getId(), audio);
				}
				
			}
		}
		
		return audioMap;
	}
	
	public byte[] getAudioMessageContent(Audio audio) {
		if(audio == null || audio.getData() == null 
				|| audio.getData().length <= 0 || audio.getCostTime() <= 0) {
			return new byte[0];
		}
		byte[] data = audio.getData();
		byte[] bytes = new byte[data.length + 4];
		byte[] intBytes = MoguByteUtil.getInstance().getBytes(audio.getCostTime());
		System.arraycopy(intBytes, 0, bytes, 0, 4);
		System.arraycopy(data, 0, bytes, 4, data.length);
		return bytes;
	}
}
