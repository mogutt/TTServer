package com.mogujie.ares.extend;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;

import org.jboss.netty.channel.ChannelHandlerContext;

import com.mogujie.ares.lib.net.DataBuffer;

/**
 * 
 * @Description: 请求参数的描述和控制类，负责将请求数据包中的二进制数据解析成具体的变量
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-21 上午11:41:37
 *
 */
@SuppressWarnings({"rawtypes"})
public class RequestParams {
	
	// 原生的描述参数的数据，参数名 => 参数类型，详细请见配置route.xml
	private Map<String, String> metaData = new LinkedHashMap<String, String>();
	
	// 参数名和java类型的对应
	private Map<String, Class> paramsType = new LinkedHashMap<String, Class>();
	
	/**
	 * 
	 * @Description: 添加单个参数
	 * @param name 参数名称
	 * @param typeName 参数类型名称
	 * @param typeClazz 参数的java类型
	 */
	public void add(String name, String typeName, Class typeClazz) {
		metaData.put(name, typeName);
		paramsType.put(name, typeClazz);
	}
	
	public Map<String, Class> getParamsType() {
		return paramsType;
	}

	public void setParamsType(Map<String, Class> paramsType) {
		this.paramsType = paramsType;
	}

	public Map<String, String> getMetaData() {
		return metaData;
	}

	public void setMetaData(Map<String, String> metaData) {
		this.metaData = metaData;
	}

	/**
	 * @Description: 将请求数据包中的二进制数据解析成具体的变量
	 * @param dataBuffer
	 * @param actionContext
	 * @return
	 * @throws Exception
	 * NOTE: 最后一个参数 version不需要客户端手动传的.处理参数的时候,自动从请求的报头里取的.
	 */
	public Object[] decode(DataBuffer dataBuffer, ChannelHandlerContext context, ActionContext actionContext,int version) throws Exception {
		Object[] values = null;
		
		if(metaData == null || metaData.size() <= 0) {
			values = new Object[0];
			return values;
		}
		
		values = new Object[metaData.size()];
		Iterator<String> it = metaData.keySet().iterator();
		String type;
		String name;
		int i = 0;
		while(it.hasNext() && i!=metaData.size()-1) {
			name = it.next();
			type = metaData.get(name);
			Object value = null;
			if(name.startsWith("${") && name.endsWith("}")) { // 系统数据
				// 这里特判
				if(name.equals("${channel.remoteAddress}")) { // 客户端的地址信息
					value = context.getChannel().getRemoteAddress().toString();
				} else if(name.equals("${action.requestType}")) {
					value = actionContext.getRequestType();
				}
				// TO BE CONTINUED....
			} else {
				if("int".equals(type)) {
					value = dataBuffer.readInt();
				} else if("char".equals(type)) {
					value = dataBuffer.readChar();
				} else if("byte".equals(type)) {
					value = dataBuffer.readByte();
				} else if("String".equals(type)) {
					value = dataBuffer.readString();
				} else if("int-array".equals(type)) { // int数组
					value = dataBuffer.readIntArray();
				} else if("byte-array".equals(type)) { // byte数组
					value = dataBuffer.readByteArray();
				} else if("string-array".equals(type)) { // String数组
					value = dataBuffer.readStringArray();
				} else if("DataBuffer".equals(type)) { // 直接传DataBuffer,业务方自己解析，太霸道了
					value = dataBuffer.readDataBuffer();
				}else {
					throw new Exception("不支持的类型");
				}
			}
			values[i++] = value;
		}
		values[i++] = version;
		return values;
	}
	
	/**
	 * @throws Exception 当传入的参数类型不支持时返回这个错误，比如Map什么的神类型
	 * @Description: 添加一个方法的参数，要按照从左到右的顺序添加，不然会有问题
	 * @param @param paramsMap 参数配置的map，参数名 => 类型名称
	 * @return Class[] 直接返回参数类型对应的java类型数组，顺序与传入时(配置)一致
	 * @throws
	 */
	public Class[] addParams(Map<String, String> paramsMap) throws Exception {
		Class[] paramTypes = new Class[paramsMap.size()];
		Iterator<String> it = paramsMap.keySet().iterator();
		int j = 0;
		Class typeClazz = null;
		while(it.hasNext()) {
			String name = it.next();
			String typeName = paramsMap.get(name);
			if("int".equals(typeName)) { 
				typeClazz = int.class;
			} else if("String".equals(typeName)) {
				typeClazz = String.class;
			} else if("byte".equals(typeName)) { // byte，和int分开是因为数据包中int占4个字节，byte只占一个字节
				typeClazz = byte.class;
			} else if("int-array".equals(typeName)) { // int数组
				typeClazz = int[].class;
			} else if("byte-array".equals(typeName)) { // byte数组
				typeClazz = byte[].class;
			} else if("string-array".equals(typeName)) { // string数组
				typeClazz = String[].class;
			} else if("DataBuffer".equals(typeName)) { // 鬼畜的dataBuffer
				typeClazz = DataBuffer.class;
			} else {
				throw new Exception(typeName + "是神马!");
			}
			paramTypes[j++] = typeClazz;
			add(name, typeName, typeClazz);
		}

		return paramTypes;
	}
}
