---
title: grafana-oauth-授权.md
date: 2019-07-12 03:50:35
tags: [grafana, oauth, github, gitlab]
---

## github 授权

### github 上创建 oauth 应用

在  <https://github.com/settings/applications/new> 页面创建应用

- `Application name`: `grafana` 
- `Homepage URL`: `https://grafana.hatlonely.com`
- `Application description`: 可以不填
- `Authorization callback URL`: `https://grafana.hatlonely.com/login/github`

点击注册之后会跳转的页面中获取 `Client ID` 和 `Client Secret`

### 配置 github oauth

修改配置文件 `/etc/grafana/grafana.ini` 的 auth.github

```
[auth.github]
enabled = true
allow_sign_up = true
client_id = <上一步中的 Client ID>
client_secret = <上一步中的 Client Secret>
scopes = user:email,read:org
auth_url = https://github.com/login/oauth/authorize
token_url = https://github.com/login/oauth/access_token
api_url = https://api.github.com/user
team_ids =
allowed_organizations =
```

修改配置文件 `/etc/grafana/grafana.ini` 的 `root_url`

```
root_url = https://grafana.hatlonely.com
```

## gitlab 授权

### gitlab 上创建 oauth 应用

登陆 gitlab，【头像】→【setting】→【Applications】

- `Name`: `grafana` 
- `Redirect URI`: `https://grafana.hatlonely.com/login/gitlab`
- `Scopes`: 选 api 就行

点击注册之后会跳转的页面中获取 `Application ID` 和 `Secret`

### 配置 gitlab oauth

修改配置文件 `/etc/grafana/grafana.ini` 的 auth.gitlab

```
[auth.gitlab]
enabled = true
allow_sign_up = false
client_id = <上一步中的 Application ID>
client_secret = <上一步中的 Secret>
scopes = api
auth_url = https://gitlab.hatlonely.com/oauth/authorize
token_url = https://gitlab.hatlonely.com/oauth/token
api_url = https://gitlab.hatlonely.com/api/v4
allowed_groups =
```

修改配置文件 `/etc/grafana/grafana.ini` 的 `root_url`

```
root_url = https://grafana.hatlonely.com
```

## 链接

- grafana 配置: <https://grafana.com/docs/installation/configuration/>
- grafana github 授权: <https://grafana.com/docs/auth/github/>
- grafana gitlab 授权: <https://grafana.com/docs/auth/gitlab/>

