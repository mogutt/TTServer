package com.mogujie.ares.model;

/*
 * 
 * @Description: 服务配置相关操作
 * @author zuoye - zuoye[at]mogujie.com
 * @date 2013-11-5
 * 
 */
public class ServerConfigModel {

    private static ServerConfigModel configModelInstance;

    public static ServerConfigModel getInstance() {
        if (configModelInstance == null) {
            configModelInstance = new ServerConfigModel();
        }
        return configModelInstance;
    }

    private ServerConfigModel() {
    }

    public String getSensitivityWord() throws Exception {
        return "";
    }
}
