# QtTest 使用说明

## 1. 文档目的

QtTest 是 Qt 自带的单元测试框架，用于直接验证 C++/Qt 模块的输入、输出、状态和信号，不需要启动完整 GUI。

本目录中的测试主要用于：

- 验证 `qt-demo` 非 UI 模块的行为；
- 在修改代码后快速发现功能回归；
- 为后续 Topic 校验、重连策略和 MQTT 异步流程提供可重复的验证入口；
- 让本地构建和持续集成使用同一套测试。

当前测试不连接公网 Broker，也不会启动 `MqttClientWgt` 窗口。

## 2. 当前目录结构

```text
tests/
├── tests.pro
├── qttest-guide.md
├── mqtttopicbuilder/
│   ├── mqtttopicbuilder.pro
│   └── tst_mqtttopicbuilder.cpp
├── mqttproxymanager/
│   ├── mqttproxymanager.pro
│   └── tst_mqttproxymanager.cpp
├── mqttreconnectpolicy/
│   ├── mqttreconnectpolicy.pro
│   └── tst_mqttreconnectpolicy.cpp
└── mqttclientmgr/
    ├── mqttclientmgr.pro
    └── tst_mqttclientmgr.cpp
```

顶层 `tests.pro` 是 QMake `SUBDIRS` 工程，一次构建所有测试子工程：

```qmake
TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += mqtttopicbuilder \
           mqttproxymanager \
           mqttreconnectpolicy \
           mqttclientmgr
```

每个子目录生成一个独立的测试程序：

```text
tst_mqtttopicbuilder.exe
tst_mqttproxymanager.exe
tst_mqttreconnectpolicy.exe
tst_mqttclientmgr.exe
```

独立测试程序便于单独运行、定位失败模块，也方便后续继续增加测试目录。

## 3. QtTest 的运行机制

一个基本的 QtTest 测试类继承 `QObject`，使用 `Q_OBJECT` 启用 Qt 元对象系统，并把测试函数声明为私有槽函数：

```cpp
class MqttTopicBuilderTest : public QObject
{
    Q_OBJECT

private slots:
    void slot_emptyAccountReturnsEmpty();
};
```

文件末尾使用 QtTest 宏生成测试程序入口：

```cpp
QTEST_GUILESS_MAIN(MqttTopicBuilderTest)

#include "tst_mqtttopicbuilder.moc"
```

运行过程如下：

1. `moc` 读取 `Q_OBJECT` 和槽函数信息；
2. `QTEST_GUILESS_MAIN` 创建 `QCoreApplication`；
3. QtTest 通过元对象系统查找测试槽函数；
4. QtTest 逐个调用测试函数；
5. 断言成功时输出 `PASS`，失败时输出 `FAIL`；
6. 全部通过时进程返回 `0`，存在失败时返回非 `0`。

因此 `nmake check`、`make check` 和持续集成都可以根据退出码判断测试是否成功。

## 4. 测试工程配置

测试子工程至少需要以下配置：

```qmake
QT += core testlib
CONFIG += c++11 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mqtttopicbuilder
```

关键配置说明：

| 配置 | 作用 |
|---|---|
| `QT += testlib` | 链接 QtTest 模块 |
| `CONFIG += testcase` | 生成 `check` 测试目标 |
| `CONFIG += console` | Windows 下保留控制台输出 |
| `CONFIG += c++11` | 保持 Demo 的 C++11 兼容约束 |
| `CONFIG -= app_bundle` | macOS 下不把测试程序生成 App Bundle |

当前测试子工程直接编译被测模块的 `.cpp` 文件，例如：

```qmake
SOURCES += tst_mqtttopicbuilder.cpp \
           $$PWD/../../mqtttopicbuilder.cpp

HEADERS += $$PWD/../../mqtttopicbuilder.h
```

这样测试调用的是 Demo 中真实的业务实现，不会复制一份测试专用逻辑。

## 5. 普通测试和断言

下面的测试验证空账号不生成 Topic：

```cpp
void MqttTopicBuilderTest::slot_emptyAccountReturnsEmpty()
{
    QCOMPARE(MqttTopicBuilder::inboxTopic(QString("")), QString(""));
    QCOMPARE(MqttTopicBuilder::inboxTopic(QString("   ")), QString(""));
}
```

常用断言如下：

| 断言 | 用途 |
|---|---|
| `QVERIFY(condition)` | 要求条件为 `true` |
| `QVERIFY(!condition)` | 要求条件为 `false` |
| `QVERIFY2(condition, message)` | 条件失败时附带原因 |
| `QCOMPARE(actual, expected)` | 比较实际值和期望值 |
| `QFAIL(message)` | 主动标记当前测试失败 |
| `QSKIP(message)` | 当前环境不满足条件时跳过测试 |

断言失败后，QtTest 会输出测试名称、实际值、期望值、源文件和行号，然后继续执行其他测试用例。

## 6. 数据驱动测试

同一段逻辑需要验证多组输入时，优先使用数据驱动测试。

### 6.1 准备测试数据

数据函数必须使用“测试函数名 + `_data`”的固定命名：

```cpp
void MqttTopicBuilderTest::slot_buildsInboxTopic_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("expectedTopic");

    QTest::newRow("plain account")
            << QString("alice")
            << QString("user/alice/inbox");

    QTest::newRow("trimmed account")
            << QString("  alice-01  ")
            << QString("user/alice-01/inbox");
}
```

`addColumn` 定义列名和类型，`newRow` 定义一组测试数据。每个 Row 名称会显示在测试结果中，应该表达具体场景。

### 6.2 读取并验证数据

```cpp
void MqttTopicBuilderTest::slot_buildsInboxTopic()
{
    QFETCH(QString, accountId);
    QFETCH(QString, expectedTopic);

    QCOMPARE(MqttTopicBuilder::inboxTopic(accountId), expectedTopic);
}
```

QtTest 会为每个 `newRow` 分别调用一次 `slot_buildsInboxTopic()`：

```text
PASS : MqttTopicBuilderTest::slot_buildsInboxTopic(plain account)
PASS : MqttTopicBuilderTest::slot_buildsInboxTopic(trimmed account)
```

`QFETCH` 的类型和名称必须与 `addColumn` 完全一致。

## 7. 测试生命周期

QtTest 支持以下固定生命周期槽函数：

| 函数 | 调用时机 |
|---|---|
| `initTestCase()` | 整个测试类开始前调用一次 |
| `cleanupTestCase()` | 整个测试类结束后调用一次 |
| `init()` | 每个测试用例开始前调用 |
| `cleanup()` | 每个测试用例结束后调用 |

这些名称是 QtTest 框架规定的特殊入口。普通测试槽仍按本项目规范使用 `slot_` 前缀。

适合放入生命周期函数的内容包括：

- 创建和销毁共享测试对象；
- 为每个用例恢复默认配置；
- 清理临时文件；
- 复位全局代理或环境状态。

测试之间不应依赖执行顺序。每个测试都应能独立运行。

## 8. 信号和异步行为测试

MQTT 连接、订阅确认和消息接收都是异步行为，后续测试会使用 `QSignalSpy`：

```cpp
QSignalSpy connectedSpy(mqttClientMgr, &MqttClientMgr::sig_connected);
QVERIFY(connectedSpy.isValid());

mqttClientMgr->connectToHost(params);

QTRY_COMPARE(connectedSpy.count(), 1);
```

常用异步断言：

| 断言 | 用途 |
|---|---|
| `QTRY_VERIFY(condition)` | 等待条件在超时前变为 `true` |
| `QTRY_COMPARE(actual, expected)` | 等待实际值达到期望值 |
| `QSignalSpy::count()` | 统计信号发射次数 |
| `QSignalSpy::takeFirst()` | 读取首次信号参数 |

读取信号参数示例：

```cpp
QList<QVariant> arguments = errorSpy.takeFirst();
QCOMPARE(arguments.at(0).toInt(), expectedErrorCode);
```

异步测试不应使用固定时长的 `QThread::sleep()`。应等待明确的信号或状态，并设置合理超时。

## 9. GUI 事件测试

需要测试 QWidget 交互时，可以使用：

```cpp
QTest::mouseClick(connectButton, Qt::LeftButton);
QTest::keyClicks(hostLineEdit, QString("broker.example.com"));
QCOMPARE(statusLabel->text(), QString("Connecting"));
```

GUI 测试入口使用：

```cpp
QTEST_MAIN(MqttClientWidgetTest)
```

本 Demo 优先测试非 UI 业务模块。只有界面交互本身无法通过业务层测试覆盖时，才增加 QWidget 测试。

## 10. Windows 构建和运行

以下命令使用当前 Demo 配置的 Qt 5.15.2 和 Visual Studio 2017 x86 工具链：

```bat
cd /d D:\Code\GitHub\qmqtt\qt-demo

mkdir build\tests-windows
cd build\tests-windows

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x86
set "PATH=C:\Qt\5.15.2\msvc2019\bin;%PATH%"

"C:\Qt\5.15.2\msvc2019\bin\qmake.exe" ..\..\tests\tests.pro
nmake
nmake check
```

命令作用：

| 命令 | 作用 |
|---|---|
| `qmake` | 为顶层和子测试工程生成 Makefile |
| `nmake` | 编译全部测试程序 |
| `nmake check` | 依次运行全部测试程序 |

单独运行一个测试程序：

```bat
mqtttopicbuilder\release\tst_mqtttopicbuilder.exe
mqttproxymanager\release\tst_mqttproxymanager.exe
```

只运行一个测试函数：

```bat
mqtttopicbuilder\release\tst_mqtttopicbuilder.exe slot_emptyAccountReturnsEmpty
```

列出测试程序中的函数：

```bat
mqtttopicbuilder\release\tst_mqtttopicbuilder.exe -functions
```

将结果写入文本文件：

```bat
mqtttopicbuilder\release\tst_mqtttopicbuilder.exe -o testresults.txt,txt
```

## 11. Linux 和 macOS 构建

使用与 Demo 匹配的 Qt 环境执行：

```bash
cd qt-demo
mkdir -p build/tests
cd build/tests

qmake ../../tests/tests.pro
make -j4
make check
```

如果系统同时安装多个 Qt 版本，应使用所选 Qt 安装目录中的完整 `qmake` 路径。

## 12. Qt Creator 中运行

1. 使用 Qt Creator 打开 `qt-demo/tests/tests.pro`；
2. 选择与 Demo 一致的 Qt Kit；
3. 构建全部子项目；
4. 打开 Qt Creator 的“Tests/测试”面板；
5. 选择单个用例、单个测试类或全部测试运行；
6. 双击失败项跳转到对应代码行。

如果 Qt Creator 没有识别测试：

- 确认测试类包含 `Q_OBJECT`；
- 确认测试函数位于 `private slots`；
- 确认 `.pro` 包含 `QT += testlib` 和 `CONFIG += testcase`；
- 重新运行 QMake 后再构建。

## 13. 测试结果说明

成功输出示例：

```text
PASS   : MqttTopicBuilderTest::slot_emptyAccountReturnsEmpty()
PASS   : MqttTopicBuilderTest::slot_buildsInboxTopic(plain account)
Totals: 6 passed, 0 failed, 0 skipped, 0 blacklisted
```

QtTest 会自动显示 `initTestCase()` 和 `cleanupTestCase()` 结果，所以统计数量可能比项目中显式编写的测试函数多两个。

失败输出示例：

```text
FAIL!  : MqttTopicBuilderTest::slot_buildsInboxTopic(invalid account)
Actual   (actualTopic): "user/+/inbox"
Expected (expectedTopic): ""
```

失败时应先判断：

1. 被测实现确实存在缺陷；
2. 测试期望是否符合当前接口约定；
3. 测试之间是否共享了未复位的全局状态；
4. 异步测试是否等待了正确的信号，而不是依赖固定延时。

不能仅为了让测试变绿而修改正确的业务行为或降低断言要求。

## 14. 新增测试模块

以新增 `MqttReconnectPolicy` 测试为例：

1. 新建全小写目录 `tests/mqttreconnectpolicy/`；
2. 新建 `mqttreconnectpolicy.pro`；
3. 新建 `tst_mqttreconnectpolicy.cpp`；
4. 测试类命名为 `MqttReconnectPolicyTest`；
5. 普通测试槽使用 `slot_` 前缀；
6. 在顶层 `tests.pro` 的 `SUBDIRS` 中添加 `mqttreconnectpolicy`；
7. 重新运行 QMake；
8. 执行 `nmake` 和 `nmake check`。

每个测试用例应遵循 Arrange、Act、Assert 三个阶段：

```text
Arrange：准备对象和输入
Act：调用被测接口
Assert：验证返回值、状态或信号
```

## 15. 当前测试覆盖范围

### MqttTopicBuilder

- 空账号返回空 Topic；
- 纯空白账号返回空 Topic；
- 普通账号生成 inbox Topic；
- 自动清理账号两端空格；
- Unicode 账号生成 inbox Topic；
- 拒绝 Topic 层级分隔符 `/`；
- 拒绝 MQTT 通配符 `+` 和 `#`；
- 拒绝嵌入空字符。

### MqttProxyManager

- 禁用全局代理；
- 拒绝空 Host；
- 拒绝空白 Host；
- 拒绝零端口；
- 拒绝未知代理类型；
- 无效参数不会覆盖原代理；
- 应用 HTTP 代理；
- 应用 SOCKS5 代理；
- 应用代理用户名和密码。

### MqttReconnectPolicy

- 初始重连等待时间为 5 秒；
- 等待时间按 `5 → 10 → 20 → 40 → 60` 秒指数增长；
- 达到 60 秒后保持封顶值；
- 连接成功后可重置为 5 秒。

### MqttClientMgr

- 未连接时拒绝订阅、取消订阅和发布；
- 已连接时拒绝空 Topic；
- 已连接时拒绝超出 `0～2` 的 QoS；
- 已连接时接受合法订阅、取消订阅和发布请求；
- 允许 MQTT 标准支持的空 Payload；
- 临时断线后安排自动重连；
- 主动断开后不安排自动重连；
- 鉴权失败后停止自动重连；
- 首次连接后发送自动订阅并处理 SUBACK；
- 断线重连后只恢复一次已登记订阅；
- 取消订阅后不再恢复该 Topic；
- 使用本地 `QTcpServer` 模拟 Broker，不依赖公网服务。

## 16. 常见问题

### 找不到 Qt5Test.dll 或 Qt5Core.dll

将当前 Qt 的 `bin` 目录加入 `PATH`：

```bat
set "PATH=C:\Qt\5.15.2\msvc2019\bin;%PATH%"
```

### 找不到 nmake

先调用 Visual Studio 开发环境脚本：

```bat
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x86
```

### 出现 undefined reference to vtable 或 unresolved external symbol

检查：

- 测试类是否包含 `Q_OBJECT`；
- `.cpp` 末尾是否包含对应 `.moc` 文件；
- 修改工程后是否重新运行了 QMake。

### 新测试没有被执行

检查测试函数是否声明在 `private slots` 中，以及顶层 `tests.pro` 是否添加了对应子目录。

### 测试在本机成功但在 CI 失败

优先检查是否依赖：

- 本机固定路径；
- 公网 Broker；
- 执行顺序；
- 未清理的配置或临时文件；
- 不确定的固定延时；
- 本机已有的 Qt 插件或环境变量。

测试应尽量使用 Qt 跨平台 API、本地临时资源和可控的假服务。

## 17. GitHub Actions 持续集成

仓库通过 `.github/workflows/qt-demo.yml` 自动验证 Demo，触发条件包括：

- 修改 `qt-demo`、QMQTT 源码或对应 QMake 工程文件后推送代码；
- 创建或更新 Pull Request；
- 在 GitHub Actions 页面手动触发。

工作流分别在以下环境完成原生构建和测试：

| 平台 | 目标架构 | 编译器 | Qt 来源 |
|---|---|---|---|
| Windows 2022 | x64 | MSVC | Qt 5.15.2 `win64_msvc2019_64` |
| Windows 2022 | x86 | MSVC | Qt 5.15.2 `win32_msvc2019` |
| Ubuntu 22.04 | x64 | GCC | Qt 5.15.2 `gcc_64` |
| Ubuntu 22.04 | ARM64 | GCC | Ubuntu Qt 5.15.3 原生包 |
| macOS 15 Intel | x64 | Clang | Homebrew `qt@5` 5.15.19 原生包 |
| macOS 15 Apple Silicon | ARM64 | Clang | Homebrew `qt@5` 5.15.19 原生包 |

Windows x86 任务运行在 x64 Runner 上，但通过 MSVC x86 工具链和 Qt 32 位库生成并运行真正的 32 位程序。Ubuntu ARM64 和 macOS ARM64 任务直接运行在 GitHub ARM64 Runner 上。

Qt 5.15.2 官方桌面仓库没有 Linux ARM64 和 macOS ARM64 预编译包。为避免耗时且不稳定的 Qt 源码编译，两个 ARM64 任务使用系统提供的原生 Qt 5 包；它们仍满足项目 Qt 5.11.3 及以上的兼容要求。

macOS Intel 和 Apple Silicon 统一使用 Homebrew `qt@5` 5.15.19，减少同一操作系统不同架构之间由 Qt 补丁版本造成的行为差异。工作流会检查 `qmake` 报告的实际版本，不符合矩阵要求时直接失败。

GitHub 当前不提供麒麟或 UOS 的 Hosted Runner。Ubuntu 构建可以验证大部分通用 Linux 源码、QMake 工程和 Qt API 兼容性，但不能证明 Ubuntu 生成的二进制可直接在麒麟或 UOS 上交付。涉及发布时，还需要验证目标系统的 glibc、libstdc++、OpenSSL、Qt 插件、X11/Wayland、桌面集成和安装包格式。

麒麟和 UOS 可以尝试分别部署 GitHub Actions 自托管 Runner，并添加 `kylin`、`uos`、`x64` 或 `ARM64` 标签。GitHub 官方支持列表没有单独列出这两个发行版，因此接入前还需在目标系统验证 Runner 服务本身。没有对应 Runner 时不应把任务直接加入当前矩阵，否则任务会一直排队。只需要编译检查时也可以使用目标系统容器；涉及 QWidget 启动、系统代理、证书、输入法和桌面交互时，应使用真实系统或虚拟机 Runner。

每个平台依次执行：

1. 安装 Qt Core、Network、Widgets、TestLib 和 WebSockets 等工程依赖；
2. 使用 `qt-demo/mqtt-client.pro` 构建 Demo；
3. 使用 `qt-demo/tests/tests.pro` 构建全部 QtTest 程序；
4. 执行 `nmake check` 或 `make check`。

CI 不调用本地构建脚本，因此不会覆盖脚本中显式配置的 Qt 或 Visual Studio 路径。CI 使用 GitHub Runner 临时安装的 Qt，本地脚本仍遵循“配置路径非空时严格使用指定版本，留空时扫描并让用户选择”的规则。每个任务会在日志中输出目标架构、Runner 架构、Qt 版本和安装路径，并检查生成的 Demo 二进制架构；架构不匹配时任务会直接失败。

测试使用本地 `QTcpServer` 模拟 Broker，不需要公网 MQTT 服务、账号密码或仓库 Secret。CI 通过只能证明 Demo 可以跨平台编译且自动化用例通过；真实 Broker 的 TLS、代理、弱网和跨网络连通性仍应在集成环境中验证。
