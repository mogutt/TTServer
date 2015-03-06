package com.mogujie.ares.util;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.mogujie.ares.data.User;
import com.mogujie.ares.lib.net.DataBuffer;
import java.util.Iterator;

/*
 * 蘑菇街IM工具类
 */
public final class MoguUtil {

    /*
     * 判断是否是管理员
     */
    public boolean isAdmin(User user) {
        return (user == null) ? false : isAdmin(user.getUserType());
    }

    /*
     * 判断是否是管理员
     */
    public boolean isAdmin(int type) {
        return (type == 1);
    }

    /*
     * 对int数组去重
     * 
     * @param array 需要去重的数组
     */
    public static int[] distinct(int[] array) {
        Set<Integer> intSet = new HashSet<Integer>();
        for(int i = 0 ; i < array.length; i++){
            intSet.add(array[i]);
        }
        int[] ints = new int[intSet.size()];
        Iterator<Integer> iter = intSet.iterator();
        int i = 0;
        while(iter.hasNext()){
            ints[i++] = iter.next();
        }
        return ints;
    }

    /*
     * 对int数组去重并返回map对象
     * 
     * @param array 需要去重的数组
     */
    public static Map<Integer, Integer> distinctToMap(int[] array) {
        int[] uniqArray = distinct(array);
        Map<Integer, Integer> mapIds = new HashMap<Integer, Integer>();
        for (Integer uniqArray1 : uniqArray) {
            mapIds.put(uniqArray1, uniqArray1);
        }
        return mapIds;
    }

    /*
     * 获得DB中参数？占位符
     * 
     * @parma number 参数个数
     */
    public static String getArgsHolder(int number) {
        return getArgsHolder("?", ",", number);
    }

    /*
     * 获得DB中参数？占位符
     * 
     * @param hoder 占位的字符
     * 
     * @param split 分隔的字符
     * 
     * @parma number 参数个数
     */
    public static String getArgsHolder(String hoder, String split, int number) {
        StringBuilder buffer = new StringBuilder();
        if (number <= 0) {
            return buffer.toString();
        }

        for (int i = 0; i < number; i++) {
            buffer.append(split).append(hoder);
        }
        return buffer.toString().substring(1);

    }

    /**
     * 
     * @Description: md5加密
     * @param str
     * @return
     * @throws NoSuchAlgorithmException
     */
    public static String md5(String str) throws NoSuchAlgorithmException {
        MessageDigest tool = MessageDigest.getInstance("MD5");
        tool.update(str.getBytes());
        byte[] bytes = tool.digest();
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < bytes.length; i++) { // 字节数组转换成十六进制字符串，形成最终的密文
            int v = bytes[i] & 0xff;
            if (v < 16) {
                sb.append(0);
            }
            sb.append(Integer.toHexString(v));
        }

        return sb.toString();
    }

    /*
     * 将请求附带的信息写回
     */
    public static DataBuffer writeAttachments(DataBuffer buffer,
            DataBuffer attachment) {
        if (null != buffer && null != attachment) {
            buffer.writeInt(attachment.readableBytes());
            if (attachment.readableBytes() > 0) {
                buffer.writeDataBuffer(attachment);
            }
        }

        return buffer;
    }
}
