package com.mogujie.ares.extend.action;

import java.util.List;

import com.mogujie.ares.data.TransmitFile;
import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;
import com.mogujie.ares.model.FileModel;
import com.mogujie.ares.util.MoguUtil;

/**
 * 
 * @Description: 用户相关的类
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-8-12 下午4:55:45
 * 
 */
public class FileAction extends BaseAction {

    private static final Logger logger = LoggerFactory
            .getLogger(FileAction.class);

    /*
     * 
     * @Description: 存储离线文件的信息
     * 
     * @param fromUserId
     * 
     * @param userIds
     * 
     * @return
     */
    public DataBuffer addFileRecord(int commandId, int requestId,
            int fromUserId, int toUserId, String taskId, String filePath, int fileSize,
            int version) {
        logger.info("add file recored : " + "fromUserId=" + fromUserId
                + ", toUserId=" + toUserId + ", taskId = " + taskId + ", filePath=" + filePath);
        DataBuffer buffer;
        if (fromUserId <= 0 || toUserId <= 0 || filePath == null
                || "".equals(filePath)) {
            buffer = new DataBuffer();
            buffer.writeInt(requestId); // request uuid
            buffer.writeInt(1); // result
            buffer.writeInt(commandId); // source commandId
            buffer.writeInt(fromUserId); // fromUserId
            buffer.writeInt(toUserId); // toUserId
            return buffer;
        }

        buffer = new DataBuffer();
        buffer.writeInt(requestId); // request uuid
        boolean isSuccess = false;
        isSuccess = FileModel.getInstance().saveFileRecord(fromUserId,
                toUserId, taskId, filePath, fileSize);
        int result = isSuccess ? 0 : 2;
        buffer.writeInt(result); // result
        buffer.writeChar((char) commandId); // source commandId
        buffer.writeInt(fromUserId); // fromUserId
        buffer.writeInt(toUserId); // toUserId
        return buffer;
    }

    /*
     * 
     * @Description: 删除离线文件的信息
     * 
     * @param fromUserId
     * 
     * @param userIds
     * 
     * @return
     */
    public DataBuffer removeFileRecord(int commandId, int requestId,
            int fromUserId, int toUserId, String taskId, int version) {
        logger.info("remove file recored : " + "taskId=" + taskId);
        DataBuffer buffer;
        if (null == taskId || taskId.isEmpty()) {
            buffer = new DataBuffer();
            buffer.writeInt(requestId); // request uuid
            buffer.writeInt(1); // result
            buffer.writeChar((char) commandId);
            buffer.writeInt(fromUserId); // fromUserId
            buffer.writeInt(toUserId); // toUserId
            return buffer;
        }

        buffer = new DataBuffer();
        buffer.writeInt(requestId); // request uuid
        TransmitFile file = FileModel.getInstance().getFileRecord(taskId);
        boolean isSuccess = false;
        int now = (int) (System.currentTimeMillis() / 1000);
        if (file != null && file.getStatus() == 1
                && file.getCreated() > now - 7 * 86400) {
            isSuccess = FileModel.getInstance().deleteFileRecord(taskId);
        }
        int result = isSuccess ? 0 : 2;
        buffer.writeInt(result); // result
        buffer.writeChar((char) commandId);
        buffer.writeInt(fromUserId); // fromUserId
        buffer.writeInt(toUserId); // toUserId
        return buffer;
    }

    /*
     * 
     * @Description: 存储或删除立宪文件的信息
     * 
     * @param fromUserId
     * 
     * @param userIds
     * 
     * @return
     */
    public DataBuffer getFileRecord(int requestUserId, DataBuffer attachment, int version) {
        logger.info("-->> getFileRecord ->> get file recored : "
                + requestUserId);
        DataBuffer buffer;
        if (requestUserId <= 0) {
            buffer = new DataBuffer();
            buffer.writeInt(requestUserId); // request uuid
            buffer.writeInt(0); // file count
            return MoguUtil.writeAttachments(buffer, attachment);
        }

        buffer = new DataBuffer();
        List<TransmitFile> fileList = FileModel.getInstance().getUserFiles(
                requestUserId); // 这里不会返回null
        int size = fileList.size();
        buffer.writeInt(requestUserId); // request userId
        buffer.writeInt(size);
        TransmitFile file = null;
        for (int i = 0; i < size; i++) {
            file = fileList.get(i);
            buffer.writeInt(file.getFromUserId());
            buffer.writeString(file.getTaskId());
            buffer.writeString(file.getFilePath());
            buffer.writeInt(file.getFileSize());
        }
        return MoguUtil.writeAttachments(buffer, attachment);
    }

}
