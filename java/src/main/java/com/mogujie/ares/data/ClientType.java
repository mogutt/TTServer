/**
 * 
 */
package com.mogujie.ares.data;

/**
 * @author seishuchen
 * @email shallowgrave@126.com
 */
public enum ClientType {
    WINDOWS(1), MAC(2), IOS(17), ANDROID(18);
    private int value;

    ClientType(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }
    
    public static ClientType valueOf(int value) {
        switch (value) {
        case 1:
        	return WINDOWS;
        case 2:
            return MAC;
        case 17:
            return IOS;
        case 18:
            return ANDROID;
        default:
            return MAC;
        }
    }
    
    /**
     * 获得客户端类型缓存Key前缀
     * @param value
     * @return
     */
    public static String prefixKeyOf(int value) {
    	switch (value) {
    	case 17:
    		return "IOS";
    	case 18:
    		return "ANDROID";
    	default:
    		return "";
    	}
    }
    
    public static String prefixKeyOf(ClientType clientType) {
    	return prefixKeyOf(clientType.value);
    }
}
