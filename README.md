I5做客户端，PC做服务器端，
I5从json文档读取配置，连接指定的PC程序，
将json格式的心跳消息定时发给PC端显示
PC端返回给I5当前的PC时间
I5收到后修改本地时间

通信全部用json文本串,
两端的收发json消息均应打印出来

