# 即时通讯系统 (Chat System)

一个基于 Qt 框架开发的即时通讯系统，包含客户端和服务器端两个部分。支持文字聊天、图片传输、文件传输、语音消息、群组聊天等功能。

## 📋 目录

- [项目概述](#项目概述)
- [技术栈](#技术栈)
- [项目结构](#项目结构)
- [功能特性](#功能特性)
- [环境要求](#环境要求)
- [编译与运行](#编译与运行)
- [配置说明](#配置说明)
- [通信协议](#通信协议)
- [数据库设计](#数据库设计)
- [主要模块](#主要模块)
- [使用说明](#使用说明)
- [开发说明](#开发说明)

## 📖 项目概述

本项目是一个完整的即时通讯解决方案，采用 C/S 架构设计：

- **ChatClient（客户端）**：提供用户界面，处理用户交互，与服务器进行通信
- **ChatServer（服务器端）**：管理用户连接，转发消息，处理文件传输，维护数据库

### 版本信息

- 版本号：1.0.0.1
- 开发框架：Qt 5.x / Qt 6.x
- 开发语言：C++11
- 数据库：SQLite

## 🛠 技术栈

### 客户端 (ChatClient)

- **Qt 模块**：
  - `Qt Core`：核心功能
  - `Qt Gui` / `Qt Widgets`：图形界面
  - `Qt Network`：网络通信
  - `Qt SQL`：数据库操作
  - `Qt Multimedia`：多媒体支持（语音录制/播放）

- **第三方库**：
  - FMOD：音频处理库（用于语音消息）
  - 科大讯飞语音 SDK：语音识别与合成

### 服务器端 (ChatServer)

- **Qt 模块**：
  - `Qt Core`：核心功能
  - `Qt Gui` / `Qt Widgets`：管理界面
  - `Qt Network`：TCP 服务器
  - `Qt SQL`：数据库操作

- **第三方库**：
  - Qt5Xlsx：Excel 文件处理（用于数据导出）

## 📁 项目结构

```
.
├── ChatClient/                    # 客户端项目
│   ├── basewidget/               # 基础 UI 组件
│   │   ├── animationstackedwidget.*  # 动画堆叠窗口
│   │   ├── chatbubble.*          # 聊天气泡组件
│   │   ├── clineedit.*           # 自定义输入框
│   │   ├── customwidget.*        # 自定义窗口基类
│   │   ├── rotatingstackedwidget.*  # 旋转堆叠窗口
│   │   ├── pictureedit/          # 图片编辑功能
│   │   └── qqlist/               # 好友/群组列表组件
│   ├── comapi/                   # 公共 API
│   │   ├── global.h              # 全局工具函数
│   │   ├── myapp.*               # 应用程序配置管理
│   │   ├── iteminfo.*            # 消息项信息
│   │   └── qqcell.*              # 好友/群组单元格数据
│   ├── uipage/                   # UI 页面
│   │   ├── chatwindow.*          # 聊天窗口
│   │   ├── systemmessagedialog.* # 系统消息对话框
│   │   ├── systemsetting.*       # 系统设置
│   │   ├── weatherwidget.*       # 天气组件（示例）
│   │   └── widgethead.*          # 头像组件
│   ├── media/                    # 媒体处理
│   │   ├── AudioRecorder.*       # 音频录制
│   │   ├── voice.*               # 语音处理
│   │   └── fmod/                 # FMOD 音频库
│   ├── pictureedit/              # 图片编辑
│   ├── resource/                 # 资源文件
│   │   ├── background/           # 背景图片
│   │   ├── common/               # 通用图标
│   │   ├── head/                 # 默认头像
│   │   ├── images/               # 图片资源
│   │   ├── qss/                  # 样式表
│   │   └── sound/                # 音效文件
│   ├── clientsocket.*           # TCP 客户端套接字
│   ├── databasemagr.*            # 数据库管理
│   ├── loginwidget.*             # 登录界面
│   ├── mainwindow.*              # 主窗口
│   ├── main.cpp                  # 程序入口
│   └── ChatClient.pro            # 项目配置文件
│
└── ChatServer/                   # 服务器端项目
    ├── basewidget/               # 基础 UI 组件
    ├── resource/                 # 资源文件
    ├── clientsocket.*           # 客户端连接管理
    ├── databasemagr.*           # 数据库管理
    ├── tcpserver.*              # TCP 服务器
    ├── mainwindow.*             # 服务器管理界面
    ├── main.cpp                 # 程序入口
    └── ChatServer.pro            # 项目配置文件
```

## ✨ 功能特性

### 客户端功能

#### 1. 用户认证
- ✅ 用户登录
- ✅ 用户注册
- ✅ 自动登录（可选）
- ✅ 记住密码
- ✅ 服务器配置（IP、端口）

#### 2. 好友管理
- ✅ 添加好友
- ✅ 删除好友
- ✅ 好友列表显示
- ✅ 好友在线状态显示
- ✅ 好友头像显示与更新
- ✅ 刷新好友状态

#### 3. 群组功能
- ✅ 创建群组
- ✅ 加入群组
- ✅ 退出群组
- ✅ 群组成员管理
- ✅ 群组聊天

#### 4. 聊天功能
- ✅ 一对一文字聊天
- ✅ 群组文字聊天
- ✅ 图片发送与接收
- ✅ 文件传输（带进度显示）
- ✅ 语音消息录制与发送
- ✅ 表情发送
- ✅ 聊天历史记录
- ✅ 消息时间显示
- ✅ 聊天气泡样式

#### 5. 文件传输
- ✅ 文件选择与发送
- ✅ 文件接收与保存
- ✅ 传输进度显示
- ✅ 传输速度显示
- ✅ 文件下载管理

#### 6. 其他功能
- ✅ 系统托盘
- ✅ 消息通知
- ✅ 自定义头像（支持裁剪）
- ✅ 系统设置
- ✅ 数据备份与恢复
- ✅ 窗口动画效果
- ✅ 自定义主题样式

### 服务器端功能

#### 1. 服务器管理
- ✅ TCP 消息服务器（端口：5363）
- ✅ TCP 文件服务器（端口：7777）
- ✅ 服务器启动/停止
- ✅ 连接状态监控

#### 2. 用户管理
- ✅ 用户注册验证
- ✅ 用户登录验证
- ✅ 用户状态管理（在线/离线）
- ✅ 用户信息查询
- ✅ 用户列表显示
- ✅ 用户身份管理（管理员/经理/员工）

#### 3. 消息转发
- ✅ 私聊消息转发
- ✅ 群组消息广播
- ✅ 系统消息推送
- ✅ 消息类型识别与处理

#### 4. 文件服务
- ✅ 文件上传处理
- ✅ 文件下载服务
- ✅ 文件存储管理

#### 5. 数据管理
- ✅ 数据库备份
- ✅ 数据恢复
- ✅ 数据导出（Excel）
- ✅ 用户数据管理

## 🔧 环境要求

### 开发环境

- **操作系统**：Windows 7/8/10/11, Linux, macOS
- **Qt 版本**：Qt 5.12+ 或 Qt 6.x
- **编译器**：
  - Windows: MinGW 或 MSVC
  - Linux: GCC 5.4+
  - macOS: Clang
- **C++ 标准**：C++11 或更高

### 运行时依赖

#### 客户端
- Qt 运行时库
- FMOD 动态库（`fmodex.dll` / `libfmodex.so`）
- 科大讯飞语音 SDK（`msc.dll` / `libmsc.so`）

#### 服务器端
- Qt 运行时库
- Qt5Xlsx 库（可选，用于 Excel 导出）

## 🚀 编译与运行

### 编译客户端

```bash
# 进入客户端目录
cd ChatClient

# 使用 qmake 生成 Makefile
qmake ChatClient.pro

# 编译（Windows MinGW）
mingw32-make

# 或使用 MSVC
nmake

# Linux/macOS
make
```

编译后的可执行文件位于：`../release/Client/ChatClient.exe`（Windows）

### 编译服务器端

```bash
# 进入服务器端目录
cd ChatServer

# 使用 qmake 生成 Makefile
qmake ChatServer.pro

# 编译
mingw32-make  # Windows MinGW
# 或
make          # Linux/macOS
```

编译后的可执行文件位于：`../release/Server/ChatServer.exe`（Windows）

### 运行

1. **启动服务器**：
   ```bash
   # 运行服务器程序
   ./ChatServer
   ```
   默认监听端口：
   - 消息服务器：5363
   - 文件服务器：7777

2. **启动客户端**：
   ```bash
   # 运行客户端程序
   ./ChatClient
   ```
   首次运行需要配置服务器地址和端口。

## ⚙️ 配置说明

### 客户端配置

配置文件位置：`{应用目录}/config/config.ini`

主要配置项：
```ini
[Server]
HostAddr=127.0.0.1        # 服务器地址
MsgPort=5363              # 消息服务器端口
FilePort=7777             # 文件服务器端口

[User]
UserName=                 # 用户名
Password=                 # 密码（加密存储）
AutoLogin=false           # 是否自动登录
RememberPasswd=false      # 是否记住密码
```

### 服务器端配置

配置文件位置：`{应用目录}/config/config.ini`

主要配置项：
```ini
[Server]
MsgPort=5363              # 消息服务器端口
FilePort=7777             # 文件服务器端口

[Database]
Path=./data/info.db       # 数据库路径
```

### 数据目录结构

客户端数据目录：
```
{应用目录}/
├── data/                 # 数据目录
│   ├── user.db          # 用户数据库
│   └── msg.db           # 消息数据库
├── recv/                # 接收文件目录
├── head/                # 头像目录
├── face/                # 表情目录
├── record/              # 录音文件目录
└── config/              # 配置目录
```

服务器端数据目录：
```
{应用目录}/
├── data/
│   └── info.db          # 服务器数据库
├── recv/                # 文件存储目录
├── head/                # 用户头像存储
└── config/              # 配置目录
```

## 📡 通信协议

### 端口说明

- **MSG_PORT_ONLINE** (6353)：在线状态通知端口（已弃用）
- **MSG_PORT_MESSAGE** (5363)：消息通信端口
- **TCP_FILE_PORT** (7777)：文件传输端口

### 消息类型 (E_MSG_TYPE)

#### 用户认证 (0x10-0x14)
- `Register (0x10)`：用户注册
- `Login (0x11)`：用户登录
- `Logout (0x12)`：用户注销
- `LoginRepeat (0x13)`：重复登录

#### 用户状态 (0x15-0x17)
- `UserOnLine (0x15)`：用户上线通知
- `UserOffLine (0x16)`：用户下线通知
- `UpdateHeadPic (0x17)`：更新头像

#### 好友/群组管理 (0x20-0x29)
- `AddFriend (0x20)`：添加好友
- `AddGroup (0x21)`：添加群组
- `AddFriendRequist (0x22)`：添加好友请求
- `AddGroupRequist (0x23)`：添加群组请求
- `CreateGroup (0x25)`：创建群组

#### 信息获取 (0x30-0x39)
- `GetMyFriends (0x30)`：获取好友列表
- `GetMyGroups (0x31)`：获取群组列表
- `RefreshFriends (0x35)`：刷新好友状态
- `RefreshGroups (0x36)`：刷新群组状态

#### 消息发送 (0x40-0x49)
- `SendMsg (0x40)`：发送私聊消息
- `SendGroupMsg (0x41)`：发送群组消息
- `SendFile (0x42)`：发送文件
- `SendPicture (0x43)`：发送图片
- `SendFace (0x44)`：发送表情

#### 其他操作 (0x50-0x65)
- `ChangePasswd (0x50)`：修改密码
- `DeleteFriend (0x55)`：删除好友
- `DeleteGroup (0x56)`：退出群组
- `SendFileOk (0x60)`：文件发送完成
- `GetFile (0x65)`：下载文件
- `GetPicture (0x66)`：下载图片

### 消息格式

消息采用 JSON 格式传输：

```json
{
  "type": 0x40,           // 消息类型
  "from": 1001,           // 发送者 ID
  "to": 1002,            // 接收者 ID（群组时为群组 ID）
  "data": {              // 消息数据
    "text": "消息内容",
    "time": "2024-01-01 12:00:00"
  }
}
```

### 状态码 (E_STATUS)

- `ConnectedHost (0x01)`：已连接服务器
- `DisConnectedHost (0x02)`：断开连接
- `LoginSuccess (0x03)`：登录成功
- `LoginPasswdError (0x04)`：密码错误
- `OnLine (0x05)`：在线
- `OffLine (0x06)`：离线
- `RegisterOk (0x07)`：注册成功
- `RegisterFailed (0x08)`：注册失败
- `AddFriendOk (0x09)`：添加好友成功
- `AddFriendFailed (0x0A)`：添加好友失败

## 🗄️ 数据库设计

### 客户端数据库

#### user.db（用户数据库）

**friends 表**：好友信息
```sql
CREATE TABLE friends (
    id INTEGER PRIMARY KEY,
    userId INTEGER,      -- 用户 ID
    friendId INTEGER,    -- 好友 ID
    friendName TEXT,     -- 好友名称
    headFile TEXT        -- 头像文件
);
```

**groups 表**：群组信息
```sql
CREATE TABLE groups (
    id INTEGER PRIMARY KEY,
    userId INTEGER,      -- 用户 ID
    groupId INTEGER,      -- 群组 ID
    groupName TEXT,       -- 群组名称
    headFile TEXT         -- 群组头像
);
```

#### msg.db（消息数据库）

**messages 表**：聊天记录
```sql
CREATE TABLE messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    userId INTEGER,       -- 用户 ID
    friendId INTEGER,    -- 好友/群组 ID
    type INTEGER,        -- 消息类型（Text/Audio/Picture/Files）
    direction INTEGER,   -- 方向（0:接收, 1:发送）
    content TEXT,        -- 消息内容
    time TEXT,           -- 时间戳
    filePath TEXT        -- 文件路径（如果是文件消息）
);
```

### 服务器端数据库

#### info.db（服务器数据库）

**users 表**：用户信息
```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE,     -- 用户名
    password TEXT,        -- 密码（加密）
    head TEXT,           -- 头像文件
    status INTEGER,      -- 状态（0:离线, 1:在线）
    identity INTEGER,    -- 身份（0x01:管理员, 0x02:经理, 0x03:员工）
    registerTime TEXT    -- 注册时间
);
```

**friends 表**：好友关系
```sql
CREATE TABLE friends (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    userId INTEGER,      -- 用户 ID
    friendId INTEGER,    -- 好友 ID
    UNIQUE(userId, friendId)
);
```

**groups 表**：群组信息
```sql
CREATE TABLE groups (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,           -- 群组名称
    creatorId INTEGER,   -- 创建者 ID
    createTime TEXT      -- 创建时间
);
```

**group_members 表**：群组成员
```sql
CREATE TABLE group_members (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    groupId INTEGER,     -- 群组 ID
    userId INTEGER,      -- 用户 ID
    UNIQUE(groupId, userId)
);
```

## 🧩 主要模块

### 客户端核心模块

#### 1. ClientSocket（网络通信）
- 负责与服务器建立 TCP 连接
- 处理消息的发送与接收
- 管理连接状态
- 消息序列化与反序列化

#### 2. ClientFileSocket（文件传输）
- 独立的文件传输通道
- 支持文件上传与下载
- 传输进度监控
- 断点续传支持（可扩展）

#### 3. DataBaseMagr（数据库管理）
- 单例模式管理数据库连接
- 好友信息存储与查询
- 聊天记录存储与查询
- 群组信息管理

#### 4. ChatWindow（聊天窗口）
- 聊天界面显示
- 消息发送与接收
- 文件传输界面
- 历史记录加载

#### 5. MainWindow（主窗口）
- 好友列表管理
- 群组列表管理
- 聊天窗口管理
- 系统菜单与托盘

#### 6. LoginWidget（登录界面）
- 用户登录
- 服务器配置
- 自动登录处理

### 服务器端核心模块

#### 1. TcpMsgServer（消息服务器）
- TCP 服务器监听
- 客户端连接管理
- 消息接收与转发
- 用户状态管理

#### 2. TcpFileServer（文件服务器）
- 文件传输服务器
- 文件上传处理
- 文件下载服务
- 文件存储管理

#### 3. ClientSocket（客户端连接）
- 单个客户端连接管理
- 消息解析与处理
- 消息转发
- 连接状态维护

#### 4. DataBaseMagr（数据库管理）
- 用户信息管理
- 好友关系管理
- 群组管理
- 数据查询与更新

#### 5. MainWindow（管理界面）
- 服务器控制
- 用户列表显示
- 连接状态监控
- 数据管理功能

## 📖 使用说明

### 服务器端使用

1. **启动服务器**：
   - 运行 `ChatServer.exe`
   - 点击"启动服务器"按钮
   - 查看状态栏确认服务器已启动

2. **用户管理**：
   - 在"用户管理"页面查看所有用户
   - 可以手动添加用户（需要数据库操作）
   - 查看用户在线状态

3. **数据管理**：
   - 使用"数据备份"功能备份数据库
   - 使用"数据恢复"功能恢复数据库
   - 使用"数据导出"功能导出为 Excel

### 客户端使用

1. **首次使用**：
   - 运行 `ChatClient.exe`
   - 在登录界面配置服务器地址和端口
   - 点击"注册"创建新账号
   - 输入用户名和密码登录

2. **添加好友**：
   - 在主界面右键点击好友列表区域
   - 选择"添加好友"
   - 输入好友用户名
   - 等待对方确认

3. **发送消息**：
   - 双击好友打开聊天窗口
   - 在输入框输入消息
   - 点击"发送"或按 Enter 键
   - 支持发送图片、文件、语音

4. **文件传输**：
   - 在聊天窗口点击"发送文件"按钮
   - 选择要发送的文件
   - 等待对方接收
   - 接收文件会自动保存到 `recv/` 目录

5. **群组聊天**：
   - 右键点击群组列表
   - 选择"创建群组"或"加入群组"
   - 在群组中发送消息，所有成员可见

## 💻 开发说明

### 代码规范

- 使用 C++11 标准
- 类名使用大驼峰命名（PascalCase）
- 函数和变量使用小驼峰命名（camelCase）
- 私有成员变量使用 `m_` 前缀
- 使用 Qt 的信号槽机制进行通信

### 扩展开发

#### 添加新的消息类型

1. 在 `unit.h` 中的 `E_MSG_TYPE` 枚举添加新类型
2. 在 `ClientSocket` 中添加消息发送函数
3. 在服务器端 `ClientSocket` 中添加消息处理
4. 在客户端 `MainWindow` 中添加消息解析

#### 添加新的 UI 组件

1. 在 `basewidget/` 目录创建新组件
2. 继承 `CustomWidget` 或 `QWidget`
3. 在 `.pri` 文件中注册组件
4. 在需要的地方包含头文件使用

### 调试技巧

- 使用 `qDebug()` 输出调试信息
- 检查网络连接状态
- 查看数据库文件内容
- 使用 Qt Creator 的调试器

### 常见问题

1. **连接服务器失败**：
   - 检查服务器是否启动
   - 检查防火墙设置
   - 确认服务器地址和端口正确

2. **文件传输失败**：
   - 检查文件服务器是否启动
   - 确认文件大小未超过限制
   - 检查磁盘空间

3. **数据库错误**：
   - 检查数据库文件权限
   - 确认数据库文件未损坏
   - 查看数据库连接状态

## 📝 许可证

本项目仅供学习和研究使用。

## 👥 贡献

欢迎提交 Issue 和 Pull Request。

## 📧 联系方式

如有问题或建议，请通过 Issue 反馈。

---

**注意**：本项目为学习项目，生产环境使用请自行评估安全性和稳定性。

