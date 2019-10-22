---
title: docker jenkins ci/cd
date: 2019-10-22 18:07:38
tags: [docker, jenkins, ci/cd, 持续集成, 运维, 自动化]
---

Jenkins 是一个很老的 ci/cd 服务了，但是至今并未过时，几乎是各种公司的首选，足见其功能强大

最近用 docker 搭建了一个 Jenkins 服务，实现了代码提交后自动触发测试和部署，再也不用手动发布了，节省了大量的时间

## 制作 docker 镜像

```
FROM jenkins/jenkins:2.191
COPY executors.groovy /usr/share/jenkins/ref/init.groovy.d/executors.groovy
USER root
RUN apt-get update
RUN apt-get install -y build-essential
```

这里我在基础镜像上安装了 make 工具，因为我大部分项目都是用 make 构建

## 部署 jenkins 到 docker swarm

我将 jenkins 服务本身也部署到了 docker 集群中，compose 文件内容如下：

``` yml
version: "3.5"
services:
  jenkins:
    image: hatlonely/jenkins:${version}
    hostname: jenkins
    user: root
    ports:
      - "8080:8080"
      - "50000:50000"
    deploy:
      replicas: 1
      restart_policy:
        condition: on-failure
    volumes:
      - "/var/docker/jenkins/data:/var/jenkins_home"
      - "/var/docker:/var/docker"
      - "/var/run/docker.sock:/var/run/docker.sock"
      - "/bin/docker:/usr/bin/docker"
    networks:
      - jenkinsnet

networks:
  jenkinsnet:
    name: jenkinsnet
```

这里主要要解决两个问题：

1. jenkins 本身作为容器在 docker 集群环境下运行，如何访问外部 docker 环境

``` yml
volumes:
    - "/var/run/docker.sock:/var/run/docker.sock"
    - "/bin/docker:/usr/bin/docker"
```

docker 是 c/s 架构，会在本地后台启动一个服务来管理和维护容器，客户端通过套接字和服务通信，将套接字映射到容器内部，可以让容器也能访问宿主机的 docker，此外将 docker 命令本身映射到容器内部，就不需要在容器内部再安装一个 docker 了

2. 部署服务的数据挂载，如何挂载宿主机

``` yml
volumes:
    - "/var/docker/jenkins/data:/var/jenkins_home"
    - "/var/docker:/var/docker"
```

这里我有个实践的约定，会将所有 docker 容器的状态映射到宿主机的 `/var/docker/{service}` 下面，Jenkins 的状态保存在 `/var/jenkins_home` 下，这里我们把它直接映射到 `/var/docker/jenkins/data` 下，另外，其他的服务需要通过容器内的 Jenkins 部署，volume 挂载的数据只能映射到 Jenkins 容器内部，而我们约定，这些数据只能在 `/var/docker` 下，因此，只需要再挂载这个目录，其他服务的状态就都能持久化到宿主机了

完整代码参考：<https://github.com/hpifu/docker-jenkins>

执行 `make build` 完成镜像的制作（我把 Dockerfile 中的端口映射注释掉了，是因为我的环境里面所有的服务都统一用 nginx 作反向代理对外了）
执行 `make deploy` 即可完成 Jenkins 的部署

## 创建第一个流水线项目，部署 Jenkins 服务本身

部署完成之后，访问 <http://127.0.0.1:8080>，按照提示很容易就可以完成初始化，初始化完成之后，`/var/jenkins_home` 里面会产生很多数据，这些数据是 Jenkins 的状态，前面我们已把这个目录映射到了宿主机的 `/var/docker/jenkins/data`，这样重启 Jenkins 数据也不会丢失

初始化完成之后终于可以创建任务了，相比之下，这个过程可能是最简单的

【新建任务】→【流水线】(名称填 docker-jenkins)→【确定】

General

- ✔️ github 项目: git@github.com:hpifu/docker-jenkins.git
- ✔️ 不允许并发构建

构建触发器

- ✔️ GitHub hook trigger for GITScm polling

流水线

- ✔️ Pipeline script

```
pipeline {
    agent any
    stages {
        stage('image') {
            steps {
                sh 'make build'
            }
        }
        stage('deploy') {
            steps {
                sh 'make deploy'
            }
        }
    }
}
```

点保存之后点立即构建即可

## github webhook

要实现 github push 之后自动触发，需要在 github 设置 webhook（需要 owner 权限），另外需要 Jenkins 有对外网可见的地址

1. 进入项目主页，如本项目主页：<https://github.com/hpifu/docker-jenkins>
2. 点击【Settings】→【Webhooks】→【Add webhook】

```
Payload URL: <https://<your jenkins server address>/github-webhook/>
Content type：<application/json>
```

配置完成后，可以提交下试试，在 webhooks 页面可以看到刚刚配置的 hook，拖到最下面有最近的 hook 触发记录

Jenkins 第一次构建需要手动触发，之后的构建都会自动触发，偶尔没有触发，可能是 github 和你的服务之间的网络问题，可去 github webhook 页面检查 hook 触发记录

## 我的最佳实践

对项目作一些简单的规范，能大大地简化整个部署流程，前面创建项目的时候我们使用的是 `Pipeline script`，可以看到还有另一个选项 `Pipeline script from SCM` 支持直接从一个 git 地址获取这个脚本，一般名为 Jenkinsfile，这也是我推荐的一种方式，Jenkinsfile 和代码一起维护起来，这样不同的项目，不同的 Jenkins 环境，创建的 Jenkins 任务都是一样的，大大简化了创建任务的复杂度

另一方面，项目的构建，编译，运行，打包，部署，测试等都通过 Makefile 实现，这样所有执行的动作都变成了一条 make 命令，Jenkinsfile 的逻辑变得非常简单，甚至可以是通用的，下面是一些示例:

- go 项目: <https://github.com/hpifu/go-tech/blob/master/Jenkinsfile> 
- vue 项目: <https://github.com/hpifu/vue-tech/blob/master/Jenkinsfile>

通过这些示例可以看到，复杂的构建过程都被 Makefile 屏蔽了，而通过 Makefile，不同类型，不同语言的项目的构建过程也对 Jenkins 屏蔽了，达到了统一

## 链接

- 参考代码: <https://github.com/hpifu/docker-jenkins>
- docker hub: <https://hub.docker.com/_/jenkins>
