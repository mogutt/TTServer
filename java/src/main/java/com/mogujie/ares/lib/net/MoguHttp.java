package com.mogujie.ares.lib.net;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.methods.ByteArrayRequestEntity;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.methods.RequestEntity;
import org.apache.commons.httpclient.methods.StringRequestEntity;
import org.apache.commons.httpclient.methods.multipart.ByteArrayPartSource;
import org.apache.commons.httpclient.methods.multipart.FilePart;
import org.apache.commons.httpclient.methods.multipart.MultipartRequestEntity;
import org.apache.commons.httpclient.methods.multipart.Part;
import org.apache.commons.httpclient.methods.multipart.PartSource;
import org.apache.commons.httpclient.params.HttpConnectionManagerParams;
import org.apache.http.NameValuePair;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.utils.URLEncodedUtils;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.util.EntityUtils;
import org.codehaus.jackson.map.ObjectMapper;

import com.alibaba.druid.util.StringUtils;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;

public class MoguHttp {
	
	private static Logger logger = LoggerFactory.getLogger(MoguHttp.class);

	public static MoguHttpResponse get(String url) {
		MoguHttpResponse response = new MoguHttpResponse();
		response.setStatusCode(404);
		if(StringUtils.isEmpty(url)) {
			return response;
		}
		int statusCode = 404;
		CloseableHttpClient httpClient = null; 
		HttpGet httpGet = null; 
		try {
			httpClient = HttpClients.createDefault();
			httpGet = new HttpGet(url);//HTTP Get请求(POST雷同)
			RequestConfig requestConfig = RequestConfig
					.custom()
					.setSocketTimeout(2000)
					.setConnectTimeout(2000)
					.build();//设置请求和传输超时时间
			httpGet.setConfig(requestConfig);
			CloseableHttpResponse hresp = httpClient.execute(httpGet);//执行请求
			String responseString = EntityUtils.toString(hresp.getEntity());
			response.setStatusCode(hresp.getStatusLine().getStatusCode());
			response.setResponseBody(responseString);
			return response;
		} catch (Exception e) {
			logger.error("error code: " + statusCode, e);
		} finally {
			if(httpGet != null) {
				httpGet.releaseConnection();
			}
		}
		return response;
	}
	
	@SuppressWarnings("rawtypes")
	public static MoguHttpResponse post(String url) {
		return post(url, new HashMap());
	}
	
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static MoguHttpResponse post(String url, Map params) {
		MoguHttpResponse response = new MoguHttpResponse();
		if(StringUtils.isEmpty(url)) {
			response.setStatusCode(404);
			return response;
		}

		CloseableHttpClient httpClient = null; 
		HttpPost httpPost = null; 
		try {
			httpPost = new HttpPost(url);//HTTP Get请求(POST雷同)
			List<NameValuePair> postData = new ArrayList<NameValuePair>();
			Iterator<Entry> piter = params.entrySet().iterator();
			while(piter.hasNext()) {
				Entry entry = piter.next();
				postData.add(new BasicNameValuePair(String.valueOf(entry.getKey()), String.valueOf(entry.getValue())));
			}
			StringEntity entity = new StringEntity(URLEncodedUtils.format(postData, "UTF-8"));
			httpPost.setEntity(entity);
			RequestConfig requestConfig = RequestConfig
					.custom()
					.setSocketTimeout(2000)
					.setConnectTimeout(2000)
					.build();//设置请求和传输超时时间
			httpPost.setConfig(requestConfig);
			httpClient = HttpClients.createDefault();
			CloseableHttpResponse hresp = httpClient.execute(httpPost);//执行请求
			String responseString = EntityUtils.toString(hresp.getEntity());
			response.setStatusCode(hresp.getStatusLine().getStatusCode());
			response.setResponseBody(responseString);
			return response;
		} catch(Exception e) {
			logger.error("url: " + url, e);
		} finally {
			if(httpPost != null) {
				httpPost.releaseConnection();
			}
		}
		return response;
	}
	
	/**
	 * 
	 * @Description: 上传文件，返回上传完成后文件的key，上传失败返回""空字符串
	 * @param url
	 * @param fileContent
	 * @param paramMap
	 * @return
	 */
	public static String uploadByteFile(String url, byte[] fileContent) {
		
		if(StringUtils.isEmpty(url) || fileContent == null || fileContent.length == 0) {
			return "";
		}
		
		PostMethod postMethod = new PostMethod(url);
		RequestEntity entity = new ByteArrayRequestEntity(fileContent);
		postMethod.setRequestEntity(entity);
	    
		HttpClient client = new HttpClient();
		client.getHttpConnectionManager().getParams().setConnectionTimeout(500);
		String key = "";
		try {
			client.executeMethod(postMethod);
			byte[] respByte = postMethod.getResponseBody();
			String responseJsonData = new String(respByte, "UTF-8");
			logger.info(responseJsonData);
			ObjectMapper objectMapper = new ObjectMapper();
			@SuppressWarnings("unchecked")
			Map<String, String> maps = objectMapper.readValue(responseJsonData, Map.class);
			String status = maps.get("status");
			String token = maps.get("message");
			if("0".equals(status) && !StringUtils.isEmpty(token)) {
				key = token;
			}
			return key;
		} catch (Exception e) {
			logger.error("", e);
		} finally {
			postMethod.releaseConnection();
		}

		return key;
	}
	
	/**
	 * 
	 * @Description: 上传文件，返回上传完成后文件的key，上传失败返回""空字符串
	 * @param url
	 * @param fileContent
	 * @param paramMap
	 * @return
	 */
	public static String uploadAudioByteFile(String url, byte[] fileContent) {
		
		if(StringUtils.isEmpty(url) || fileContent == null || fileContent.length == 0) {
			return "";
		}
		
		PostMethod postMethod = new PostMethod(url);
		Part[] parts = new Part[1];
		String fileName = System.currentTimeMillis() + ".audio";
		PartSource ps = new ByteArrayPartSource(fileName, fileContent);
		parts[0] = new FilePart("", ps);
		postMethod.addParameter("Content-Disposition", "form-data;name=\"file\";filename=\"\"");
		postMethod.addParameter("Content-Type", "image/jpeg");
		RequestEntity entity = new MultipartRequestEntity(parts, postMethod.getParams());
		postMethod.setRequestEntity(entity);
		HttpClient client = new HttpClient();
		client.getHttpConnectionManager().getParams().setConnectionTimeout(500);
		String key = "";
		try {
			client.executeMethod(postMethod);
			byte[] respByte = postMethod.getResponseBody();
			String responseJsonData = new String(respByte, "UTF-8");
			logger.info(responseJsonData);
			ObjectMapper objectMapper = new ObjectMapper();
			@SuppressWarnings("unchecked")
			Map<String, Object> maps = objectMapper.readValue(responseJsonData, Map.class);
			Object oStatus = maps.get("error_code");
			Integer iStatus = (Integer)oStatus;
			int status = iStatus.intValue();
			String path = (String)maps.get("path");
			if( 0 == status && !StringUtils.isEmpty(path)) {
				key = path;
			}
			return key;
		} catch (Exception e) {
			logger.error("", e);
		} finally {
			postMethod.releaseConnection();
		}

		return key;
	}
	
	/**
	 * 
	 * @Description: post 没有参数名字
	 * @param url
	 * @param fileContent
	 * @param paramMap
	 * @return
	 */
	public static MoguHttpResponse post(String url, String strParam) {

		MoguHttpResponse response = new MoguHttpResponse();
		response.setStatusCode(4007);
		response.setResponseBody("");
		if(StringUtils.isEmpty(url) || StringUtils.isEmpty(strParam)) {
			return response;
		}
		
		PostMethod postMethod = new PostMethod(url);
		RequestEntity entity;
		try {
			entity = new StringRequestEntity(strParam, "text/plain", "UTF-8");
		} catch (UnsupportedEncodingException e) {
			logger.error("", e);
			return response;
		}
		postMethod.setRequestEntity(entity);

		HttpClient client = new HttpClient();
		client.getHttpConnectionManager().getParams().setConnectionTimeout(500);
		try {
			client.executeMethod(postMethod);
			byte[] respByte = postMethod.getResponseBody();
			String responseJsonData = new String(respByte, "UTF-8");
			ObjectMapper objectMapper = new ObjectMapper();
			@SuppressWarnings("unchecked")
			Map<String, Object> maps = objectMapper.readValue(responseJsonData, Map.class);
			int status = Integer.valueOf(maps.get("code").toString());
			String msg = maps.get("msg").toString();
			response.setStatusCode(status);
			response.setResponseBody(msg);
			return response;
		} catch (Exception e) {
			logger.error("", e);
		} finally {
			postMethod.releaseConnection();
		}

		return response;
	}
	
	
	/**
	 * 
	 * @Description: 下载文件，返回文件的二进制流
	 * @param url
	 * @param fileContent
	 * @param paramMap
	 * @return
	 */
	public static byte[] downloadByteFile(String url) {

		if (StringUtils.isEmpty(url)) {
			return null;
		}
		HttpClient client = new HttpClient();
		HttpConnectionManagerParams managerParams = client
				.getHttpConnectionManager().getParams();
		managerParams.setConnectionTimeout(500); // 0.5s超时
		managerParams.setSoTimeout(500); // 0.5s超时
		GetMethod httpGet = new GetMethod(url);
		byte[] bytes = null;
		try {
			client.executeMethod(httpGet);
			bytes = httpGet.getResponseBody();
		} catch (Exception e) {
			logger.error(url, e);
		} finally {
			httpGet.releaseConnection();
		}
		return bytes;
	}
	
}
