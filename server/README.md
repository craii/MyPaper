# SERVER SIDE TUTORIAL

## 概况
server 端命令行工具，用于向 MyPaper 提供：
1. 缩略弄壁纸列表，用于列表展示；
2. 全尺寸壁纸列表，用于实际的壁纸设置；
3. 为实现上述目的必备的一些附属功能；
以及 为自己学习了解 cpp 提供情绪价值。

## 后续目标
1. 包装成wordpress插件，方便维护操作；
2. 其他。


## 编译PaperServer

### 编译POCO

拉取 [POCO仓库](https://github.com/pocoproject/poco)到服务器，以便后续编译；

`git clone https://github.com/pocoproject/poco.git && cd poco`

安装ssl库，如果你使用的是阿里云， 请改用`dnf`命令安装：
`apt-get install openssl libssl-dev`

**安装其它必要的依赖：**

`sudo apt-get -y update && sudo apt-get -y install git g++ make cmake libssl-dev libmysqlclient-dev libpq-dev`

阿里云改用：

`sudo yum install -y git gcc-c++ make cmake3 openssl-devel mysql-devel postgresql-devel `

**Buid with Cmake：**

`mkdir cmake-build
 cd cmake-build`

注意要静态编译(-DBUILD_SHARED_LIBS=OFF，如果这里是ON，请自行修改 CMakeLists )，以及把SQLITE也一并build；(因为当时试图编译成单体程序，但opencv在阿里云不知道为什么就是不能静态编译成功，因此就放弃了。)

`cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DENABLE_NETSSL=ON \
    -DENABLE_CRYPTO=ON \
    -DENABLE_MONGODB=OFF \
    -DENABLE_REDIS=OFF \
    -DENABLE_PDF=OFF \
    -DENABLE_ZIP=OFF \
    -DENABLE_PAGECOMPILER=OFF \
    -DENABLE_PAGECOMPILER_FILE2PAGE=OFF \
    -DENABLE_DATA=ON   \
    -DENABLE_DATA_SQLITE=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/local`


**安装头文件和.a 文件：**

`sudo cmake --build . --target install`

安装完成后你可以在 `/usr/local/lib`以及`/usr/local/include` 中找到 .a 以及 .h 文件，如果需要修改安装位置，可在 `CMakeLists` 中修改。

### 安装 OpenCV

如果你的服务器是ubuntu，请改用apt-get

`sudo dnf install opencv opencv-devel`


### 编译PaperServer

**拉取仓库：**

`cd /home && git clone https://github.com/craii/MyPaper && cd MyPaper/server`


**Build PaperServer:**

`mkdir build && cd build`

`cmake .. -DCMAKE_BUILD_TYPE=Release `

`make -j&(nproc)` 

编译完成后，你可以在 `/home/MyPaper/server/build` 中 找到 `PaperServer` 可执行程序。


## 正式使用前的准备工作

开放端口给`PaperServer` 使用，这里以 10001 为例：

`sudo firewall-cmd --permanent --add-port=10001/tcp`

刷新防火墙使其生效：

`sudo firewall-cmd --reload`


根据实际情况修改你的 nginx 配置文件：

```server
{
    listen 80;
    listen 443 ssl;
    listen 443 quic;   
    .............. 
    #原有其他配置
     #poco 服务
    # --- Poco 服务反向代理配置开始 ---
    location ^~ /wallpaper {
        proxy_pass http://127.0.0.1:10001 ;  # 转发到你的 Poco 程序端口
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
 
    location ^~ /images {
        proxy_pass http://127.0.0.1:10001 ;  # 转发到你的 Poco 程序端口
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
 
    location ^~ /refresh{ 
        proxy_pass http://127.0.0.1:10001 ; # 转发到你的 Poco 程序端口 
        proxy_http_version 1.1; 
        proxy_set_header Host $host; 
        proxy_set_header X-Real-IP $remote_addr; 
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for; 
        proxy_set_header X-Forwarded-Proto $scheme;
    }
    # --- Poco 服务反向代理配置结束 ---
#原有其他配置
..........
}
```

测试语法是否正确
`sudo nginx -t`

如果 OK，重载配置（不中断服务）
`sudo systemctl reload nginx`



## 使用PaperServer
此时，可以将全尺寸壁纸上传到 `full_size_images` ,需要注意，为了最好的显示效果，壁纸的宽高比最好是（375, 220）。


**启动程序**
`nohup ./PaperServer 10001 "YourTokens" "YourSite.com"`

**生成压缩图片，以便列表加载：**
使用浏览器访问： `https://YourSite.com/refresh?token=YourTokens`


**修改 MyPaper 设置**
此时，你的你可以在 MyPaper app的设置页面中修改：

服务器地址为：`https://YourSite.com`

访问令牌为：`YourTokens`

至此享用 MyPaper 吧。