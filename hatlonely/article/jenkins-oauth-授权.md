---
title: jenkins oauth 授权
date: 2019-07-11 17:33:00
tags: [jenkins, oauth, github, gitlab]
---

## github 授权

### 安装 github oauth 插件

【Manage Jenkins】→【Manage Plugins】→【Available】，过滤中搜索 oauth，安装插件 github oauth 插件

### github 上创建 oauth 应用

在  <https://github.com/settings/applications/new> 页面创建应用

- `Application name`: `jenkins` 
- `Homepage URL`: `https://jenkins.hatlonely.com`
- `Application description`: 可以不填
- `Authorization callback URL`: `https://jenkins.hatlonely.com/securityRealm/finishLogin`

点击注册之后会跳转的页面中获取 `Client ID` 和 `Client Secret`

### 配置 github oauth

【Manage Jenkins】→【Configure Global Security】→【安全域】，选 【Github Authentication Plugin】

- `GitHub Web URI`: `https://github.com`
- `GitHub API URI`:	`https://api.github.com`
- `Client ID`: 上一步中的 click id
- `Client Secret`: 上一步中的 client secret
- `OAuth Scope(s)`:	`read:org,user:email,repo`

## gitlab 授权

### 安装 gitlab 插件

【Manage Jenkins】→【Manage Plugins】→【Available】，过滤中搜索 oauth，安装插件 gitlab oauth 插件

### gitlab 上创建 oauth 应用

登陆 gitlab，【头像】→【setting】→【Applications】

- `Name`: `jenkins` 
- `Redirect URI`: `https://jenkins.hatlonely.com/securityRealm/finishLogin`

点击注册之后会跳转的页面中获取 `Application ID` 和 `Secret`

### 配置 gitlab oauth

【Manage Jenkins】→【Configure Global Security】→【安全域】，选 【Gitlab Authentication Plugin】

- `GitLab Web URI`: `https://gitlab.hatlonely.com`
- `GitLab API URI`: `https://gitlab.hatlonely.com`
- `Client ID`: 上一步中的 click id
- `Client Secret`: 上一步中的 client secret

## 链接

- jenkins github oauth plugin wiki: <https://wiki.jenkins.io/display/JENKINS/GitHub+OAuth+Plugin>
- jenkins gitlab oauth plugin wiki: <https://wiki.jenkins.io/display/JENKINS/GitLab+OAuth+Plugin>
