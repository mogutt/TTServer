package com.mogujie.ares.extend.action;

import com.mogujie.ares.extend.BaseAction;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.DataBuffer;

public class Monitor extends BaseAction {

    @SuppressWarnings("unused")
    private static final Logger logger = LoggerFactory.getLogger(Monitor.class);

    /**
     * 
     * @Description: heartbeat
     * @param clientAddress
     * @return
     */
    public DataBuffer heartbeat(String clientAddress, int version) {
        DataBuffer dataBuffer = new DataBuffer(0);
        return dataBuffer; // 不用返回
    }
}