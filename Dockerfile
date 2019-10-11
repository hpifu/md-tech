FROM centos:centos7
RUN yum install -y kde-l10n-Chinese && yum reinstall -y glibc-common && localedef -c -f UTF-8 -i zh_CN zh_CN.utf8
RUN yum install -y epel-release
RUN yum install -y python36 python36-setuptools python36-pip
RUN pip3 install --upgrade pip
RUN pip3 install requests
RUN pip3 install flask
RUN pip3 install redis
RUN pip3 install pymongo
RUN pip3 install behave
RUN pip3 install pyhamcrest
RUN pip3 install pymysql
RUN pip3 install cryptography
ENV LANG=zh_CN.UTF-8
ENV LANGUAGE=zh_CN:zh
ENV LC_ALL=zh_CN.UTF-8

COPY . /var/docker/md-tech
WORKDIR /var/docker/md-tech/scripts
CMD [ "sh", "main.sh" ]
