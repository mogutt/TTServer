package com.mogujie.ares.lib.net;

import java.nio.ByteBuffer;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffers;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelDownstreamHandler;

import com.mogujie.ares.configure.SysConstants;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;

/**
 * 
 * @Description: 数据编码器，把要发送的数据按照一定格式拼接成二进制的流
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午4:02:07
 * 
 */
public class FrameBinaryEncoder extends SimpleChannelDownstreamHandler {
    @SuppressWarnings("unused")
	private static final Logger logger = LoggerFactory
            .getLogger(FrameBinaryEncoder.class);

    /**
     * @Description: 把要发送的数据编码成二进制数据并发送
     */
    public void writeRequested(ChannelHandlerContext ctx, MessageEvent e)
            throws Exception {

        Packet request = (Packet) e.getMessage();
        // encode
        DataBuffer contentBuffer = request.getContentBuffer();
        ChannelBuffer dataBuffer = null;
        if (contentBuffer != null) {
            dataBuffer = contentBuffer.getOrignalBuffer();
        } // 数据部分
        ChannelBuffer totalBuffer;
        ByteBuffer headBuffer = ByteBuffer
                .allocate(SysConstants.PROTOCOL_HEADER_LENGTH);
        if (dataBuffer != null) {
            int length = dataBuffer.readableBytes()
                    + SysConstants.PROTOCOL_HEADER_LENGTH; // 总的包长
            headBuffer.putInt(length);// 消息长度
            char[] tc = Character.toChars(request.getServiceId());
            headBuffer.putChar(tc[0]); // 服务号, 后端这边固定1000
            tc = Character.toChars(request.getCommandId());
            headBuffer.putChar(tc[0]); // 命令号
            tc = Character.toChars(request.getVersion());
            headBuffer.putChar(tc[0]);// 消息version, 1个字节
            tc = Character.toChars(request.getReserved());
            headBuffer.putChar(tc[0]); // 保留，可用于序列号等
            headBuffer.flip(); // 这里需要重置一下游标和长度

            totalBuffer = ChannelBuffers.dynamicBuffer();
            totalBuffer.writeBytes(headBuffer);
            if (length > SysConstants.PROTOCOL_HEADER_LENGTH) { // 数据部分长度不为0才写入
                int dataLength = dataBuffer.readableBytes();
                byte[] data = new byte[dataLength];
                dataBuffer.readBytes(data);
                totalBuffer.writeBytes(data);
            }
        } else {
            int length = SysConstants.PROTOCOL_HEADER_LENGTH; // 总的包长
            headBuffer.putInt(length);// 消息长度
            char[] tc = Character.toChars(request.getServiceId());
            headBuffer.putChar(tc[0]); // 服务号, 后端这边固定1000
            tc = Character.toChars(request.getCommandId());
            headBuffer.putChar(tc[0]); // 命令号 TODO
            headBuffer.put((byte) request.getVersion());// 消息version, 1个字节
            tc = Character.toChars(request.getReserved());
            headBuffer.putChar(tc[0]); // 保留，可用户如序列号等
            headBuffer.flip(); // 这里需要重置一下游标和长度

            totalBuffer = ChannelBuffers.copiedBuffer(headBuffer);
        }
        // logger.info("encode serviceId : " + request.getServiceId()
        // + " commandId: " + request.getCommandId());
        Channels.write(ctx, e.getFuture(), totalBuffer);
        //logger.info("发送totalBuffer : " + totalBuffer.toString(Charset.defaultCharset()));
    }

}
