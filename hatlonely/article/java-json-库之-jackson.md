---
title: java json 库之 jackson
date: 2018-03-18 15:15:08
tags: [java, json, jackson]
thumbnail: /img/thumbnail/tree2.jpg
---

jackson 是一个 java json 库，提供了完备的 json 解析，序列化以及反序列化功能

### 依赖配置

在 build.gradle 里面添加依赖配置

```
compile group: 'com.fasterxml.jackson.core', name: 'jackson-core', version: '2.9.4'
compile group: 'com.fasterxml.jackson.core', name: 'jackson-databind', version: '2.9.4'
compile group: 'com.fasterxml.jackson.core', name: 'jackson-annotations', version: '2.9.4'
```

### json 解析

``` golang
String jsonString = "{\"name\": \"hatlonely\" /* comment */, \"birthday\": \"2018-03-18 15:26:37\", \"mails\": [\"hatlonely@foxmail.com\", \"hatlonely@gmail.com\"]}";

JsonFactory jsonFactory = new JsonFactory();
jsonFactory.enable(Feature.ALLOW_COMMENTS);
ObjectMapper objectMapper = new ObjectMapper(jsonFactory);
JsonNode node = objectMapper.readTree(jsonString);

assertThat(node.path("name").asText(), equalTo("hatlonely"));
assertThat(node.path("birthday").asText(), equalTo("2018-03-18 15:26:37"));
assertThat(node.path("mails").size(), equalTo(2));
assertThat(node.path("mails").path(0).asText(), equalTo("hatlonely@foxmail.com"));
assertThat(node.path("mails").path(1).asText(), equalTo("hatlonely@gmail.com"));
```

调用 `ObjectMapper.readTree` 就能讲 json 字符串解析成一个 `JsonNode` 对象，然后通过 `path` 方法就可以获取 json 中各个字段的值了，这种方式可以用来读取 json 格式的配置文件，可以用一个 JsonFactory 打开 ALLOW_COMMENTS 特性，可以在 json 里面加入注释

### 序列化与反序列化

#### 首先定义一个对象

``` golang
class Person {
    String name;

    @JsonFormat(pattern = "yyyy-MM-dd hh:mm:ss")
    Date birthday;

    @JsonProperty("mails")
    List<String> emails;

    // 省略了 getter/setter
}
```

除了支持基本的数据类型，还支持 List 和 Map 类型，甚至还支持 Date 类型，Date 类型默认的格式是 ISO8601 格式，也可以通过 `@JsonFormat` 指定日期格式，通过 `@JsonProperty` 指定字段在 json 中的字段名

#### 序列化

``` golang
SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));

Person person = new Person();
person.setName("hatlonely");
person.setBirthday(dateFormat.parse("2018-03-18 15:26:37"));
person.setEmails(Arrays.asList("hatlonely@foxmail.com", "hatlonely@gmail.com"));

ObjectMapper objectMapper = new ObjectMapper();
String jsonString = objectMapper.writeValueAsString(person);

assertThat(jsonString, equalTo(
        "{\"name\":\"hatlonely\",\"birthday\":\"2018-03-18 03:26:37\",\"mails\":[\"hatlonely@foxmail.com\",\"hatlonely@gmail.com\"]}"));
```

使用 `ObjectMapper.writeValueAsString` 方法就可以序列化成 string

#### 反序列化

``` golang
String jsonString = "{\"name\": \"hatlonely\", \"birthday\": \"2018-03-18 15:26:37\", \"mails\": [\"hatlonely@foxmail.com\", \"hatlonely@gmail.com\"]}";

ObjectMapper objectMapper = new ObjectMapper();
objectMapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
Person person = objectMapper.readValue(jsonString, Person.class);

assertThat(person.getName(), equalTo("hatlonely"));
SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
assertThat(person.getBirthday(), equalTo(dateFormat.parse("2018-03-18 15:26:37")));
assertThat(person.getEmails(), equalTo(Arrays.asList("hatlonely@foxmail.com", "hatlonely@gmail.com")));
```

使用 `ObjectMapper.readValue` 方法就能实现反序列化，可以通过 `configure` 方法设置碰到未知的属性不抛异常

### 参考链接

- Jackson JSON Tutorial：<http://www.baeldung.com/jackson>
- Jackson maven 仓库：<http://mvnrepository.com/search?q=jackson>
- Jackson 框架的高阶应用：<https://www.ibm.com/developerworks/cn/java/jackson-advanced-application/index.html>
- 测试代码链接：<https://github.com/hatlonely/hellojava/blob/master/src/test/java/jackson/JacksonTest.java>
