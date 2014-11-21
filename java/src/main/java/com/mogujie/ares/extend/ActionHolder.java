package com.mogujie.ares.extend;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import com.mogujie.ares.configure.Router;
import com.mogujie.ares.configure.Router.ActionDescricptor;
import com.mogujie.ares.extend.filter.IFilter;

/**
 * 
 * @Description: 
 * 				action的集合，在系统初始化的时候会把所有的action都初始化一遍，
 * 				缓存在这里，每个Action用一个ActionItem描述
 * 				ps: 这个类只在系统初始化的时候有修改，而且是单线程的，
 * 				后面多线程的都是读操作，不用做并发控制
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-7-22 下午4:22:55
 *
 */
public class ActionHolder {
	
	/**
	 * key: 请求头上的type
	 * value: 具体action的描述ActionItem
	 */
	private Map<Integer, ActionContext> actionMap = new HashMap<Integer, ActionContext>();
	
	private Map<String, BaseAction> actionObjectMap = new HashMap<String, BaseAction>();
	
	public ActionHolder() {
	}
	
	/**
	 * 
	 * @Description: 根据请求的type获得相应的ActionItem
	 * @param type
	 * @return
	 */
	public ActionContext get(int type) {
		return actionMap.get(type);
	}
	
	/**
	 * 
	 * @Description: 把一个Router里的所有的Action加入到Action中
	 * @param router
	 * @throws Exception 
	 */
	public void put(Router router) throws Exception {
		if(router == null) { return ; }
		Map<Integer, ActionDescricptor> actionDescMap = router.getActionMap();
		if(actionDescMap == null || actionDescMap.isEmpty()) {	return ; }
		
		Iterator<Entry<Integer, ActionDescricptor>> it = actionDescMap.entrySet().iterator();
		ActionDescricptor actionDesc = null;
		while(it.hasNext()) {
			Entry<Integer, ActionDescricptor> entry = it.next();
			actionDesc = entry.getValue();
			put(actionDesc);
		}
	}
	
	/**
	 * 
	 * @Description: 新增一个Action
	 * @param actionDesc
	 * @throws Exception
	 */
	@SuppressWarnings({"rawtypes"})
	public void put(ActionDescricptor actionDesc) throws Exception {
		
		if(actionDesc == null) {
			return ;
		}
		
		try {
			ActionContext item = new ActionContext();
			
			// action类
			String className = actionDesc.getActionClass();
			BaseAction actionObject = actionObjectMap.get(className);
			if(actionObject == null) {
				Class actionClazz = Class.forName(className);
				actionObject = (BaseAction)actionClazz.newInstance();
				actionObjectMap.put(className, actionObject);
			}
			item.setAction(actionObject);
	
			// 请求参数
			RequestParams requestParams = new RequestParams();
			Map<String, String> paramsdefine = actionDesc.getParams();
			Class[] paramTypes = requestParams.addParams(paramsdefine);
			item.setRequestParams(requestParams);
			
			// 处理业务逻辑的方法
			Method method = actionObject.getClass().getMethod(actionDesc.getMethod(), paramTypes);
			item.setDoMethod(method);
			
			// 请求类型
			item.setRequestType(actionDesc.getRequestType());
			item.setDefaultResponseType(actionDesc.getResponseType());
			
			// 过滤器，暂时还没有
			String[] strFilters = actionDesc.getFilters();
			String fname;
			IFilter filter;
			Class filterClazz;
			IFilter[] filters = new IFilter[strFilters.length];
			for(int i = 0; i < strFilters.length; i++) {
				fname = strFilters[i];
					filterClazz = Class.forName(fname);
				filter = (IFilter)filterClazz.newInstance();
				filters[i] = filter;
			}
			item.setFilters(filters);
			
			actionMap.put(item.getRequestType(), item);
		} catch (Exception e) {
			throw e;
		} 
	}

}
