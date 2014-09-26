package com.mogujie.ares.util;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

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

import com.alibaba.druid.util.StringUtils;
import com.mogujie.ares.lib.logger.Logger;
import com.mogujie.ares.lib.logger.LoggerFactory;
import com.mogujie.ares.lib.net.IMHttpResponse;

public class HttpUtil {
	
	private static Logger logger = LoggerFactory.getLogger(HttpUtil.class);

	public static IMHttpResponse get(String url) {
		IMHttpResponse response = new IMHttpResponse();
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
	public static IMHttpResponse post(String url) {
		return post(url, new HashMap());
	}
	
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static IMHttpResponse post(String url, Map params) {
		IMHttpResponse response = new IMHttpResponse();
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
	
	
}
