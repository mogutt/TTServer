package com.mogujie.ares.data;

/**
 * 
 * @ClassName: User
 * @Description: 用户对象描述类
 * @author ziye - ziye(at)mogujie.com
 * @date 2013-7-20 下午5:49:23
 * 
 */
public class User {

    protected int userId; // 用户id

    protected String uname; // 用户名

    protected String unick; // 用户昵称

    protected String avatar; // 用户头像

    protected String title; // 职务

    protected String position; // 地址，为什么不是address？

    protected int status; // 用户在职状态 0:正常(在职) 1:删除(离职) 可扩展

    protected int sex; // 性别

    protected int userType = 0; // 即identity，用户身份标识, 0-普通用户 1-管理员

    private int departId; // 部门id

    private int jobNumber; // 工号

    private String telphone; // 电话

    private String mail; // 邮箱

    private int created; // 创建时间

    private int updated; // 更新时间

    public int getUserId() {
        return userId;
    }

    public void setUserId(int userId) {
        this.userId = userId;
    }

    public String getUname() {
        return uname;
    }

    public void setUname(String uname) {
        this.uname = uname;
    }

    public String getAvatar() {
        return avatar;
    }

    public void setAvatar(String avatar) {
        this.avatar = avatar;
    }

    public int getDepartId() {
        return departId;
    }

    public void setDepartId(int departId) {
        this.departId = departId;
    }

    public String getUnick() {
        return unick;
    }

    public void setUnick(String unick) {
        this.unick = unick;
    }

    public String getTelphone() {
        return telphone;
    }

    public void setTelphone(String telphone) {
        this.telphone = telphone;
    }

    public int getCreated() {
        return created;
    }

    public void setCreated(int created) {
        this.created = created;
    }

    public int getUpdated() {
        return updated;
    }

    public void setUpdated(int updated) {
        this.updated = updated;
    }

    public int getSex() {
        return sex;
    }

    public void setSex(int sex) {
        this.sex = sex;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getPosition() {
        return position;
    }

    public void setPosition(String position) {
        this.position = position;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public int getUserType() {
        return userType;
    }

    public void setUserType(int user_type) {
        this.userType = user_type;
    }

    public int getJobNumber() {
        return jobNumber;
    }

    public void setJobNumber(int jobNumber) {
        this.jobNumber = jobNumber;
    }

    /**
     * @return the mail
     */
    public String getMail() {
        return mail;
    }

    /**
     * @param mail
     *            the mail to set
     */
    public void setMail(String mail) {
        this.mail = mail;
    }
}
