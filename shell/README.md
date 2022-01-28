## MRFFToolChain Build Shell

**What's MRFFToolChain?**

MRFFToolChain products was built for my FFmepg tutorial : [https://github.com/debugly/FFmpegTutorial](https://github.com/debugly/FFmpegTutorial).

At present MRFFToolChain contained OpenSSL、FFmpeg 、Lame、X264、Fdk-aac、libyuv、libopus.

~~All MRFFToolChain lib were made to Pod in [MRFFToolChainPod](https://github.com/debugly/MRFFToolChainPod/).~~

## Folder structure

```
.
├── README.md
├── config          #ffmpeg 功能裁剪配置
├── extra           #ffmpeg，openssl 等库的源码
├── init-ffmpeg.sh  #ffmpeg 源码初始化脚本
├── init-openssl.sh #openssl 源码初始化脚本
├── ios             #ios 平台编译工作目录
├── mac             #macos 平台编译工作目录
├──tools            #编译脚本通用依赖

├── README.md
├── build           #编译目录
│   ├── product     #编译产物
│   └── src         #构建时源码仓库
├── extra           #源码仓库，构建时根据架构放到 build/src/$plat/$libname-$arch
│   ├── fdk-aac
│   ├── ffmpeg
│   ├── lame
│   ├── libyuv
│   ├── openssl
│   ├── opus
│   └── x264
├── ffconfig        #ffmpeg 编译选项
│   ├── configure.md
│   ├── module-default.sh
│   ├── module-full.sh
│   ├── module-lite-hevc.sh
│   ├── module-lite.sh
│   └── module.sh -> module-full.sh
├── init-any.sh     #初始化源码仓库包括不同架构的仓库
├── init-cfgs       #初始化脚本会调用，里面是库的名称，git仓库地址等信息
│   ├── fdk-aac
│   ├── ffmpeg
│   ├── lame
│   ├── libyuv
│   ├── openssl
│   ├── opus
│   └── x264
├── ios             # TODO
│   ├── compile-ffmpeg.sh
│   ├── compile-openssl.sh
│   ├── make-ffmpeg-pod
│   ├── make-openssl-pod
│   └── tools
├── macos           #mac平台的编译脚本
│   ├── compile-any.sh
│   ├── compile-cfgs
│   ├── do-compile
│   └── macos-env.sh
└── tools           #初始化等依赖的脚本
    ├── env_assert.sh
    ├── init-repo.sh
    ├── pull-repo-base.sh
    └── pull-repo-ref.sh
```

## Use Mirror

从 Github 克隆代码可能很慢，所以最好在内网做一套仓库镜像，这样团队协作时，其他人就能很快的克隆代码了，时间不应浪费在这里。

## Init Lib Repo

脚本参数比较灵活，可根据需要搭配使用

```
#准备 iOS 和 macOS 平台所有库的源码
./init-all.sh all
#准备 iOS 平台源码所有库的源码
./init-all.sh ios
#准备 macOS 平台源码所有库的源码
/init-all.sh macos
#准备 ios 平台的某些库的源码
/init-all.sh ios "openssl ffmpeg"
#准备 macOS 平台的某些库的源码
/init-all.sh macos "openssl ffmpeg"
#准备 iOS 和 macOS 平台的某些库的源码
/init-all.sh macos "openssl ffmpeg"
```

## Compile

根据编译的平台，进入相应的目录，比如编译 macos 平台：

```
cd macos
# 编译 macos arm64 架构下的所有库
./compile-any.sh build all arm64
# 编译 macos x86_64 架构下的所有库
./compile-any.sh build all x86_64
# 编译 macos 所有架构下的所有库
./compile-any.sh build all
```

编译指定库，比如 openssl，ffmpeg

```
cd macos
# 编译 macos arm64 架构
./compile-any.sh build "openssl ffmpeg" arm64
# 编译 macos x86_64 架构
./compile-any.sh build "openssl ffmpeg" x86_64
# 编译 macos 所有架构
./compile-any.sh build "openssl ffmpeg"
```

清理 macos 平台编译产物

```
cd macos
# 清理 macos arm64 架构下的所有库的产物
./compile-any.sh clean all arm64
# 清理 macos x86_64 架构下的所有库的产物
./compile-any.sh clean all x86_64
# 清理 macos 所有架构下的所有库的产物
./compile-any.sh clean all
```

将 macos 平台编译产物合并成 fat 版本放到 universal 文件夹下（build 所有架构时会自动lipo，只有手动编译不同架构时才会用到 lipo）

```
cd macos
# 合并 macos 所有架构下的所有库的产物
./compile-any.sh lipo all
# 将 macos arm64 架构下的所有库的产物复制到 universal
./compile-any.sh lipo all arm64
# 将 macos x86_64 架构下的所有库的产物复制到 universal
./compile-any.sh lipo all x86_64
```

编译 ios 平台跟 macos 是一样的流程，只需要 cd 到 ios 目录操作即可。

## Use Your Mirror

如果 github 上的仓库克隆较慢，或者需要使用私有仓库的话，可在执行编译脚本前声明对应的环境变量即可！

| 名称 | 默认仓库 | 使用镜像 |
|---|---|---|
| FFmpeg | https://github.com/bilibili/FFmpeg.git | export GIT_FFMPEG_UPSTREAM=git@xx:yy/ffmpeg.git |
| libYUV | https://github.com/lemenkov/libyuv.git | export GIT_FDK_UPSTREAM=git@xx:yy/libyuv.git
| OpenSSL | https://github.com/openssl/openssl.git | export GIT_OPUS_UPSTREAM=git@xx:yy/openssl.git |
| Opus | https://gitlab.xiph.org/xiph/opus.git | export GIT_OPUS_UPSTREAM=git@xx:yy/opusfile.git  |