---
title: hexo + github 博客
date: 2016-12-21 00:05:50
tags: [hexo, github, 博客, next]
---

## 创建 github 仓库

在 github 上创建一个名为 `hatlonely.github.com` 的仓库

## 安装下载 hexo

```
sudo npm install -g hexo
```

## 初始化 hexo

```
hexo init
```

修改 `_config.yml`，增加作者信息

```
title: hatlonely的博客
subtitle:
description: 做一个有意思的程序员
author: hatlonely
language: zh-Hans
```

## 预览 hexo

```
hexo server
```

现在可以 <http://127.0.0.1:4000/> 预览效果

## 提交到 github

修改配置文件`_config.yml`，部署到github，需要将本地 ssh key 添加到 github 账号中

```
deploy:
  type: git
  repository: git@github.com:hatlonely/hatlonely.github.com.git
  branch: master
```

下载 git 插件

```
npm install hexo-deployer-git --save
```

提交

```
hexo clean
hexo generate
hexo deploy
```

现在可以访问 <http://hatlonely.github.io/>

## 美化

下载 next 主题

```
git clone https://github.com/iissnan/hexo-theme-next themes/next
```

修改 `themes/next/_config.yml`，使用 Pisces 模式

```
scheme: Pisces
```

修改头像，将图片上传到 `public/images` 里面

```
avatar: /images/avatar.png
```

修改 `_config.yml`，使用 next 主题

```
theme: next
```

## 图片支持

修改配置 `_config.yml` 中有

```
post_asset_folder:true
```

安装 `hexo-assert-image`

```
npm install https://github.com/CodeFalling/hexo-asset-image --save
```

在 `sources/_posts/` 路径下面创建与文章名一致的目录，存放图片，图片的访问方式

```
![image](article/image.jpg)
```

## 添加标签页和分类页

```
hexo new page tags
hexo new page categories
```

修改主题配置文件 `themes/next/_config.yml`

```
menu:
  home: /
  categories: /categories
  archives: /archives
  tags: /tags
```

## 多说评论支持

1. 登陆多说 <http://duoshuo.com/>，点击个人主页获取user_id
2. 创建多说站点 <http://duoshuo.com/create-site/>
3. 修改next主题配置文件 `themes/next/_config.yml`

    ```
    duoshuo_shortname: <2中的多说域名>
    ```

4. 修改next主题配置文件 `themes/next/_config.yml`

    ```
    duoshuo_info:
    ua_enable: true
    admin_enable: true
    user_id: <1中获取的user_id>
    admin_nickname: hatlonely
    ```


## 腾讯统计支持

1. 登陆腾讯分析 <http://v2.ta.qq.com/analysis/index>，添加统计页面，获取统计id
2. 修改next主题配置文件 `themes/next/_config.yml`

    ```
    tencent_analytics: 56xxxx55
    ```

可能不是实时的，会有延时，延时几个小时吧，查看一下自己页面的源码，搜索一下统计id，如果有应该就没问题


## 参考

- [hexo 官网](https://hexo.io/)
- [next 主题使用文档](http://theme-next.iissnan.com/getting-started.html)
- [matrial 主题](https://github.com/viosey/hexo-theme-material)
- [icarus 主题](https://github.com/ppoffice/hexo-theme-icarus)
