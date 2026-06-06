# UI 重构说明（2026-06-06）

## 背景

原来所有界面代码都堆在 `order_system.cpp` 一个文件里（登录页、注册页、菜单页、导航栏、业务逻辑混在一起，超过 700 行）。随着功能增加，这个文件越来越难维护，合并时也容易产生冲突。本次重构把它拆成职责单一的独立组件。

## 新的文件结构

```
ui/src/
├── main.cpp                  # 不变
├── order_system.h/cpp        # 主窗口（精简后约 170 行，只做组装和业务逻辑）
├── nav_bar.h/cpp             # 顶部导航栏组件（新增）
├── login_page.h/cpp          # 登录页组件（新增）
├── register_page.h/cpp       # 注册页组件（新增）
├── menu_page.h/cpp           # 菜单页组件（新增）
├── cart_page.h/cpp           # 购物车页占位（新增）
├── queue_page.h/cpp          # 排队页占位（新增）
├── dish_card.h/cpp           # 菜品卡片（不变）
└── comment_dialog.h/cpp      # 评论弹窗（不变）
```

CMakeLists.txt **不需要改**，`aux_source_directory(./src srcs)` 会自动收录新增的 `.cpp`。

## 各组件职责

### NavBar（nav_bar.h/cpp）
- 固定高度 54px 的顶栏
- 左侧：用户头像（彩色圆圈）+ 用户名 + 会员等级，点击弹出下拉菜单
- 下拉菜单目前只有「退出登录」一项
- 中间：菜品菜单 / 购物车 / 排队进度三个导航按钮，当前页对应按钮高亮
- 右侧：当前展示的菜品数量
- 对外暴露的接口：`setUser(User)`、`setDishCount(int)`、`setActiveNav(int)`
- 对外发出的信号：`navClicked(int pageIndex)`、`logoutRequested()`

### LoginPage（login_page.h/cpp）
- 只负责渲染登录表单
- 点击登录按钮后发射 `loginClicked(name, password)` 信号，由主窗口验证
- 点击注册入口发射 `toRegisterClicked()` 信号

### RegisterPage（register_page.h/cpp）
- 只负责渲染注册表单
- 两次密码是否一致的校验在本组件内完成
- 校验通过后发射 `registerClicked(name, password)` 信号

### MenuPage（menu_page.h/cpp）
- 左侧分类列表 + 右侧推荐栏 + 菜品卡片滚动区，完全自包含
- 评论弹窗（CommentDialog）由本组件直接打开，不再冒泡到主窗口
- 对外暴露的接口：`setData(...)` 传入菜品数据，`setDiscountRate(double)` 传入折扣率并触发卡片重建
- 对外发出的信号：`addDishClicked(int dishId)`、`dishCountChanged(int count)`

### CartPage / QueuePage
- 当前为占位组件，后续在各自文件中独立实现，不影响其他组件

### order_system（order_system.h/cpp）
- 主窗口只剩三件事：
  1. `loadData()` —— 启动时从文件读取所有数据
  2. `setupUI()` —— 实例化各页面组件并连接信号槽
  3. 登录/注册业务逻辑（`checkUser`、`doRegister`、`discountRateForUser`）

## 信号槽连接关系

```
LoginPage::loginClicked       → order_system（验证用户，成功则配置菜单页后跳转）
LoginPage::toRegisterClicked  → switchPage(1)
RegisterPage::registerClicked → order_system（创建用户，配置菜单页后跳转）
RegisterPage::toLoginClicked  → switchPage(0)
NavBar::navClicked            → switchPage(index)
NavBar::logoutRequested       → 清空当前用户，隐藏导航栏，switchPage(0)
MenuPage::addDishClicked      → order_system（状态栏提示，后续扩展为购物车）
MenuPage::dishCountChanged    → NavBar::setDishCount
```

## 后续开发建议

- 实现购物车时只需编辑 `cart_page.h/cpp`，在 `CartPage` 里添加信号与 `order_system` 连接
- 实现排队页同理，只动 `queue_page.h/cpp`
- 如需在菜单页展示折扣价，只需调用 `m_menuPage->setDiscountRate(rate)` 即可触发全量刷新

---

# 数据结构双版本化（2026-06-06）

## 背景

原来 `FileManager.h` 里 `Dish` 和 `Dish_qt` 虽然同时存在，但没有互转函数，需要手动逐字段复制；`User`、`QueueMsg`、`CommentMsg`、`DishComment_msg` 只有 C++ 版本，Qt UI 层每次使用都要手动转换，容易出错且代码重复。本次修改在 `include/FileManager.h` 中统一补齐所有结构体的 Qt 镜像版本和双向转换函数。

## 改动内容

**文件：`include/FileManager.h`**

新增 Qt 版本结构体：

| 新增 Qt 版本 | 对应 C++ 版本 |
|---|---|
| `User_qt` | `User` |
| `CommentMsg_qt` | `CommentMsg`（原定义在 Comment_msg.hpp）|
| `DishComment_msg_qt` | `DishComment_msg`（原定义在 Comment_msg.hpp）|

新增转换函数，作为 `FileManager` 的成员函数（声明在头文件，实现在 `FileManager.cpp`）：

| 函数 | 方向 |
|---|---|
| `dish_to_qt(Dish&)` | `Dish` → `Dish_qt` |
| `dish_to_cpp(const Dish_qt&)` | `Dish_qt` → `Dish` |
| `user_to_qt(const User&)` | `User` → `User_qt` |
| `user_to_cpp(const User_qt&)` | `User_qt` → `User` |
| `queuemsg_to_qt(const QueueMsg&)` | `QueueMsg` → `QueueMsg_qt` |
| `queuemsg_to_cpp(const QueueMsg_qt&)` | `QueueMsg_qt` → `QueueMsg` |
| `commentmsg_to_qt(const CommentMsg&)` | `CommentMsg` → `CommentMsg_qt` |
| `commentmsg_to_cpp(const CommentMsg_qt&)` | `CommentMsg_qt` → `CommentMsg` |
| `dishcomment_to_qt(const DishComment_msg&)` | `DishComment_msg` → `DishComment_msg_qt` |
| `dishcomment_to_cpp(const DishComment_msg_qt&)` | `DishComment_msg_qt` → `DishComment_msg` |

## 实现方式

转换函数统一作为 `FileManager` 的普通成员函数，声明在 `FileManager.h`，函数体实现在 `FileManager.cpp`。这样避免了把 Qt 依赖注入到各结构体自身，结构体保持数据定义职责，转换逻辑集中在 `FileManager` 一处。

`queue_msg.hpp` 和 `Comment_msg.hpp` 不引入任何 Qt 依赖，保持纯 C++ 头文件。

## 使用示例

```cpp
FileManager fm;

// C++ → Qt
User u = fm.getUsers_cpp()[0];
User_qt uq = fm.user_to_qt(u);
nameLabel->setText(uq.name);

// Qt → C++（UI 层构建评论后写回文件）
CommentMsg_qt cq;
cq.user_id  = "1";
cq.dish_ids = {"3", "5"};
cq.comment  = "很好吃！";
cq.rate     = 5;
cq.in_time  = QDateTime::currentDateTime();
fm.AddCommentAndUpdateMenu(fm.commentmsg_to_cpp(cq));

// C++ → Qt（格式化时间戳）
QueueMsg msg = fm.getQueue()[0];
QueueMsg_qt mqt = fm.queuemsg_to_qt(msg);
timeLabel->setText(mqt.in_time.toString("MM-dd HH:mm"));
```

---

# 注意事项
1. CMakelists里记得改下路径 
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/mingw_64") # Qt Kit Dir
2. 这一行记得加入系统的mingw32库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets mingw32) # Qt5 Shared Library 
3. 版本要从11到17
set(CMAKE_CXX_STANDARD 17)
4. 在终端临时设置Qt6Core的PATH
# 如果您使用的是 PowerShell (窗口标题栏或命令提示符里有 "PS")
$env:PATH = "D:\Qt\6.9.3\mingw_64\bin;" + $env:PATH


# 使用说明：mkdir build && cd build 
 cmake ../ui -G "MinGW Makefiles"  
 cmake --build
 ./order_system.exe

# 发布程序封装
当您要把程序发给没有安装 Qt 的朋友时，您需要使用 Qt 提供的 windeployqt 工具，它会自动把所有需要的 .dll 文件从 Qt 目录复制到您的 .exe 旁边，打包成一个可以独立运行的文件夹