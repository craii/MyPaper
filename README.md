# MyPaper
<img width="1004" height="1012" alt="app_icon_1024" src="https://github.com/user-attachments/assets/a3f3af6c-f120-4858-b9a2-c3f9d252e41a" />

wallpapaer app Pap.er temporary fix

#做这个东西的
出于对 Pap.er app 的喜爱。升级 macOS 26后，因崩溃而无法继续使用的 Pap.er ，给开发者 发送邮件  也如石沉大海，没有回应。 因而写了临时替代版。当前公开版本仅能读取本地壁纸。
<img width="1509" height="982" alt="macos_1" src="https://github.com/user-attachments/assets/dbd40a68-a00b-4ba0-8bdd-4511113559b5" />
<img width="1506" height="982" alt="macos_3" src="https://github.com/user-attachments/assets/0db06e33-8e9b-462f-82e1-cdb8174fd9d3" />
<img width="1512" height="982" alt="macos_2" src="https://github.com/user-attachments/assets/1a27485d-5c3d-4642-a5a4-3c2538e703b3" />

<img width="2557" height="1388" alt="win_3" src="https://github.com/user-attachments/assets/67d4c8ab-9978-4a3f-952d-b3a266c8f9e7" />
<img width="2560" height="1400" alt="win_2" src="https://github.com/user-attachments/assets/8e5dd80a-ed9e-46b5-9874-ced2e3251102" />
<img width="2560" height="1399" alt="win_1" src="https://github.com/user-attachments/assets/0f923117-e352-44ac-9409-6e36d091f52f" />

# 自行编译或在release中下载
## Windows:
**安装 QT:**

项目使用的是QT 6.10.1 (windows: MingW13.1.0, macOS: gcc), 因在 macOS 中使用低于 6 的 QT 会不可避免在编译时自动添加 AGL，一个macOS已经弃用的动态连接库，导致编译失败。不是不能解决，但为什么要浪费时间呢？前往官网下载QT安装完成后，应该能够在开始菜单中找到。
<img width="630" height="623" alt="Snipaste_2026-01-01_12-34-19" src="https://github.com/user-attachments/assets/c204fab3-476c-4e84-8a24-5b98dea4d8b1" />
<img width="650" height="585" alt="Snipaste_2026-01-01_12-35-56" src="https://github.com/user-attachments/assets/f137ebea-87b0-4081-b08c-3d21684b0d43" />
使用QT自带的终端(不是cmd! 不是cmd! 不是cmd!)，使用这个终端 ，QT会自动链接需要的动态库。

以下操作在 QT 自带的终端中完成：

` D:`

**拉取仓库**
` git clone https://github.com/craii/MyPaper.git && cd MyPaper `


**配置and 编译 release 版本：**
` cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
`cmake --build build-release`

编译成功后，生成的程序在这里  MyPaper \build-release\wallpaper.exe 。但注意，这时候的 wallpaper.exe 只能在自己的电脑中运行，因为运行必须的 QT链接库都没有带。如果让它能够在其他电脑中运行，需要将 wallpaper.exe  和 必要的库一起打包。

**打包 Portable 版本：**
`windeployqt wallpaper.exe`

**最后一步：**

将 MyPaper 文件夹中的 app_icon.ico 、 app_icon.png 拷贝到 build-release 文件夹中。 此时，如果要分享，把build-release整个文件夹拷走就行了。


## Windows:
以下命令在终端(terminal)中执行即可:
检查QT版本：

安装 QT 和 cmake 后，检查QT的版本 ，版本号应该是 6.10.1 或更高：
`qtpaths --qt-version`

如果你之前安装过 anaconda 或者 pyside6/pyqt等，上述命令的运行结果可能是 5.xx.x ,此时，需要额外执行操作:
在 `~/.bash_profile` 中添加
` export PATH=/Users/eliascheung/Qt/6.10.1/macos/bin:$PATH`

**拉取仓库:**
首先，我们切换到 documents 文件夹，这样方便我们在finder中找到它

`cd documents`
`git clone https://github.com/craii/MyPaper.git && cd MyPaper`
如此完成后，会在 ~/documents/ 中看到一个 MyPaper  文件夹，其中包含本项目代码和资源。

**创建 release 文件夹：**
`rm -rf build-macos-release && mkdir build-macos-release && cd build-macos-release`

**配置 release 版本：**
`
cmake .. \

-DCMAKE_BUILD_TYPE=Release \

-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \

-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
`

**编译 release 版本：**
`cmake --build . --config Release`

**打包 wallpaper.app 版本：**
`macdeployqt wallpaper.app -qmldir=..`


**APP临时签名，确保有足够的权限：**

因为在 photobackend.cpp 中：

切换壁纸功能是通过终端执行 AppleScript 实现的，
读取/保存壁纸等需要访问磁盘
这些都需要需要权限。qt自动签名可能会导致无法获取这些权限，从而无法实现切换壁纸。
`codesign --force --deep --sign - YourAppName.app`





