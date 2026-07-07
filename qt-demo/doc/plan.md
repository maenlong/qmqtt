# MQTT Demo — MVP 规划

## 目标

验证 MQTT 信令通道的技术可行性，回答以下问题：

- EMQX qmqtt 能否稳定连接自建 Broker（tcp / wss）？
- 用户名+密码认证是否正常工作？
- JSON 格式消息能否正确收发？
- Topic 路由（`user/{imAccid}/inbox`）是否可用？
- 代理模式下连接是否正常？

## MVP 范围

不涉及具体信令内容、NIM 兼容、去重、重连逻辑。

### 连接

- [x] tcp 直连（已验证 ✅ test.mosquitto.org:1883 连接/订阅/发布/接收正常）
- [ ] wss 连接（443 端口 + TLS）
- [ ] 用户名（imAccid）、密码（imToken）
- [x] Keep Alive 30s
- [x] Clean Session true
- [x] Client ID 按格式生成：`{平台前缀}-{UUID前8位}`
- [ ] 代理支持（EMQX qmqtt 不支持 QNetworkProxy）
- [x] 断线自动重连（EMQX qmqtt 内置，指数退避）

### 消息

- [x] QoS 1（已验证 ✅）
- [ ] Topic 格式：`user/{targetImAccid}/inbox`
- [ ] JSON 消息体（包含基本字段即可）

### 收发

- [ ] 订阅 `user/{selfImAccid}/inbox`
- [ ] 向任意 `user/{targetImAccid}/inbox` 发布消息
- [ ] 消息日志显示 JSON 内容

## 不包含（后续再做）

| 内容 | 说明 |
|------|------|
| 具体信令指令（CS/CSA/CSR/...） | MVP 只发 JSON，不关心语义 |
| SessionID UUID v4 | 后续信令业务再加 |
| 双通道去重 | NIM 通道集成时再做 |
| `~` 分隔格式解析 | NIM 兼容阶段再做 |
| 呼叫状态机 / 振铃 UI | 业务逻辑阶段再做 |

## 文件结构

```
qt-demo/
├── doc/
│   └── plan.md
├── main.cpp
├── mainwgt.h / mainwgt.cpp / mainwgt.ui
├── mqtt-client.pro
└── build.bat
```

## 界面布局

- Connection 组：Host / Port / Type (TCP/WSS) / Client ID / Username / Password / Keep Alive / Proxy / Connect / Disconnect / Status
- Subscribe 组：Self imAccid / Subscribe 按钮
- Publish 组：Target imAccid / JSON 输入框 / Publish 按钮
- Log 组：只读消息日志

## 交付物

一个可运行的 Qt 程序，基于 EMQX qmqtt，能连接到指定的 MQTT Broker（tcp/wss），用账号密码认证，向目标用户的 inbox topic 发送 JSON 消息并接收。
