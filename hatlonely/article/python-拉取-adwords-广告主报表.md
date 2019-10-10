---
title: python 拉取 adwords 广告主报表
date: 2019-03-07 16:25:16
tags: [python, adwords, 广告, 报表]
---

从 adwords 拉取报表数据集成到自己的平台，方便数据的统一管理和查看，这个流程还是有点复杂的

### 对接流程

1. 登陆 adwords 平台，<ads.google.com/aw/overview>，选择经理账号
2. 【工具】→【API 中心】，填写开发者信息，申请开发者令牌，测试的令牌不能获取真实的数据
3. 【API 访问权限】→【访问权限级别】→【申请基本访问权限】为令牌申请基本访问权限(每天固定的访问次数)，填写单子，之后开发者信息的邮箱 (第2步填写的邮箱信息) 会收到 adwords 的邮件审核邮件
4. 为账号开通 API 权限，<console.developers.google.com>
5. 【凭据】，如果没有项目先创建项目，选择项目，创建 OAuth 凭证，应用类型选【其他】
6. 

### 参考链接

- adword api 对接官方文档: <https://developers.google.com/adwords/api/docs/guides/start>
