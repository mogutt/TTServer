package com.mogujie.ares.configure;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.dom4j.DocumentException;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;

/**
 * 
 * @ClassName: Router
 * @Description: Action的路由配置信息类
 * @author ziye - ziye(at)mogujie.com
 * @date 2013-7-20 下午5:36:56
 */
public class Router {

	public static Router instance;
	
	// 路由的配置信息
	private Map<Integer, ActionDescricptor> actionMap = new HashMap<Integer, ActionDescricptor>();
	
	public static Router getInstance() {
		if(instance == null) {
			instance = new Router();
		}
		
		return instance;
	}
	
	private Router() {
		
	}
	
	public Map<Integer, ActionDescricptor> getActionMap() {
		return actionMap;
	}

	public void setActionMap(Map<Integer, ActionDescricptor> actionMap) {
		this.actionMap = actionMap;
	}

	/**
	 * 
	 * @Title: getActionByRequestType
	 * @Description: 根据头的type类型获得具体的action
	 * @param @param type
	 * @param @return    
	 * @return ActionItem    
	 * @throws
	 */
	public ActionDescricptor getActionByRequestType(int type) {
		return actionMap.get(type);
	}
	
	/**
	 * 
	 * @Title: load
	 * @Description: 装在配置,配置文件格式如下:
	 * <route>
	 *	<requests>
	 *		<request>
	 *			<action-class>com.mogujie.ares.extend.action.Monitor</action-class>
	 *			<method>heartbeat</method>
	 *			<request-type>1</request-type>
	 *			<response-type>1</response-type>
	 *			<params>
	 *				<String>${channel.remoteAddress}</String>
	 *			</params>
	 *			<filters>
	 *				<!-- <filter>com.mogujie.ares.filter.LoginFilter</filter> -->
	 *			</filters>
	 *		</request>
	 *	 </requests>
	 *	</route>
	 * @param @param file
	 * @param @throws Exception    
	 * @return void    
	 * @throws
	 */
	@SuppressWarnings({"unchecked"})
	public void load(String file) throws Exception {
		if(file == null || file.length() == 0) {
			throw new Exception("Route file is not specified！");
		}
		
		InputStream is = Router.class.getClassLoader().getResourceAsStream(file);
		if(is == null) {
			throw new Exception("Route file is not found! " + file);
		}
		SAXReader reader = new SAXReader();
		try {
			org.dom4j.Document configDocument = reader.read(is);
			List<Node> nodes = configDocument.selectNodes("//route/requests/request");
			Node node;
			ActionDescricptor item;
			for(int i = 0; i < nodes.size(); i++)
			{
				item = new ActionDescricptor();
				
				node = nodes.get(i);
				Node actionClassNode = node.selectSingleNode("action-class");
				item.setActionClass(actionClassNode.getStringValue());
				Node methodNode = node.selectSingleNode("method");
				item.setMethod(methodNode.getStringValue());
				Node requestTypeNode = node.selectSingleNode("request-type");
				item.setRequestType(Integer.parseInt(requestTypeNode.getStringValue()));
				Node responseTypeNode = node.selectSingleNode("response-type");
				item.setResponseType(Integer.parseInt(responseTypeNode.getStringValue()));

				// params
				Node paramsNode = node.selectSingleNode("params");
				List<Node> paramNodes = paramsNode.selectNodes("*"); // 取得所有的子节点
				Map<String, String> params = new LinkedHashMap<String, String>();
				Node param;
				for(int j = 0; j < paramNodes.size(); j++) {
					param = paramNodes.get(j);
					params.put(param.getStringValue(), param.getName());
				}
				item.setParams(params);
				
				// filters
				Node filter;
				Node filtersNode = node.selectSingleNode("filters");
				List<Node> filterNodes = filtersNode.selectNodes("*");
				List<String> filters = new ArrayList<String>();
				for(int j = 0; j < filterNodes.size(); j++) {
					filter = filterNodes.get(j);
					filters.add(filter.getStringValue());
				}
				String[] str = new String[filters.size()];
				filters.toArray(str);
				item.setFilters(str);
				
				actionMap.put(item.getRequestType(), item);
			}
		} catch (DocumentException e) {
			throw e;
		}
	}
	
	/**
	 * 
	 * @ClassName: ActionItem
	 * @Description: 单个Action的描述类
	 * @author ziye - ziye(at)mogujie.com
	 * @date 2013-7-20 下午5:42:09
	 *
	 */
	public class ActionDescricptor{
		
		// 类名
		private String actionClass;
		
		// 处理请求的方法名
		private String method;
		
		// 请求头的type，这种type的请求才处理
		private int requestType;

		// 响应的type
		private int responseType;
		
		// 请求的参数
		private Map<String, String> params;
		
		// filter过滤
		private String[] filters;
		
		public ActionDescricptor() {
		}

		public String getActionClass() {
			return actionClass;
		}

		public void setActionClass(String actionClass) {
			this.actionClass = actionClass;
		}

		public String getMethod() {
			return method;
		}

		public void setMethod(String method) {
			this.method = method;
		}

		public int getRequestType() {
			return requestType;
		}

		public void setRequestType(int requestType) {
			this.requestType = requestType;
		}
		
		public Map<String, String> getParams() {
			return params;
		}

		public void setParams(Map<String, String> params) {
			this.params = params;
		}

		public String[] getFilters() {
			return filters;
		}

		public void setFilters(String[] filters) {
			this.filters = filters;
		}

		public int getResponseType() {
			return responseType;
		}

		public void setResponseType(int responseType) {
			this.responseType = responseType;
		}
	}
}
