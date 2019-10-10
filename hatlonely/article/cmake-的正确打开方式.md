---
title: cmake 的正确打开方式
date: 2018-05-30 13:52:50
tags: [cmake, cmakelists.txt, c++, 构建工具, 单元测试, 依赖管理, makefile]
thumbnail: /img/thumbnail/grassland.jpg
---

用了那么多年 c++，今天才搞明白 cmake 该怎么用……

cmake 是一个跨平台的 c++ 构建工具，与 makefile 类似，但是 makefile 更关注依赖，cmake 更关注构建本身，所以语法上要比makefile 要简洁清晰一些，而最近发现 cmake 原来还自带了依赖管理的功能，瞬间觉得之前的用法都太低级了……

### 依赖管理

```
include(ExternalProject)

add_custom_target(third)

ExternalProject_Add(
    google_gtest
    URL https://github.com/google/googletest/archive/release-1.8.0.zip
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/gtest
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${DMP_CLIENT_SOURCE_DIR}/third/gtest/build -DBUILD_SHARED_LIBS=OFF
)
add_dependencies(third google_gtest)
```

上面这段代码就可以自动下载 `gtest` 依赖到本地的 `third/gtest/` 目录，并安装在 `third/gtest/build` 下，这个目录下面将有两个目录， `include` 头文件以及 `lib` 库文件

这里核心的命令是 ExternalProject_Add，功能很强大，支持不同的地址去获取依赖，可以是打包文件的 `URL`，比如 github 上的某个项目的 tag，或者像 boost 这种，在官网提供的下载链接，也可以直接是 `GIT_REPOSITORY`，一般建议直接使用打包的 tag，因为比较快，而且有固定的 tag，比较好做版本管理，但是有些项目引用了外部项目需要执行 `git submodule update --init`，这种就比较适合用 git 地址，会自动下载依赖模块

另外就是编译这个过程，如果是标准的使用 cmake 构建的项目，基本不需要额外的配置，会自动编译，我一般习惯设置一个编译后的 install 目录，可以通过 `CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${DMP_CLIENT_SOURCE_DIR}/third/gtest/build` 设置 cmake 的参数来实现，还有一些直接使用 makefile 构建的项目，需要自己去配置这个构建的过程，还有就是像 boost 这种，自己搞了工具，反正基本上每个库都会有些不一样，都会有些小问题需要解决，还是挺麻烦的，但是多了之后也都是一样的套路

总结一下就是这个功能有，但是使用起来还是挺麻烦的，也有人为了简化这种配置，基于这个功能整了一个 cpm，可惜现在已经不再维护了，而且里面很多库也都找不到

在发现这个依赖管理之前，我们是通过 shell 脚本来下载依赖的，虽然丑陋一点，但也基本能解决依赖的问题，相比之下，这种方式统一在了 `CMakeLists.txt` 里面，可读性上会更好一些，使用上面编译安装的命令都统一了，不需要执行额外的脚本，也会更方便一些

但是依旧很丑陋……可能历史的包袱太重吧，各种各样的库，五花八门的构建方式，cmake 能做到这样已经很不错了

### 添加头文件目录和库搜索目录

```
include_directories(
    "${DMP_CLIENT_SOURCE_DIR}/third/gtest/build/include"    
)

link_directories(
    "${DMP_CLIENT_SOURCE_DIR}/third/gtest/build/lib"
)
```

这样我们就能使用刚刚下载的 gtest 依赖了

### 生成 proto 代码

```
add_custom_command(
    OUTPUT ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.pb.cc ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.pb.h
    DEPENDS ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.proto
    COMMAND ${DMP_CLIENT_SOURCE_DIR}/third/protobuf/build/bin/protoc -I. --cpp_out=. dmpval_pb_message.proto
    WORKING_DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/proto
)
add_custom_target(
    pbout
    DEPENDS ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.pb.cc
)
add_dependencies(pbout third)
```

add_custom_command 可以执行自定义的命令，然后再使用 add_custom_target 生成一个 pbout 的目标供下面的可执行程序依赖

### 添加可执行文件

```
add_executable(test_dmpkey test/dmpkey_test.cpp ${dmpkey_source})
add_dependencies(test_dmpkey third pbout)
target_link_libraries(
    test_dmpkey
    gtest
    murmur3
    pthread
)
```

使用代码文件生成一个测试的可执行程序 `test_dmpkey`，并让这个可执行程序依赖第三方依赖 `third` 和我们的 proto 编译结果 `pbout`

### 单元测试

```
enable_testing()
add_test(NAME dmpkey_test COMMAND test_dmpkey)
add_test(NAME dmpval_test COMMAND test_dmpval)
```

增加单元测试比较容易，使用 add_test 命令，在 COMMAND 后面添加需要执行的测试命令即可，添加后就可以使用 `make test` 执行单测了

### make install

```
install(TARGETS dmpclient DESTINATION lib)
install(DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/include/datasource DESTINATION include/dmpclient)
```

install 更简单，指定源和目标即可

### CMakeLists.txt 参考代码

```
cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(DMP_CLIENT)

find_package(Threads REQUIRED)
include(ExternalProject)

add_custom_target(third)

ExternalProject_Add(
    google_gtest
    URL https://github.com/google/googletest/archive/release-1.8.0.zip
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/gtest
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${DMP_CLIENT_SOURCE_DIR}/third/gtest/build -DBUILD_SHARED_LIBS=OFF
)
add_dependencies(third google_gtest)

ExternalProject_Add(
    google_protobuf
    URL https://github.com/google/protobuf/archive/v3.5.2.zip
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/protobuf
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND sh autogen.sh && ./configure --prefix=${DMP_CLIENT_SOURCE_DIR}/third/protobuf/build --disable-shared && make -j8
    INSTALL_COMMAND make install
)
add_dependencies(third google_protobuf)

ExternalProject_Add(
    boostorg_boost
    URL https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/boost
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND sh bootstrap.sh && ./b2 link=static -j8
    INSTALL_COMMAND ./b2 install --prefix=${DMP_CLIENT_SOURCE_DIR}/third/boost/build
)
add_dependencies(third boostorg_boost)

ExternalProject_Add(
    peterscott_murmur3
    URL https://github.com/PeterScott/murmur3/archive/master.zip
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/murmur3
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND gcc -c murmur3.c && ar rcs libmurmur3.a murmur3.o
    INSTALL_COMMAND mkdir -p ../../build/{include,lib} && cp murmur3.h ../../build/include && cp libmurmur3.a ../../build/lib
)
add_dependencies(third peterscott_murmur3)

ExternalProject_Add(
    vipshop_hiredis_vip
    URL https://github.com/vipshop/hiredis-vip/archive/0.3.0.zip
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/hiredis-vip
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND PREFIX=${DMP_CLIENT_SOURCE_DIR}/third/hiredis-vip/build make install
    COMMAND rm -rf ${DMP_CLIENT_SOURCE_DIR}/third/hiredis-vip/build/lib/*.dylib
)
add_dependencies(third vipshop_hiredis_vip)

ExternalProject_Add(
    aerospike_aerospike_client_c
    GIT_REPOSITORY git@github.com:aerospike/aerospike-client-c.git
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/aerospike-client-c
    BUILD_IN_SOURCE true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND ls target | xargs -I {} cp -r target/{}/ ${DMP_CLIENT_SOURCE_DIR}/third/aerospike-client-c/build
    COMMAND rm -rf ${DMP_CLIENT_SOURCE_DIR}/third/aerospike-client-c/build/lib/libaerospike.dylib
)
add_dependencies(third aerospike_aerospike_client_c)

ExternalProject_Add(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/v3.1.2.zip
    PREFIX ${DMP_CLIENT_SOURCE_DIR}/third/json
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${DMP_CLIENT_SOURCE_DIR}/third/json/build -DBUILD_SHARED_LIBS=OFF
)
add_dependencies(third nlohmann_json)

set(CMAKE_CXX_FLAGS "-std=c++11 -O2 -g")

# generate proto struct
add_custom_command(
    OUTPUT ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.pb.cc ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.pb.h
    DEPENDS ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.proto
    COMMAND ${DMP_CLIENT_SOURCE_DIR}/third/protobuf/build/bin/protoc -I. --cpp_out=. dmpval_pb_message.proto
    WORKING_DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/proto
)
add_custom_target(
    pbout
    DEPENDS ${DMP_CLIENT_SOURCE_DIR}/proto/dmpval_pb_message.pb.cc
)
add_dependencies(pbout third)

include_directories(
    "${DMP_CLIENT_SOURCE_DIR}/include"
    "${DMP_CLIENT_SOURCE_DIR}/proto"
    "${DMP_CLIENT_SOURCE_DIR}/third/boost/build/include"
    "${DMP_CLIENT_SOURCE_DIR}/third/murmur3/build/include"
    "${DMP_CLIENT_SOURCE_DIR}/third/gtest/build/include"
    "${DMP_CLIENT_SOURCE_DIR}/third/protobuf/build/include"
    "${DMP_CLIENT_SOURCE_DIR}/third/hiredis-vip/build/include"
    "${DMP_CLIENT_SOURCE_DIR}/third/aerospike-client-c/build/include"
    "${DMP_CLIENT_SOURCE_DIR}/third/json/build/include"
)

link_directories(
    "${DMP_CLIENT_SOURCE_DIR}/third/gtest/build/lib"
    "${DMP_CLIENT_SOURCE_DIR}/third/protobuf/build/lib"
    "${DMP_CLIENT_SOURCE_DIR}/third/murmur3/build/lib"
    "${DMP_CLIENT_SOURCE_DIR}/third/hiredis-vip/build/lib"
    "${DMP_CLIENT_SOURCE_DIR}/third/aerospike-client-c/build/lib"
    "${DMP_CLIENT_SOURCE_DIR}/third/json/build/lib"
)

aux_source_directory(src/dmpkey dmpkey_source)
aux_source_directory(src/dmpval dmpval_source)
aux_source_directory(src/datasource datasource_source)
aux_source_directory(src/dmpclient dmpclient_source)
set(proto_source proto/dmpval_pb_message.pb.cc)
set(all_source ${dmpkey_source} ${dmpval_source} ${datasource_source} ${dmpclient_source} ${proto_source})

add_executable(test_dmpkey test/dmpkey_test.cpp ${dmpkey_source})
add_dependencies(test_dmpkey third pbout)
target_link_libraries(
    test_dmpkey
    gtest
    murmur3
    pthread
)

add_executable(test_dmpval test/dmpval_test.cpp ${dmpval_source} ${proto_source})
add_dependencies(test_dmpval third pbout)
target_link_libraries(
    test_dmpval
    gtest
    protobuf
    pthread
)

add_executable(test_redis_string test/redis_string_test.cpp src/datasource/redis_string.cpp)
add_dependencies(test_redis_string third pbout)
target_link_libraries(
    test_redis_string
    gtest
    hiredis_vip
    pthread
)

add_executable(test_aerospike test/aerospike_test.cpp src/datasource/aerospike.cpp)
add_dependencies(test_aerospike third pbout)
target_link_libraries(
    test_aerospike
    gtest
    pthread
    aerospike
    ssl
    crypto
    z
)

add_executable(test_dmpclient test/dmpclient_test.cpp ${all_source})
add_dependencies(test_dmpclient third pbout)
target_link_libraries(
    test_dmpclient
    gtest
    hiredis_vip
    aerospike
    ssl
    crypto
    z
    protobuf
    murmur3
    pthread
)

enable_testing()
add_test(NAME dmpkey_test COMMAND test_dmpkey)
add_test(NAME dmpval_test COMMAND test_dmpval)
add_test(NAME redis_string_test COMMAND test_redis_string)
add_test(NAME aerospike_test COMMAND test_aerospike)
add_test(NAME dmpclient_test COMMAND test_dmpclient)

add_library(dmpclient STATIC ${all_source})
add_dependencies(dmpclient third pbout)

install(TARGETS dmpclient DESTINATION lib)
install(DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/include/datasource DESTINATION include/dmpclient)
install(DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/include/localcache DESTINATION include/dmpclient)
install(DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/include/dmpclient DESTINATION include/dmpclient)
install(DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/include/dmpkey DESTINATION include/dmpclient)
install(DIRECTORY ${DMP_CLIENT_SOURCE_DIR}/include/dmpval DESTINATION include/dmpclient)
```

### 参考链接

- cmake 官方文档：<https://cmake.org/cmake/help/v3.11/manual/cmake-commands.7.html>
- ExternalProject：<https://cmake.org/cmake/help/v3.11/module/ExternalProject.html>
