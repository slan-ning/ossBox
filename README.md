ossBox
======
2013-02-27 this project has change to github.and i will promote this software to be better good use.

a free aliyun oss client

一个功能强大的阿里云oss客户端，使用bsd开源协议，你可以自由使用本软件与代码。 本软件为阿里云OSS开放存储服务的第三方客户端。 实现了上传，下载，删除，新建文件夹等功能。

0.3.1版本更新日志：
====

修正了大于10m，采用分块传输的文件不设置content-type的bug  
修正了登录失败，不能修改id与key继续登录的bug  
优化的登录提示与流程  

0.3版本更新日志：
====
主要更新：

阿里云内网主机能使用内网模式传输文件了（内网流量免费）  
采用了阿里云的最新内外网域名与API  
修正了一些显示问题  

0.2版本更新日志：
====
新功能
  
支持进度条，上传百分比，可以实时看到当前上传进度。  
新加提取文件url的功能，你可以右键提取单个文件，或者多个文件（文件夹）里所有访问的url到粘贴板。  
你现在需要再次确认才能删除所选的文件或者文件夹了。  
更加清晰的文件大小显示。  

软件特色
====

本软件使用asio异步网络模型，具有高并发的支持基础，在网络不限制的情况下，能最大限度的利用系统io。  
对小文件支持较好，在同等情况下，大大的提高了大量小文件的传输速率。  
对大文件采用分块传输的方式，并具有失败重传功能，大大提高了大文件的传输成功率。  
跨平台支持。软件在设计时，就采用了boost库来进行功能实现，后期将进行跨平台开发。目前GUI图形客户端可免安装任何文件直接运行在windows下。  

软件使用方法
====

1.打开此软件，填入oss的 AccessID 以及AccessKey(阿里云提供)。  
2.若您想软件记住id以及key，请勾选保存配置按钮。  
3.点击获取bucket。  
4.然后你就可以进行操作了，你可以将大量文件拖拽到软件界面，软件将会对所有拖拽文件进行上传。  
5.你也可以选择相应的文件项，然后右键进行对应的操作。  
祝您使用愉快。   

本项目将进行长期维护，如果有什么功能需求，或者bug反馈请直接联系我 我的邮箱：qhgongzi##gmail.com (将两个#换成@)
