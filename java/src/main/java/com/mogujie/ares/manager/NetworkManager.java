package com.mogujie.ares.manager;

import java.net.InetSocketAddress;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.jboss.netty.bootstrap.ServerBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.SimpleChannelHandler;
import org.jboss.netty.channel.group.ChannelGroup;
import org.jboss.netty.channel.group.ChannelGroupFuture;
import org.jboss.netty.channel.group.DefaultChannelGroup;
import org.jboss.netty.channel.socket.nio.NioServerSocketChannelFactory;
//import org.jboss.netty.handler.codec.frame.LengthFieldBasedFrameDecoder;
import org.jboss.netty.handler.execution.ExecutionHandler;
import org.jboss.netty.handler.execution.OrderedMemoryAwareThreadPoolExecutor;

import com.mogujie.ares.configure.Router;
import com.mogujie.ares.configure.SysConstants;
import com.mogujie.ares.extend.ActionHolder;
import com.mogujie.ares.extend.dispatch.DefaultRequestDispatcher;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.BinaryMessageHandler;
import com.mogujie.ares.lib.net.FrameBinaryDecoder;
import com.mogujie.ares.lib.net.FrameBinaryEncoder;
import com.mogujie.ares.lib.net.IDispatcher;

/**
 * 
 * @Description: 网络管理
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 下午8:29:01
 * 
 */
public class NetworkManager {

    private static final Logger logger = LoggerFactory
            .getLogger(NetworkManager.class);

    private static NetworkManager manager = new NetworkManager();

    private int port = SysConstants.SERVER_DEFAULT_PORT; // 服务器端口

    private ChannelGroup channelGroup; // 打开的所有channel的群组，用户关闭

    private ServerBootstrap bootstrap; // server

    // 所有连接着的客户端的列表
    private ConcurrentHashMap<String, ChannelHandlerContext> clientMap = new ConcurrentHashMap<String, ChannelHandlerContext>();

    private ExecutorService nettyBossThreadPool; // netty boss线程池

    private ExecutorService nettyWorkerTheadPool; // netty worker线程池

    public static NetworkManager getInstance() {
        return manager;
    }

    private NetworkManager() {

    }

    /**
     * 
     * @Description: 初始化
     * @param port
     *            服务器端口
     * @throws Exception
     */
    public void init(int port) throws Exception {

        // 消息分发器
        IDispatcher dispatcher = initDispatcher();
        // 消息接收器
        final SimpleChannelHandler channelHandler = new BinaryMessageHandler(
                dispatcher);

        final ExecutionHandler executionHandler = new ExecutionHandler(
                new OrderedMemoryAwareThreadPoolExecutor(16, 1048576, 1048576));

        this.port = port;

        nettyBossThreadPool = Executors.newCachedThreadPool(); // boss线程池，用于接收链接
        nettyWorkerTheadPool = Executors.newCachedThreadPool(); // worker线程池，boss线程接收请求之后由worker线程处理请求
        ChannelFactory factory = new NioServerSocketChannelFactory(
                nettyBossThreadPool, nettyWorkerTheadPool);

        bootstrap = new ServerBootstrap(factory);
        bootstrap.setPipelineFactory(new ChannelPipelineFactory() {
            @Override
            public ChannelPipeline getPipeline() throws Exception {
                ChannelPipeline pipeline = Channels.pipeline();
                // 接收的数据包解码
                pipeline.addLast("decoder", new FrameBinaryDecoder());
                // 发送的数据包编码
                pipeline.addLast("encoder", new FrameBinaryEncoder());

                pipeline.addLast("execution", executionHandler);

                // 具体的业务处理，这个handler只负责接收数据，并传递给dispatcher
                pipeline.addLast("handler", channelHandler);

                return pipeline;
            }

        });
        bootstrap.setOption("child.tcpNoDelay", true); // 无延迟发生
        bootstrap.setOption("child.keepAlive", true); // tcp长连

        Channel serverChannel = bootstrap
                .bind(new InetSocketAddress(this.port)); // 打开网络端口

        channelGroup = new DefaultChannelGroup(); // 添加到group
        channelGroup.add(serverChannel);

        logger.info("server started on port " + this.port);
    }

    /**
     * 
     * @Description: 初始化消息分发器
     * @return
     * @throws Exception
     */
    public IDispatcher initDispatcher() throws Exception {
        ActionHolder actionHolder = new ActionHolder();
        Router router = ConfigureManager.getInstance().getActionRouter();
        actionHolder.put(router);
        IDispatcher dispatcher = new DefaultRequestDispatcher(actionHolder);

        return dispatcher;
    }

    /**
     * @Description: 关闭网络
     */
    public void shutdown() {
        try {
            if (channelGroup != null) {
                channelGroup.close().awaitUninterruptibly();
            }
            if (bootstrap != null) {
                bootstrap.releaseExternalResources();
            }
            logger.info("shutdown...");
        } catch (Exception e) {
            logger.error(e.getMessage(), e);
        }
    }

    /**
     * @Description: 关闭网络
     */
    public void shutdown(long milliSecondTimeOut) {
        try {
            if (channelGroup != null) {
                ChannelGroupFuture future = channelGroup.close();
                future.awaitUninterruptibly(milliSecondTimeOut);
            }
            if (bootstrap != null) {
                bootstrap.releaseExternalResources();
            }
            logger.info("shutdown..." + milliSecondTimeOut);
        } catch (Exception e) {
            logger.error(e.getMessage(), e);
        }
    }

    /**
     * 
     * @Description: 连接着的客户端的列表
     * @return
     */
    public ConcurrentHashMap<String, ChannelHandlerContext> getClientMap() {
        return clientMap;
    }

    /**
     * 
     * @Description: 新的连接进来，添加到clientmap
     * @param context
     */
    public void addClient(ChannelHandlerContext context) {
        if (context == null) {
            return;
        }
        String name = context.getChannel().getRemoteAddress().toString();
        clientMap.putIfAbsent(name, context);
    }

    /**
     * 
     * @Description: 根据名字去除一个客户端连接
     * @param name
     */
    public void removeClient(String name) {
        if (name == null || name.equals("")) {
            return;
        }

        clientMap.remove(name);
    }

    /**
     * 
     * @Description: 删除一个链接的客户端
     * @param context
     */
    public void removeClient(ChannelHandlerContext context) {
        if (context == null) {
            return;
        }
        String name = context.getChannel().getRemoteAddress().toString();
        clientMap.remove(name);
    }

    /**
     * 
     * @Description: 返回工作线程
     * @return
     */
    public ExecutorService getNettyWorkerTheadPool() {
        return nettyWorkerTheadPool;
    }
}
