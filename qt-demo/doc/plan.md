# MQTT Demo — 开发规划

## 目标

验证 MQTT 信令通道的技术可行性，回答以下问题：

- EMQX qmqtt 能否稳定连接自建 Broker（tcp / ws / wss）？
- 用户名+密码认证是否正常工作？
- JSON 格式消息能否正确收发？
- Topic 路由（`user/{imAccid}/inbox`）是否可用？

## 已完成

### 项目结构

```
qt-demo/
├── doc/
│   └── plan.md
├── main.cpp                    # 程序入口
├── mqttclientwgt.h / .cpp      # UI 层，收集输入并展示状态
├── mqttclientwgt.ui             # UI 布局
├── mqttclientmgr.h / .cpp      # MQTT 业务层，纯 QObject，与 UI 无关
├── mqtttopicbuilder.h / .cpp   # Topic 业务规则，可供 GUI/CLI 复用
├── mqttproxymanager.h / .cpp   # 全局代理配置，可供 GUI/CLI 复用
├── mqtt-client.pro             # Qt 工程文件
├── mqtt-client.qrc             # 资源文件（i18n .qm）
├── mqtt-client_zh_CN.ts / .qm  # 中文翻译文件
└── build_*.bat / build_*.sh    # 构建脚本
```

### 架构

```
用户输入 → MqttClientWgt (UI)
                ├── MqttTopicBuilder (Topic 规则)
                ├── MqttProxyManager (代理配置)
                └── MqttClientMgr (MQTT 业务层)
                           ↓
                      QMQTT::Client (SDK)
```

- `MqttClientMgr`：纯 QObject，封装连接/订阅/发布/断开，通过 signals 通知结果，不包含任何 UI 代码。
- `MqttTopicBuilder`：集中封装 `user/{imAccid}/inbox` Topic 规则，避免 GUI/CLI 重复拼接字符串。
- `MqttProxyManager`：校验并应用 None/HTTP/SOCKS5 全局代理配置，不依赖 UI。
- `MqttClientWgt`：Qt Widgets 界面，只负责收集参数、调用可复用模块并展示日志/状态。

### 功能状态

#### 连接
- [x] TCP 直连（已验证 broker.emqx.io:1883 连接/订阅/发布/接收正常）
- [x] WS（WebSocket）连接
- [x] WSS（WebSocket + TLS）连接
- [x] WebSocket Path / Origin 可配置，使用 QUrl 构建连接地址
- [x] 自定义 CA 追加到系统 CA，证书加载失败时阻止连接
- [x] 用户名/密码认证
- [x] Keep Alive 30s
- [x] Clean Session true/false
- [x] Client ID 按格式生成 `qt_demo_{UUID前8位}`
- [x] 断线自动重连（EMQX qmqtt 内置，指数退避，间隔 5s）
- [x] 代理支持（None / HTTP / SOCKS5，全局 `QNetworkProxy::setApplicationProxy()`）

#### 遗嘱消息（Last Will）
- [x] Will Topic / Will Message / QoS / Retain

#### 消息
- [x] QoS 0/1/2（已验证 QoS 1）
- [x] Topic 格式：`user/{targetImAccid}/inbox`
- [x] JSON 消息体

#### 收发
- [x] 订阅 `user/{selfImAccid}/inbox`
- [x] 向任意 `user/{targetImAccid}/inbox` 发布消息
- [x] 消息日志显示 JSON 内容

#### 国际化
- [x] 运行时可切换 EN / 中文
- [x] UI 控件标签翻译
- [x] Tooltip 翻译
- [x] 日志消息翻译

### 约定

- 类命名：`MqttClientWgt`、`MqttClientMgr`，前缀 `Mqtt` + 模块名
- 槽函数：`on_控件名_信号名()`（Qt Designer 自动关联）、`slot_onXxx()`（手动连接）
- 信号：`sig_xxx()` 前缀
- 成员变量：`m_` 前缀
- 命名空间：`Ui::MqttClientWgt`（Qt Designer 生成）
- i18n：`changeEvent()` + `applyTranslations()`

## 待完成

| 内容 | 说明 | 优先级 |
|------|------|--------|
| CLI 入口 + 命令分发 | `main.cpp` 分流 GUI / CLI 模式，`CliCommandHandler` 解析命令输出 JSON Lines | P0 |
| CLI 自动测试套件 | `CliAutoTester` 用 `QProcess` 发命令给自身，对比预期，输出 PASS/FAIL | P1 |
| config.ini 配置 | 从 ini 读取 broker 地址和认证信息，避免重复输入 | P2 |
| IPC 后台驻守 | `--headless` + `QLocalServer` IPC，使命令行可操作持久连接 | P3 |

## 不包含

| 内容 | 说明 |
|------|------|
| 具体信令指令（CS/CSA/CSR/...） | 只发 JSON，不关心语义 |
| SessionID UUID v4 | 后续信令业务再加 |
| 双通道去重 | NIM 通道集成时再做 |
| `~` 分隔格式解析 | NIM 兼容阶段再做 |
| 呼叫状态机 / 振铃 UI | 业务逻辑阶段再做 |
