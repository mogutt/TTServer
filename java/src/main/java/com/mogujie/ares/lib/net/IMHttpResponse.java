package com.mogujie.ares.lib.net;

/**
 * 
 * @Description: http返回值
 * @author ziye - ziye[at]mogujie.com
 * @date 2014-3-9 下午10:21:08
 * 
 */
public class IMHttpResponse {
	private int statusCode;
	private String responseBody;

	public int getStatusCode() {
		return statusCode;
	}

	public void setStatusCode(int statusCode) {
		this.statusCode = statusCode;
	}

	public String getResponseBody() {
		return responseBody;
	}

	public void setResponseBody(String responseBody) {
		this.responseBody = responseBody;
	}

	@Override
	public String toString() {
		return "IMHttpResponse [statusCode=" + statusCode + ", responseBody="
				+ responseBody + "]";
	}
	
}