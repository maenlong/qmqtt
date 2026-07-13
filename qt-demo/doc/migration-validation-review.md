# MQTT 信令迁移 Demo 验证评估

## 1. 评估说明

- 评估日期：2026-07-13
- 评估对象：`qt-demo` Qt/QMQTT 跨平台 Demo
- 需求依据：《MQTT 信令迁移方案》
- 评估目标：判断当前 Demo 是否已经充分验证“使用 MQTT 替代网易云信 NIM SDK 承载会议呼叫信令”的技术方案

本报告区分以下两类状态：

- **已验证**：存在可运行实现，并且已经通过对应场景的实际测试。
- **已实现待验证**：代码已经提供相关能力，但缺少目标环境或完整场景的验证证据。
- **未完成**：当前实现尚未覆盖迁移方案要求。

## 2. 总体结论

当前 Demo 已经基本完成 **Qt/QMQTT 传输通道可行性验证**，能够证明 QMQTT 可以作为 Qt 客户端的 MQTT 通信基础，完成连接、订阅、发布和接收。

当前 Demo **尚未完成 NIM 信令迁移方案的闭环验证**，不能仅依据现有结果得出“Mqtt 可以完整替代 NIM 信令通道并进入生产迁移”的结论。

可以将当前阶段理解为：

| 判断问题 | 当前结论 |
|---|---|
| QMQTT 能否用于 Qt 客户端收发 MQTT 消息？ | 基本可以，TCP 基础收发已经验证 |
| QMQTT 能否承载 `user/{imAccid}/inbox` Topic？ | 可以，Topic 构建和 QoS 1 收发已经实现 |
| MQTT 能否完整替代现有 NIM 呼叫信令？ | 尚未验证完成 |
| 当前是否具备进入正式迁移和灰度发布的依据？ | 尚不具备，需要完成信令协议、鉴权、重连、多端和跨平台联调 |

## 3. 当前验证矩阵

| 验证项 | 当前状态 | 说明 |
|---|---|---|
| Windows 编译运行 | 已验证 | 已有 Windows Release 构建结果 |
| TCP 连接、订阅、发布、接收 | 已验证 | 已使用 `broker.emqx.io:1883` 完成基础收发测试 |
| WebSocket 连接 | 已实现待验证 | 已有 WS 连接代码，缺少目标服务环境的验证记录 |
| WSS + TLS 连接 | 已实现待验证 | 支持 WSS、自定义 CA、Path 和 Origin，但尚未证明真实 `443 + /message` 链路可用 |
| 用户名和密码认证 | 已实现待验证 | 客户端能够设置凭证，但尚未与 Portal/go-auth 真实鉴权链路联调 |
| `user/{imAccid}/inbox` Topic | 已实现 | 已通过 `MqttTopicBuilder` 统一生成 |
| QoS 1 订阅和发布 | 已实现 | UI 默认使用 QoS 1，能够接收 SUBACK/PUBACK 结果 |
| Keep Alive 30 秒 | 已实现 | 符合迁移方案要求 |
| Clean Session | 已实现 | 默认开启，符合当前方案 |
| MQTT v3.1.1 | 未完成 | Demo 未显式设置版本，当前 qmqtt 默认使用 MQTT v3.1.0 |
| Client ID 平台前缀 | 未完成 | 当前格式为 `qt_demo_{UUID前8位}`，不符合 `{平台前缀}-{appInstanceUuid}` |
| Client ID 跨启动持久化 | 未完成 | 当前每次启动重新生成，未使用 `QSettings` 等持久化机制 |
| JSON 消息格式 | 部分实现 | 可以发送和显示 JSON 文本，但没有序列化、解析和字段校验 |
| CS/CSC/CSA/CSR/CSB 指令 | 未完成 | 当前不识别具体指令语义 |
| CSAN/CSRN/CSBN 多端通知 | 未完成 | 尚未实现同账号多设备通知流程 |
| SessionID UUID v4 | 未完成 | 当前未生成和维护真实 SessionID |
| `senderClientId` 自身回环过滤 | 未完成 | 接收消息时只打印原始 payload，没有过滤自身消息 |
| 自动重连 | 已实现待验证 | 已开启 qmqtt 自动重连，但使用固定 5 秒间隔 |
| 指数退避、最大 60 秒 | 未完成 | 当前重连策略不符合迁移方案 |
| 鉴权失败停止重试 | 未完成 | 错误仅显示为数值，未针对鉴权错误关闭自动重连 |
| Broker 停启后的恢复 | 未验证 | 尚无停 Broker、恢复 Broker、重新订阅和恢复收发的完整测试记录 |
| Portal MQTT 发起呼叫 | 未验证 | 需要 Portal、Mosquitto 和客户端共同联调 |
| Portal 过渡期双通道发送 | 未验证 | 当前 Qt Demo 无法单独验证 Portal 的 NIM + MQTT 双发逻辑 |
| Mosquitto ACL | 未验证 | 尚未验证只能订阅自身 inbox、可以发布到目标用户 inbox 的规则 |
| macOS/Linux 运行 | 未验证 | 已提供构建脚本，但构建脚本不等于实际运行验证 |
| PC 与移动端互通 | 未验证 | 需要 Android、iOS、鸿蒙客户端参与联调 |
| Demo 自动化验收测试 | 未完成 | 仓库现有测试主要覆盖 qmqtt 库本身，没有覆盖 `qt-demo` 业务流程 |

## 4. 已经取得的有效验证成果

### 4.1 QMQTT 基础能力可以复用

`MqttClientMgr` 已经将以下能力从 UI 中独立出来：

- 创建和销毁 MQTT 客户端；
- TCP、WS、WSS 连接；
- 用户名和密码设置；
- Keep Alive 和 Clean Session 设置；
- 自动重连；
- 订阅、取消订阅和发布；
- SUBACK、UNSUBACK、PUBACK、PINGRESP 和错误信号转发；
- WSS 自定义 CA 和自签名证书测试支持。

这一结构符合 UI 与业务逻辑解耦要求，可以继续被 GUI、CLI 或自动测试入口复用，不需要推翻重写。

### 4.2 Topic 规则已经独立封装

`MqttTopicBuilder` 集中生成：

```text
user/{imAccid}/inbox
```

这与迁移方案的 Topic 设计一致，也避免了 Topic 字符串在 UI 或业务代码中重复拼接。

### 4.3 基础连接参数符合迁移方向

当前默认的 Keep Alive 30 秒、Clean Session 和 QoS 1 与迁移方案一致。WSS、WebSocket Path、自定义 CA 等配置也能够支持后续验证 `wss://域名/message` 的真实部署链路。

## 5. 关键未完成项

### 5.1 MQTT 协议版本不符合方案

迁移方案要求所有平台统一使用 MQTT v3.1.1。当前 Demo 没有调用 `QMQTT::Client::setVersion()`，而仓库内 qmqtt 的默认版本为：

```cpp
MQTTVersion::V3_1_0
```

因此当前 Demo 实际上不能证明 MQTT v3.1.1 链路已经验证。连接配置阶段应显式设置：

```cpp
m_client->setVersion(QMQTT::MQTTVersion::V3_1_1);
```

不应依赖客户端库默认值。

### 5.2 Client ID 不符合迁移规则

迁移方案要求 Client ID 使用以下格式，并跨应用启动保持一致：

```text
{平台前缀}-{appInstanceUuid}
```

当前 Demo 使用：

```text
qt_demo_{UUID前8位}
```

并且每次启动都会重新生成。这会影响：

- Broker 端客户端实例识别；
- 同账号多端设备区分；
- `senderClientId` 自身回环过滤；
- 客户端日志排查；
- 应用重启后的身份一致性。

后续应使用 Qt 平台宏生成 `PC-Win`、`PC-Mac` 或 `PC-Linux` 前缀，并通过 `QSettings` 持久化 appInstanceUuid。

### 5.3 JSON 只作为文本传输，没有形成信令协议层

当前 Demo 发布时直接读取文本框内容，接收时将 payload 原样显示。它只能证明“MQTT 可以传输一段 JSON 文本”，不能证明“客户端可以正确处理会议呼叫信令”。

尚未覆盖：

- JSON 格式合法性检查；
- `v` 协议版本检查；
- 必填字段检查；
- `commandType` 合法值检查；
- SessionID UUID v4 生成和校验；
- `senderId`、`receiverId` 和 `senderClientId` 解析；
- JSON 到内部信令数据结构的转换；
- 非法消息和不支持版本的处理；
- CS、CSC、CSA、CSR、CSB、CSAN、CSRN、CSBN 的业务分发。

建议新增独立的非 UI 信令协议模块，例如 `MqttSignalCodec`，统一负责数据结构、JSON 序列化、解析和校验。

### 5.4 同账号多端场景尚未验证

MQTT v3.1.1 没有 MQTT 5.0 的 `noLocal` 订阅选项。设备向自身账号 inbox 发布 CSAN、CSRN 或 CSBN 后，发布设备自身也会收到消息。

迁移方案要求接收端执行：

```text
senderClientId == 本机 Client ID → 丢弃
senderClientId != 本机 Client ID → 继续处理
```

当前 Demo 没有解析 `senderClientId`，因此尚未验证这一关键设计。后续至少需要三个 Demo 实例模拟主叫方、被叫设备 D1 和同账号被叫设备 D2。

### 5.5 自动重连策略不符合方案

当前 Demo 使用 qmqtt 自动重连，重连间隔固定为 5 秒。迁移方案要求：

- 首次连接失败后继续重试；
- 使用指数退避；
- 最大重试间隔为 60 秒；
- 用户名密码错误或无权限时停止重试；
- 断线恢复后重新订阅 inbox。

当前错误回调只输出错误码，没有区分 `MqttBadUserNameOrPasswordError` 和 `MqttNotAuthorizedError`。客户端不会直接收到 Portal HTTP 接口的 403，而是收到 Mosquitto 返回的 MQTT CONNACK 错误，因此应按 qmqtt 的 `ClientError` 处理。

### 5.6 目标部署链路尚未完成端到端验证

最终链路不是公共 Broker TCP，而是：

```text
Qt Client
  → wss://portal.example.com/message:443
  → 云 SLB
  → K8s Ingress
  → Mosquitto WebSocket Listener
  → go-auth
  → Portal /mqtt/auth 与 /mqtt/acl
```

只有在该链路中完成正确凭证、错误凭证、合法订阅、越权订阅、合法发布和断线恢复测试，才能确认部署方案可用。

### 5.7 缺少可重复的场景测试证据

当前 Demo 主要依赖人工输入和日志观察。仓库中的自动测试覆盖 qmqtt 库本身，不覆盖 `qt-demo` 的迁移业务要求。

缺少的关键测试包括：

- 两个新客户端之间的完整呼叫和响应；
- 同账号两个设备的停止振铃通知；
- 错误凭证和 ACL 拒绝；
- Broker 停止和恢复；
- 网络短时中断；
- QoS 1 重复投递；
- WSS 证书错误；
- Windows、macOS、Linux 运行；
- PC 与移动端互通；
- Portal 向新客户端发起呼叫。

## 6. 迁移文档中的 QoS 1 风险

迁移方案中“新版本客户端只使用 MQTT 单通道，因此每条信令只到达一次，无需去重”的描述需要进一步明确。

MQTT QoS 1 的语义是 **至少一次送达**，不是恰好一次。在 PUBACK 丢失、连接中断或重连重发时，同一条 PUBLISH 允许被重复投递。

迁移后可以移除“NIM + MQTT 双通道副本去重”，但呼叫业务仍应具备幂等处理能力。建议使用以下业务键识别重复命令：

```text
sessionId + commandType + senderId
```

至少应保证：

- 重复 CS 不会弹出多个振铃窗口；
- 重复 CSC 不会重复结束已结束的呼叫；
- 重复 CSA/CSR/CSB 不会让状态机重复迁移；
- CSAN/CSRN/CSBN 重复到达不会造成异常副作用。

这不是恢复双通道去重，而是 MQTT QoS 1 下必要的业务幂等保护。

## 7. 建议的后续实施顺序

### 第一阶段：修正客户端基础约束

1. 显式设置 MQTT v3.1.1。
2. 实现带平台前缀且跨启动持久化的 Client ID。
3. 实现鉴权错误识别、指数退避和最大 60 秒重连间隔。
4. 明确重连成功后的订阅恢复行为。

### 第二阶段：补齐信令协议层

1. 定义统一的 MQTT 信令数据结构。
2. 实现八类 `commandType` 枚举。
3. 实现 JSON 序列化、反序列化和字段校验。
4. 每次呼叫生成 SessionID UUID v4。
5. 实现 `senderClientId` 自身回环过滤。
6. 实现基于业务键的幂等处理接口。

### 第三阶段：完成真实服务端联调

1. 使用真实 `wss://域名/message` 地址连接。
2. 验证正确和错误 `imAccid`/`imToken`。
3. 验证只能订阅自身 inbox。
4. 验证能够向目标用户 inbox 发布。
5. 验证 TLS、SLB、Ingress、Mosquitto、go-auth 和 Portal 的完整链路。

### 第四阶段：完成业务场景验收

1. 新客户端 A 向新客户端 B 发送 CS。
2. B 返回 CSA、CSR 或 CSB。
3. A 发送 CSC。
4. 同账号设备 D1/D2 验证 CSAN、CSRN 和 CSBN。
5. 验证 QoS 1 重复消息不会产生重复业务行为。
6. 停止并恢复 Broker，验证自动重连和重新订阅。
7. Portal 向新客户端发起 MQTT 呼叫。

### 第五阶段：完成跨平台验证

1. Windows、macOS、Linux 分别完成构建和实际运行。
2. PC 与 Android/iOS/鸿蒙客户端互通。
3. 移动端前后台切换及网络切换测试。
4. Portal 双通道发送覆盖新旧客户端。

## 8. 完成验收的最低标准

满足以下条件后，才建议将结论更新为“MQTT 替代 NIM 信令方案验证完成”：

- [ ] 客户端明确使用 MQTT v3.1.1；
- [ ] Client ID 符合平台规则并跨启动持久化；
- [ ] 八类信令均可正确序列化、发送、解析和分发；
- [ ] SessionID 使用 UUID v4；
- [ ] 同账号多端自身回环过滤通过测试；
- [ ] QoS 1 重复消息不会造成重复业务副作用；
- [ ] 鉴权失败不会持续重连；
- [ ] 临时断线使用指数退避并可恢复；
- [ ] Broker 停启后客户端能够恢复连接和订阅；
- [ ] 真实 WSS、Portal 鉴权和 ACL 链路通过测试；
- [ ] Portal 到新客户端呼叫通过测试；
- [ ] Windows、macOS、Linux 通过实际运行验证；
- [ ] 至少一个 PC 与移动端组合通过互通测试；
- [ ] 关键场景有可重复执行的测试步骤和结果记录。

## 9. 最终判断

当前 Demo 适合用于：

- 确认 QMQTT 库具备继续集成的基础能力；
- 验证 Qt 客户端 MQTT 连接、Topic 路由和基础收发；
- 作为后续信令协议模块和自动测试的开发基础。

当前 Demo 暂不适合用于：

- 宣布 MQTT 已经完整替代 NIM；
- 直接进入生产灰度；
- 证明 Portal、Mosquitto、go-auth、Ingress 和客户端的端到端链路已经可靠；
- 证明同账号多端、异常重连和跨平台组合已经满足项目要求。

现有架构不需要推翻。建议在 `MqttClientMgr` 和 `MqttTopicBuilder` 的基础上补充独立信令协议层、Client ID 持久化模块、重连策略和自动化场景测试，完成真实环境联调后再做最终迁移决策。
