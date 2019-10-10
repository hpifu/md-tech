---
title: admob 广告开发者报表 api
date: 2018-11-01 19:37:54
tags: [admob, 广告报表, api]
thumbnail: /img/thumbnail/mountain2.jpg
---

广告是移动应用非常好的变现模式，作为开发者经常会接很多家的广告平台，每个广告平台都有自己的报表系统，就会有一个各个平台数据汇总分析的需求，首先第一步就需要从各个广告平台获取数据，除了在web页面提供基本的数据导出功能，基本上各个平台都会提供 api 来方便数据拉取的自动化。

admob 是 google 的移动开发者广告平台，同样也提供了完备的 api 来获取报表数据

### 创建 API 凭证

报表 api 使用 oauth 2.0 授权，需要先开通云平台账号，需要填一下账单信息。开通账号后，在平台上创建一个 oauth 2.0 的凭证

![google oauth 授权](/img/images/google_create_oauth.jpg)

创建完成后，`下载JSON` 秘钥文件，保存为 `client_secrets.json`

### 使用 google api 的 python 库

```
sudo pip3 install google-api-python-client
sudo pip3 install oauth2client
```

### 初始化服务

``` python
from apiclient import sample_tools

scope = ['https://www.googleapis.com/auth/adsense.readonly']
client_secrets_file = 'client_secrets.json'

service, _ = sample_tools.init(
    '', 'adsense', 'v1.4', __doc__,
    client_secrets_file,
    parents=[], scope=scope
)
```

### 调用服务 [api](https://developers.google.com/adsense/management/v1.4/reference/accounts/reports/generate)

``` python
results = service.accounts().reports().generate(
    accountId='pub-131xxxxxxxxxxxxx',
    startDate=start.strftime('%Y-%m-%d'),
    endDate=end.strftime('%Y-%m-%d'),
    metric=metric,
    dimension=dimension,
    useTimezoneReporting=True,
).execute()
```

`accountId`: 在 admob 平台的首页上 <https://apps.admob.com/v2/home> 点击自己的头像， `发布商 ID` 就是 `accountId`
`metric`: 指标项，点击、展示、收入等
`dimension`: 维度，日期、app、广告位、国家等
`startDate`: 开始日期，yyyy-mm-dd 格式
`endDate`: 结束时间，yyyy-mm-dd 格式
`useTimezoneReporting`: 使用账户所在的时区

### 参考代码

代码依赖了一个 `client_secrets.json` 的授权文件

``` python
import json
import datetime
import argparse
from apiclient import sample_tools


def collect(start=None, end=None):
    if not start:
        start = datetime.datetime.now() - datetime.timedelta(days=1)
    if not end:
        end = start

    scope = ['https://www.googleapis.com/auth/adsense.readonly']
    client_secrets_file = 'client_secrets.json'

    service, _ = sample_tools.init(
        '', 'adsense', 'v1.4', __doc__,
        client_secrets_file,
        parents=[], scope=scope
    )

    dimension = [
        "DATE", "APP_NAME", "APP_PLATFORM", "AD_UNIT_NAME", "AD_UNIT_ID", "COUNTRY_CODE"
    ]
    metric = [
        "AD_REQUESTS", "CLICKS", "INDIVIDUAL_AD_IMPRESSIONS", "EARNINGS", "REACHED_AD_REQUESTS_SHOW_RATE"
    ]

    results = service.accounts().reports().generate(
        accountId='pub-131xxxxxxxxxxxxx',
        startDate=start.strftime('%Y-%m-%d'),
        endDate=end.strftime('%Y-%m-%d'),
        metric=metric,
        dimension=dimension,
        useTimezoneReporting=True,
    ).execute()

    headers = [i.lower() for i in dimension + metric]
    datas = []
    for row in results.get('rows'):
        data = {}
        for i in range(len(row)):
            data[headers[i]] = row[i]
        datas.append(data)
    return datas


def transform(datas):
    for data in datas:
        # 数据转化
        pass
    return datas


def serialize(datas):
    for data in datas:
        print(json.dumps(data))


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="""Example:
            python3 admob.py -s 20181025 -e 20181028
        """
    )
    parser.add_argument(
        '-s', '--start',
        type=lambda d: datetime.datetime.strptime(d, '%Y%m%d'),
        help='start time',
        default=None
    )
    parser.add_argument(
        '-e', '--end',
        type=lambda d: datetime.datetime.strptime(d, '%Y%m%d'),
        help='end time',
        default=None
    )
    args = parser.parse_args()
    serialize(transform(collect(args.start, args.end)))


if __name__ == '__main__':
    main()
```

### 参考链接

- google 广告平台报表 api:  <https://developers.google.com/adsense/management/v1.4/reference/accounts/reports/generate>
- google 广告平台指标和维度:
  <https://developers.google.com/adsense/management/metrics-dimensions>
- google oauth 授权:  <https://developers.google.com/adwords/api/docs/guides/authentication>
- How to use Google Adsense API to download Adsense data: <https://009co.com/?p=389>
- google admob 平台: <https://apps.admob.com/v2/home>
- google 云平台: <https://console.cloud.google.com/home/dashboard>
- adsense 平台凭据: <https://console.cloud.google.com/apis/credentials>