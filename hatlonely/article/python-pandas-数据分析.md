---
title: python pandas 数据分析
date: 2019-01-06 20:05:26
tags: [python, pandas, 数据分析]
thumbnail: /img/thumbnail/field2.jpg
---

pandas 是一个强大的数据处理库，用这个库，我们可以非常方便地对数据进行加载，抽取，转化，以及输出


### 数据加载

pandas 可以读取各种格式的数据，包括 csv，json 等等

``` python
df = pd.read_json("test.json", lines=True)
df = pd.read_csv("test.csv", sep=",", encoding="utf-8")
df = pd.read_csv(StringIO(text))
df = pd.DataFrame.from_records(items)
```

### 数据写入

pandas 可以输出成各种格式

``` python
print(df.to_json(orient="records", lines=True))
print(df.to_csv(encoding="utf-8"))
print(df.to_html())
writer = pd.ExcelWriter("test.xlsx")
df.to_excel(writer, "sheet1", index=False)
writer.save()
```

### 列选择

``` python
df = df[["col1", "col2", "col3"]]
```

### 行选择

```
df = df.loc[
    (df["col1"] == "val1") &
    ((df["col2"] == "val2") | (df["col3"] == "val3"))
]
```

### 列过滤

``` python
df = df.drop(["col1", "col2"], axis=1)
```

### 行过滤

``` python
df = df.drop(df.loc[df["col1"] == "val1"].index)
```

### 列重命名

``` python
df = df.rename(columns={"col1": "rename_col1"})
```

### 行更新

``` python
df = df.loc[df["col1"] == "val1", "col1"] = df["col1"] / 6.7
```

### 新增列

``` python
df["col3"] = df["col1"] * 1000 / df["col2"]
```

### 聚合

``` python
df = df.groupby(["col1", "col2"], as_index=False).sum()
```

### 排序

``` python
df = df.sort_values(by=["col1", "col2"], ascending=[True, False])
```

### 级联

``` python
df = pd.merge(df1, df2, how="left")
```

### 类型修改

``` python
df["@timestamp"] = "2019-01-15T00:00:00"
df["date"] = pd.to_datetime(df["@timestamp"])
df["date"] -= datetime.timedelta(days=2)

df = df.astype({
    "revenue": float, "impression": int,
})
```

### 填充空值

``` python
df.loc[df["col1"].isna(), "col1"] = 0
```
