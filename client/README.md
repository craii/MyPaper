# 自行编译或在release中下载
MyPaper (Mac App Pap.er temporary substitution)

## Windows:
**安装 QT:**

项目使用的是QT 6.10.1 (windows: MingW13.1.0, macOS: gcc), 因在 macOS 中使用低于 6 的 QT 会不可避免在编译时自动添加 AGL，一个macOS已经弃用的动态连接库，导致编译失败。不是不能解决，但为什么要浪费时间呢？前往官网下载QT安装完成后，应该能够在开始菜单中找到。
<img width="630" height="623" alt="Snipaste_2026-01-01_12-34-19" src="https://github.com/user-attachments/assets/c204fab3-476c-4e84-8a24-5b98dea4d8b1" />
<img width="650" height="585" alt="Snipaste_2026-01-01_12-35-56" src="https://github.com/user-attachments/assets/f137ebea-87b0-4081-b08c-3d21684b0d43" />
<br/>
使用QT自带的终端(不是cmd! 不是cmd! 不是cmd!)，使用这个终端 ，QT会自动链接需要的动态库。

以下操作在 QT 自带的终端中完成：

` D:`

**拉取仓库**
<br/>
` git clone https://github.com/craii/MyPaper.git && cd MyPaper `
<br/>

**配置and 编译 release 版本：**
<br/>
` cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
`cmake --build build-release`
<br/>
编译成功后，生成的程序在这里  MyPaper \build-release\wallpaper.exe 。但注意，这时候的 wallpaper.exe 只能在自己的电脑中运行，因为运行必须的 QT链接库都没有带。如果让它能够在其他电脑中运行，需要将 wallpaper.exe  和 必要的库一起打包。

**打包 Portable 版本：**
<br/>
`windeployqt wallpaper.exe`
<br/>

**最后一步：**

将 MyPaper 文件夹中的 app_icon.ico 、 app_icon.png 拷贝到 build-release 文件夹中。 此时，如果要分享，把build-release整个文件夹拷走就行了。


## macOS:
以下命令在终端(terminal)中执行即可:
检查QT版本：

安装 QT 和 cmake 后，检查QT的版本 ，版本号应该是 6.10.1 或更高：
<br/>
`qtpaths --qt-version`
<br/>
如果你之前安装过 anaconda 或者 pyside6/pyqt等，上述命令的运行结果可能是 5.xx.x ,此时，需要额外执行操作:
在 `~/.bash_profile` 中添加
<br/>
` export PATH=/Users/eliascheung/Qt/6.10.1/macos/bin:$PATH`
<br/>

**拉取仓库:**
首先，我们切换到 documents 文件夹，这样方便我们在finder中找到它
<br/>
`cd documents`
`git clone https://github.com/craii/MyPaper.git && cd MyPaper`
<br/>
如此完成后，会在 ~/documents/ 中看到一个 MyPaper  文件夹，其中包含本项目代码和资源。

**创建 release 文件夹：**
<br/>
`rm -rf build-macos-release && mkdir build-macos-release && cd build-macos-release`
<br/>
**配置 release 版本：**
<br/>
`cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15`
<br/>

**编译 release 版本：**
<br/>
`cmake --build . --config Release`
<br/>
**打包 wallpaper.app 版本：**
<br/>
`macdeployqt wallpaper.app -qmldir=..`
<br/>

**APP临时签名，确保有足够的权限：**

因为在 photobackend.cpp 中：

切换壁纸功能是通过终端执行 AppleScript 实现的，
读取/保存壁纸等需要访问磁盘
这些都需要需要权限。qt自动签名可能会导致无法获取这些权限，从而无法实现切换壁纸。
<br/>
`codesign --force --deep --sign - YourAppName.app`





