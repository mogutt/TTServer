package com.mogujie.ares.util;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.mogujie.ares.data.User;
import com.mogujie.ares.lib.net.DataBuffer;

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
        return (type == 1) ? true : false;
    }

    /*
     * 对int数组去重
     * 
     * @param array 需要去重的数组
     */
    public static int[] distinct(int[] array) {
        Set<Integer> intSet = new HashSet<Integer>();
        int intVal;
        int length = array.length;
        for (int i = 0; i < length; i++) {
            intVal = array[i];
            if (!intSet.contains(intVal)) {
                intSet.add(intVal);
            }
        }
        Integer[] uniqArray = new Integer[intSet.size()];
        intSet.toArray(uniqArray);
        int[] ints = new int[uniqArray.length];
        for (int i = 0; i < uniqArray.length; i++) {
            ints[i] = uniqArray[i];
        }
        return ints;
    }

    /*
     * 对int数组去重并返回map对象
     * 
     * @param array 需要去重的数组
     */
    public static Map<Integer, Integer> distinctToMap(int[] array) {
        Set<Integer> intSet = new HashSet<Integer>();
        int intVal;
        int length = array.length;
        for (int i = 0; i < length; i++) {
            intVal = array[i];
            if (!intSet.contains(intVal)) {
                intSet.add(intVal);
            }
        }
        Integer[] uniqArray = new Integer[intSet.size()];
        intSet.toArray(uniqArray);
        Map<Integer, Integer> mapIds = new HashMap<Integer, Integer>();
        for (int i = 0; i < uniqArray.length; i++) {
            mapIds.put(uniqArray[i], uniqArray[i]);
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
        StringBuffer buffer = new StringBuffer("");
        if (number <= 0) {
            return buffer.toString();
        }

        for (int i = 0; i < number; i++) {
            buffer.append(split + hoder);
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
        StringBuffer sb = new StringBuffer();
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
