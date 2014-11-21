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

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import redis.clients.jedis.Jedis;

import com.mogujie.ares.configure.BizConstants;
import com.mogujie.ares.data.ClientType;
import com.mogujie.ares.data.Counter;
import com.mogujie.ares.data.GroupCounterItem;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.manager.CacheManager;
import com.mogujie.ares.manager.CacheManager.CachePoolName;

/*
 * 
 * @Description: 计数器相关操作
 * @author shitou - shitou[at]mogujie.com
 * @date 2013-7-22 下午2:19:39
 *
 */
public class CounterModel {

    private static CounterModel counterModelInstance;
    private static final Logger logger = LoggerFactory
            .getLogger(CounterModel.class);

    public static CounterModel getInstance() {
        if (counterModelInstance == null) {
            counterModelInstance = new CounterModel();
        }
        return counterModelInstance;
    }

    private CounterModel() {
    }

    /*
     * 
     * @Description: 发了一条消息，同时新增发送和接收用户的消息计数器
     * 
     * @param fromUserId 发送的用户
     * 
     * @param toUserId 接收的用户
     */
    public void incrUserMsgCount(int fromUserId, int toUserId) {
        if (fromUserId == 0 || toUserId == 0) {
            return;
        }
        CacheManager cacheManager = CacheManager.getInstance();
        Jedis counterInstance = null;
        try {
            counterInstance = cacheManager.getResource(CachePoolName.counter);
            counterInstance.hincrBy(String.valueOf(fromUserId),
                    String.valueOf(toUserId), 1);
            counterInstance.hincrBy(String.valueOf(toUserId),
                    String.valueOf(fromUserId), 1);
        } catch (Exception e) {
            logger.error(fromUserId + " : " + toUserId, e);
        } finally {
            cacheManager.returnResource(CachePoolName.counter, counterInstance);
        }
    }

    /*
     * @Description: set counter value
     * 
     * @param userId
     * 
     * @param set_count
     */
    public void setMsgCount(int userId, HashMap<String, String> set_count) {
        if (userId == 0 || set_count.size() == 0) {
            return;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        Jedis counterInstance = null;
        try {
            counterInstance = cacheManager.getResource(CachePoolName.counter);
            counterInstance.hmset(String.valueOf(userId), set_count);
        } catch (Exception e) {
            logger.error(userId + " : " + set_count.toString(), e);
        } finally {
            cacheManager.returnResource(CachePoolName.counter, counterInstance);
        }
    }

    /*
     * 
     * @Description: 获得某用户对所有最近联系人的消息计数器
     * 
     * @param userId
     * 
     * @param items
     * 
     * @return
     */
    public Counter getUserMsgCounter(int userId, final String... items) {
        Counter userMsgCount = new Counter();
        if (userId == 0 || items.length == 0) {
            return userMsgCount;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        Jedis counterInstance = null;
        List<String> userCounter = null;
        try {
            counterInstance = cacheManager.getResource(CachePoolName.counter);
            userCounter = counterInstance.hmget(String.valueOf(userId), items);
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.counter, counterInstance);
        }

        if (userCounter == null || userCounter.size() == 0) {
            return userMsgCount;
        }

        Map<String, Integer> userMsgCountMap = new HashMap<String, Integer>();
        int loop = 0;
        for (String item : items) {
            userMsgCountMap.put(item, Integer.valueOf(userCounter.get(loop++)));
        }
        userMsgCount.setUserId(userId);
        userMsgCount.setMsgCount(userMsgCountMap);
        return userMsgCount;
    }

    /*
     * 
     * @Description: 获得用户来自他的某好友的未读消息
     * 
     * @param userId
     * 
     * @param friendUserId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public int getUserFriendUnreadCount(int userId, int friendUserId, int clientType) {
        int unreadCnt = 0;
        if (userId != 0 && friendUserId != 0) {
            CacheManager cacheManager = CacheManager.getInstance();
            Jedis counterInstance = null;
            String friendUnreadCnt = null;
            try {
                counterInstance = cacheManager
                        .getResource(CachePoolName.unread);
                friendUnreadCnt = counterInstance.hget(ClientType.prefixKeyOf(clientType) + String.valueOf(userId),
                        String.valueOf(friendUserId));
                logger.debug("shuchentest:" + friendUnreadCnt);
            } catch (Exception e) {
                logger.error("", e);
            } finally {
                cacheManager.returnResource(CachePoolName.unread,
                        counterInstance);
            }
            if (friendUnreadCnt != null) {
                unreadCnt = Integer.valueOf(friendUnreadCnt);
            }
        }
        return unreadCnt;
    }

    /*
     * 
     * @Description: 新增用户未读消息
     * 
     * @param fromUserId
     * 
     * @param toUserId
     * 
     * @param clientType 客户端类型@see ClientType
     */
    public void increaseUserUnreadMsgCount(int fromUserId, int toUserId) {
        if (fromUserId == 0 || toUserId == 0) {
            return;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        Jedis unreadInstance = null;
        try {
            unreadInstance = cacheManager.getResource(CachePoolName.unread);
            unreadInstance.hincrBy(ClientType.prefixKeyOf(ClientType.MAC) + String.valueOf(toUserId),
                    String.valueOf(fromUserId), 1);
            unreadInstance.hincrBy(ClientType.prefixKeyOf(ClientType.IOS) + String.valueOf(toUserId),
            		String.valueOf(fromUserId), 1);
            unreadInstance.hincrBy(ClientType.prefixKeyOf(ClientType.ANDROID) + String.valueOf(toUserId),
            		String.valueOf(fromUserId), 1);
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.unread, unreadInstance);
        }
    }

    /*
     * 
     * @Description: 获得用户所有的未读消息
     * 
     * @param userId
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public Counter getUnreadMsgCount(int userId, int clientType) {
        Counter userUnreadCount = new Counter();

        if (userId == 0) {
            return userUnreadCount;
        }
        CacheManager cacheManager = CacheManager.getInstance();
        Jedis unreadInstance = null;
        Map<String, String> userUnreadCountMap = new HashMap<String, String>();
        try {
            unreadInstance = cacheManager.getResource(CachePoolName.unread);
            userUnreadCountMap = unreadInstance.hgetAll(ClientType.prefixKeyOf(clientType) + String.valueOf(userId));
            logger.info( "shuchentest:" +String.valueOf(userId) + " : " + userUnreadCountMap);
	        logger.info( String.valueOf(userId) + " : " + userUnreadCountMap);
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.unread, unreadInstance);
        }

        Map<String, Integer> finalSaveUserUnreadCount = new HashMap<String, Integer>();
        Iterator<Map.Entry<String, String>> iter = userUnreadCountMap
                .entrySet().iterator();
        int limit = 0;
        while (iter.hasNext() && limit < 300) // 最多返回300人的未读消息计数.
        {
            limit++;
            Map.Entry<String, String> entry = iter.next();
            finalSaveUserUnreadCount.put(entry.getKey(),
                    Integer.valueOf(entry.getValue()));
        }
        userUnreadCountMap.clear();

        userUnreadCount.setUserId(userId);
        userUnreadCount.setUnreadCount(finalSaveUserUnreadCount);
        return userUnreadCount;
    }

    /*
     * 
     * @Description: 清除用户来自某好友的未读消息计数
     * 
     * @param userId
     * 
     * @param friendUserId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public boolean clearUserUnreadItemCount(int userId, int friendUserId, int clientType) {
        if (userId == 0 || friendUserId == 0) {
            return false;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        Jedis unreadInstance = null;
        try {
            unreadInstance = cacheManager.getResource(CachePoolName.unread);
            unreadInstance.hdel(ClientType.prefixKeyOf(clientType) + String.valueOf(userId),
                    String.valueOf(friendUserId));
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.unread, unreadInstance);
        }

        return true;
    }

    // 增加用户消息中心里的未读聊天消息计数.
    public void increaseChatNewCount(int toUserId) {
        if (toUserId == 0) {
            return;
        }
        // logger.info(" -- increaseChatNewCount --toUserId:"+toUserId
        // +" key1:"+SysConstants.REDIS_CINFO_PREFIX_KEY+toUserId+" key2:"+SysConstants.REDIS_CINFO_SUB_KEY);

        // CacheManager cacheManager = CacheManager.getInstance();
        // Jedis cinfoInstance = null;
        // Jedis cinfoPushInstance = null;
        // try {
        // cinfoInstance = cacheManager.getResource(CachePoolName.cinfo);
        // cinfoInstance.hincrBy(SysConstants.REDIS_CINFO_PREFIX_KEY+toUserId,SysConstants.REDIS_CINFO_SUB_KEY,
        // 1);
        // cinfoPushInstance =
        // cacheManager.getResource(CachePoolName.cinfo_push);
        // cinfoPushInstance.publish("hancock_channel_" + (toUserId / 10000000),
        // ""+toUserId);
        // } catch(Exception e) {
        // logger.error("", e);
        // } finally {
        // cacheManager.returnResource(CachePoolName.cinfo, cinfoInstance);
        // cacheManager.returnResource(CachePoolName.cinfo_push,
        // cinfoPushInstance);
        // }
    }

    // 更新用户消息中心里的未读聊天消息.
    public void updateChatNewCount(int userId) {
        // if(userId == 0)
        // {
        // return;
        // }
        // CacheManager cacheManager = CacheManager.getInstance();
        // Jedis unreadInstance = null;
        // Map<String, String> userUnreadCountMap = new HashMap<String,
        // String>();
        // try {
        // unreadInstance = cacheManager.getResource(CachePoolName.unread);
        // userUnreadCountMap = unreadInstance.hgetAll(String.valueOf(userId));
        // } catch(Exception e) {
        // logger.error("", e);
        // } finally {
        // cacheManager.returnResource(CachePoolName.unread, unreadInstance);
        // }
        //
        // int totalUnreadCount=0;
        // Iterator<Map.Entry<String, String>> iter =
        // userUnreadCountMap.entrySet().iterator();
        // while(iter.hasNext())
        // {
        // Map.Entry<String, String> entry = iter.next();
        // totalUnreadCount+= Integer.valueOf(entry.getValue());
        // }
        // userUnreadCountMap.clear();
        // Jedis cinfoInstance = null;
        // Jedis cinfoPushInstance = null;
        // try {
        // cinfoInstance = cacheManager.getResource(CachePoolName.cinfo);
        // cinfoInstance.hset(SysConstants.REDIS_CINFO_PREFIX_KEY+userId,SysConstants.REDIS_CINFO_SUB_KEY,
        // ""+totalUnreadCount);
        // cinfoPushInstance =
        // cacheManager.getResource(CachePoolName.cinfo_push);
        // cinfoPushInstance.publish("hancock_channel_" + (userId / 10000000),
        // ""+userId);
        // } catch(Exception e) {
        // logger.error("", e);
        // } finally {
        // cacheManager.returnResource(CachePoolName.cinfo, cinfoInstance);
        // cacheManager.returnResource(CachePoolName.cinfo_push,
        // cinfoPushInstance);
        // }

    }

    // ------------- 群组相关 -------------

    /*
     * 
     * @Description: 返回某用户在某个群里的未读消息计数
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public Map<Integer, Integer> getUserGroupUnreadCount(int userId,
            int[] groupIds, int clientType) {
        Map<Integer, Integer> counterMap = null;
        if (userId <= 0 || groupIds == null || groupIds.length == 0) {
            return new HashMap<Integer, Integer>();
        }
        counterMap = new HashMap<Integer, Integer>();
        int length = groupIds.length;
        int gid = 0;
        GroupCounterItem counterItem = null;
        for (int i = 0; i < length; i++) {
            gid = groupIds[i];
            if (gid <= 0) {
                counterMap.put(gid, 0);
                continue;
            }
            if (!counterMap.containsKey(gid)) {
                counterItem = getUserGroupCount(userId, gid, clientType);
                counterMap.put(gid, counterItem.getUserUnreadCount());
            }
        }

        return counterMap;
    }

    /*
     * 
     * @Description: 返回某用户在某群里的未读消息数
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    public GroupCounterItem getUserGroupCount(int userId, int groupId, int clientType) {
        if (userId <= 0 || groupId <= 0) {
            GroupCounterItem counterItem = new GroupCounterItem();
            counterItem.setUserId(userId);
            counterItem.setGroupId(groupId);
            counterItem.setGroupTotalCount(0);
            counterItem.setUserUnreadCount(0);
            counterItem.setLastMessageId(0);
            return counterItem;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        Jedis counterInstance = null;
        String strTotalCount = null;
        try {
            counterInstance = cacheManager
                    .getResource(CachePoolName.group_counter);
            strTotalCount = counterInstance.hget(getGroupRedisKey(groupId),
                    BizConstants.GROUP_COUNTER_SUBKEY_COUNTER);
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.group_counter,
                    counterInstance);
        }
        int totalCount = 0;
        if (null != strTotalCount && !"".equals(strTotalCount)) {
            try {
                totalCount = Integer.valueOf(strTotalCount);
            } catch (Exception e) {
                logger.error("convert unread total, userId = " + userId
                        + ", groupId = " + groupId, e);
                totalCount = 0;
            }
        }
        if (totalCount <= 0) {
            GroupCounterItem counterItem = new GroupCounterItem();
            counterItem.setUserId(userId);
            counterItem.setGroupId(groupId);
            counterItem.setGroupTotalCount(0);
            counterItem.setUserUnreadCount(0);
            counterItem.setLastMessageId(0);
            return counterItem;
        }

        Map<String, String> userGroupCounter = null;
        try {
            counterInstance = cacheManager
                    .getResource(CachePoolName.group_counter);
            userGroupCounter = counterInstance.hgetAll(getUserGroupRedisKey(
                    userId, groupId, clientType));
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.group_counter,
                    counterInstance);
        }
        int readCount = 0;
        int lastId = 0;
        if (null != userGroupCounter && !userGroupCounter.isEmpty()) {
            try {
                String strReadCount = userGroupCounter
                        .get(BizConstants.GROUP_COUNTER_SUBKEY_COUNTER);
                readCount = Integer.valueOf(strReadCount);
                String lastMsgId = userGroupCounter
                        .get(BizConstants.GROUP_COUNTER_SUBKEY_LASTID);
                lastId = Integer.valueOf(lastMsgId);
            } catch (Exception e) {
                logger.error("convert unread readed, userId = " + userId
                        + ", groupId = " + groupId, e);
                // 出错了就让他最近的100条消息，防止丢消息
                readCount = 0;
                lastId = 0;
            }
        }
        int unreadCount = totalCount - readCount;
        unreadCount = unreadCount > BizConstants.GROUP_UNREAD_MAX_COUNTER ? BizConstants.GROUP_UNREAD_MAX_COUNTER
                : unreadCount;
        GroupCounterItem counterItem = new GroupCounterItem();
        counterItem.setUserId(userId);
        counterItem.setGroupId(groupId);
        counterItem.setGroupTotalCount(totalCount);
        counterItem.setUserUnreadCount(unreadCount);
        counterItem.setLastMessageId(lastId);
        return counterItem; // TODO
    }

    /*
     * 
     * @Description: 获取用户在指定群读过的最后一条消息Id,给用户取未读消息用
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     * 
     * @throws Exception
     */
    public int getUserLastReadMsgIdInGroup(int userId, int groupId, int clientType)
            throws Exception {
        if (userId <= 0 || groupId <= 0) {
            return -1;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        int msgId = -1;
        String strId = null;
        Jedis counterInstance = null;
        try {
            counterInstance = cacheManager
                    .getResource(CachePoolName.group_counter);

            strId = counterInstance.hget(getUserGroupRedisKey(userId, groupId, clientType),
                    BizConstants.GROUP_COUNTER_SUBKEY_LASTID);
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.group_counter,
                    counterInstance);
        }
        if (!"".equals(strId)) {
            try {
                msgId = Integer.valueOf(strId);
            } catch (Exception e) {
                logger.error("message Id convert: " + strId, e);
                msgId = -1;
            }
        }

        return msgId;
    }

    /*
     * 
     * @Description: 清除用户对应的某个群组的计数
     * 
     * @return
     */
    public boolean clearUserGroupCounter(int userId, int groupId, int clientType) {
        if (userId <= 0 || groupId <= 0) {
            return false;
        }

        CacheManager cacheManager = CacheManager.getInstance();
        Jedis counterInstance = null;
        Map<String, String> countData = null;
        try {
            counterInstance = cacheManager
                    .getResource(CachePoolName.group_counter);
            countData = counterInstance.hgetAll(getGroupRedisKey(groupId));
            if (countData != null && !countData.isEmpty()) {
                counterInstance.hmset(getUserGroupRedisKey(userId, groupId, clientType),
                        countData);
            }
        } catch (Exception e) {
            logger.error("", e);
        } finally {
            cacheManager.returnResource(CachePoolName.group_counter,
                    counterInstance);
        }

        return true;
    }

    /*
     * 
     * @Description: 发了一条消息，同时新增发送和接收用户的消息计数器
     * 
     * @param fromUserId 发送的用户
     * 
     * @param toUserId 接收的用户
     */
    public boolean incrGroupMsgCount(int groupId, int lastMessageId) {
        if (groupId == 0 || lastMessageId == 0) {
            return false;
        }
        CacheManager cacheManager = CacheManager.getInstance();
        Jedis counterInstance = cacheManager
                .getResource(CachePoolName.group_counter);

        counterInstance.hincrBy(getGroupRedisKey(groupId),
                BizConstants.GROUP_COUNTER_SUBKEY_COUNTER, 1);
        counterInstance.hset(getGroupRedisKey(groupId),
                BizConstants.GROUP_COUNTER_SUBKEY_LASTID,
                String.valueOf(lastMessageId));

        cacheManager.returnResource(CachePoolName.group_counter,
                counterInstance);
        return true;
    }

    /*
     * 
     * @Description: 获取用户所在群组未读消息计数的redis key
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @param clientType 客户端类型@see ClientType
     * 
     * @return
     */
    private String getUserGroupRedisKey(int userId, int groupId, int clientType) {
    	StringBuilder sb = new StringBuilder();
    	sb.append(ClientType.prefixKeyOf(clientType));
    	sb.append("_");
    	sb.append(userId);
    	sb.append("_");
    	sb.append(groupId);
    	sb.append(BizConstants.GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX);
        return  sb.toString();
    }

    /*
     * 
     * @Description: 获取用户群组redis的key
     * 
     * @param userId
     * 
     * @param groupId
     * 
     * @return
     */
    private String getGroupRedisKey(int groupId) {
        return groupId + BizConstants.GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX;
    }
}
