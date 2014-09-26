package com.mogujie.ares.lib.net;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffers;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.codec.frame.FrameDecoder;

import com.mogujie.ares.configure.SysConstants;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;

/**
 * 
 * @Description: 数据包解析器
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午3:44:58
 * 
 */
public class FrameBinaryDecoder extends FrameDecoder {

    private static final Logger logger = LoggerFactory
            .getLogger(FrameBinaryDecoder.class);

    /**
     * 解析数据包，主要负责解析数据包前8个字节统一格式的头部信息，生成Packet对象， 剩余的数据部分的解析在后面具体的action处理
     */
    @Override
    protected Object decode(ChannelHandlerContext ctx, Channel channel,
            ChannelBuffer buffer) throws Exception {

        if (buffer.readableBytes() < SysConstants.PROTOCOL_HEADER_LENGTH) {
            return null;
        }

        buffer.markReaderIndex(); // 标记一下
        int length = buffer.readInt(); // 消息长度
        char serviceId = buffer.readChar(); // 服务号, 后端这边固定1000
        char commandId = buffer.readChar(); // 命令号
        char version = buffer.readChar(); // 消息version, 1个字节
        char reserved = buffer.readChar(); // 保留，可用户如序列号等
        int contentLength = length - SysConstants.PROTOCOL_HEADER_LENGTH; // 计算数据包中业务数据部分的长度(去除头部长度16)

        if (buffer.readableBytes() < contentLength) {
            logger.info("数据长度：" + contentLength);
            buffer.resetReaderIndex(); // 返回到上面标记的位置
            return null;
        }

        ChannelBuffer cBuffer = ChannelBuffers.buffer(contentLength);
        buffer.readBytes(cBuffer, contentLength); // 转移所有业务部分的数据到新的byte

        Packet packet = new Packet();
        packet.setLength(contentLength);
        packet.setServiceId(serviceId);
        packet.setCommandId(commandId);
        packet.setVersion(version);
        packet.setReserved(reserved);
        DataBuffer dataBuffer = new DataBuffer(cBuffer); // 数据部分
        packet.setContentBuffer(dataBuffer);

        // logger.info("decode packet serviceId : " + packet.getServiceId()
        // + " commandId: " + packet.getCommandId());
        return packet;
    }

}
