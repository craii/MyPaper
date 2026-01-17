# MyPaper (Mac App Pap.er temporary substitution)
<img width="251" height="253" alt="app_icon" src="https://github.com/user-attachments/assets/1d66cb8a-e14d-498d-be60-eed3e6eedb91" />


wallpaper app Pap.er temporary fix <br/>

因最爱壁纸软件 [**Pap.er**](https://www.paperapp.net/) 崩溃后作者不修复的无奈之作。 适配 **macOS** 和 **Windows** 。

# 初衷
做这个东西是出于对 Pap.er app 的喜爱，也是作为学习 cpp 之后的练习(否则我实在找不到任何理由用cpp写网络服务)。升级 macOS 26后，因崩溃而无法继续使用的 Pap.er ，给开发者 发送邮件  也如石沉大海，没有回应。 因而写了临时替代版。
# 一些注意事项
原本想做接入 **unsplash**的 api 实现更新壁纸的，但创建**unsplash app**时发现其使用协议**不允许做壁纸类app**；而且如此做也侵犯了 **Pap.er** 的权益。 *因此，当前公开版本仅能读取本地壁纸。 当然，后续会做服务器端，实现热门壁纸更新。* 
## 当前进度
   1. 【✔】读取本地壁纸并切换；
   2. 【✔】定时更新壁纸；
   3. 【✔】壁纸服务器(服务器端)；
   4. 【❌】App端读取服务器获取壁纸；
   5. 【❌】美化UI


# 截图
<img width="1509" height="982" alt="macos_1" src="https://github.com/user-attachments/assets/dbd40a68-a00b-4ba0-8bdd-4511113559b5" />
<img width="1506" height="982" alt="macos_3" src="https://github.com/user-attachments/assets/0db06e33-8e9b-462f-82e1-cdb8174fd9d3" />
<img width="1512" height="982" alt="macos_2" src="https://github.com/user-attachments/assets/1a27485d-5c3d-4642-a5a4-3c2538e703b3" />

<img width="2557" height="1388" alt="win_3" src="https://github.com/user-attachments/assets/67d4c8ab-9978-4a3f-952d-b3a266c8f9e7" />
<img width="2560" height="1400" alt="win_2" src="https://github.com/user-attachments/assets/8e5dd80a-ed9e-46b5-9874-ced2e3251102" />
<img width="2560" height="1399" alt="win_1" src="https://github.com/user-attachments/assets/0f923117-e352-44ac-9409-6e36d091f52f" />


# App下载
你可以在releases section 中下载适用于windows或mac的版本

[**Releases**](https://github.com/craii/MyPaper/releases)


# App编译

**App(Windows and macOS)**: [**App端说明与编译方式**](https://github.com/craii/MyPaper/blob/main/client/README.md)

**Server(Linux: rhel fedora centos anolis)**: [**Server端编译**](https://github.com/craii/MyPaper/blob/main/server/README.md)

