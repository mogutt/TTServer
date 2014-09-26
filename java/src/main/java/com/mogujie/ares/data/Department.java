package com.mogujie.ares.data;

public class Department {

    protected int departId; // 部门id

    // protected String departName; // 部门名

    protected String title; // 部门标题

    protected String description; // 部门描述

    protected int parentDepartId; // 父部门id(上级部门id)

    protected int leader; // 部门leader

    protected int status; // 部门状态 0：正常 1：删除

    private int created; // 创建时间

    private int updated; // 更新时间

    public int getDepartId() {
        return departId;
    }

    public void setDepartId(int departId) {
        this.departId = departId;
    }

    // public String getDepartName() {
    // return departName;
    // }
    //
    // public void setDepartName(String departName) {
    // this.departName = departName;
    // }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public int getParentDepartId() {
        return parentDepartId;
    }

    public void setParentDepartId(int parentDepartId) {
        this.parentDepartId = parentDepartId;
    }

    public int getLeader() {
        return leader;
    }

    public void setLeader(int leader) {
        this.leader = leader;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
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

    public Department() {
        // TODO Auto-generated constructor stub
    }

}
