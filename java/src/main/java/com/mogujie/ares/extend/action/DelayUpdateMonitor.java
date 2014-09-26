package com.mogujie.ares.extend.action;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.model.GroupModel;
import com.mogujie.ares.model.RelationshipModel;

public class DelayUpdateMonitor {
	public static final int DEFAULT_PACKET_SEND_MONTOR_INTERVAL = 30 * 1000;

    private Monitor mMonitor;
    private  ConcurrentHashMap<Integer, Integer> mGroupForUpdateMap = new ConcurrentHashMap<Integer, Integer>();
    private  ConcurrentHashMap<String, Integer> mPersonalForUpdateMap = new ConcurrentHashMap<String, Integer>();
    private volatile boolean mNeedStop;
    private volatile boolean mStarted = false;
    private static final Logger logger = LoggerFactory
            .getLogger(MessageContent.class);
    private static DelayUpdateMonitor instance;

	public static DelayUpdateMonitor getInstance() {
		if (null == instance) {
			instance = new DelayUpdateMonitor();
		}
		return instance;
	}

	private DelayUpdateMonitor() {
            mMonitor = new Monitor("TT-Delay-Update-Monitor",
                    DEFAULT_PACKET_SEND_MONTOR_INTERVAL);
    }

    public synchronized void start() {
        if (mStarted)
            return;
        mNeedStop = false;
        mMonitor.start();
        mStarted = true;
    }

    public synchronized void stop() {
        if (mNeedStop) {
            return;
        }
        mNeedStop = true;
        mStarted = false;
    }

    public void AddPersonalUpdate(int uA, int uB, int time) {
    	int small = uA < uB ? uA : uB;
    	int big = uA + uB - small;
    	String sKey = small + ":" + big;
    	logger.info("user " + uA + " and " + uB + "wants to update to :" + time);
    	synchronized(mPersonalForUpdateMap) {
    		if (mPersonalForUpdateMap.containsKey(sKey)) {
    			mPersonalForUpdateMap.remove(sKey);
            }
    		mPersonalForUpdateMap.put(sKey, time);
    	}
    }
    
    public void AddGroupUpdate(int groupId, int time) {
    	logger.info("Group " + groupId + "wants to update to :" + time);
    	synchronized(mGroupForUpdateMap) {
    		if (mGroupForUpdateMap.containsKey(groupId)) {
    			mGroupForUpdateMap.remove(groupId);
            }
    		mGroupForUpdateMap.put(groupId, time);
    	}
    }
    
    private class Monitor extends Thread {

        private int mInterval;
        public Monitor(String name, int interval) {
            setName(name);
            mInterval = interval;
        }

        @Override
        public void run() {
            super.run();
            try {
                while (!mNeedStop) {
                	ConcurrentHashMap<Integer, Integer> oldGroupMap = new ConcurrentHashMap<Integer, Integer>();
                	synchronized(mGroupForUpdateMap) {
                		ConcurrentHashMap<Integer, Integer> tmpMap;
                		tmpMap = mGroupForUpdateMap;
                		mGroupForUpdateMap = oldGroupMap;
                		oldGroupMap = tmpMap;
                	}
                	GroupModel groupModel = GroupModel.getInstance();
                	for(Map.Entry<Integer,Integer> e: oldGroupMap.entrySet() ){
                		int groupId = e.getKey();
                		int time = e.getValue();
                		groupModel.updateGroupTime(groupId, time);
                		logger.info("Group " + groupId + " update to :" + time);
                	}
                	
                	ConcurrentHashMap<String, Integer> oldPersonalMap = new ConcurrentHashMap<String, Integer>();
                	synchronized(mPersonalForUpdateMap) {
                		ConcurrentHashMap<String, Integer> tmpMap;
                		tmpMap = mPersonalForUpdateMap;
                		mPersonalForUpdateMap = oldPersonalMap;
                		oldPersonalMap = tmpMap;
                	}
                	
                	RelationshipModel relationModel = RelationshipModel.getInstance();
                	for(Map.Entry<String,Integer> e: oldPersonalMap.entrySet() ){
                		String[] resultString = e.getKey().split(":");
                		int uA = Integer.parseInt(resultString[0]);
                		int uB = Integer.parseInt(resultString[1]);
                		int time = e.getValue();
                		relationModel.updateRelationShip(uA, uB, time);
                		logger.info("user " + uA + " and " + uB + " update to :" + time);
                	}
                	
                    sleep(mInterval);
                }
            } catch (Exception e) {
            }
        }
    }
}
