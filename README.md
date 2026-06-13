# 饱了么 · 点单管理系统

> C++ 课程期末小组项目。一个用 **Qt6** 写的桌面点餐软件，界面模仿「美团 / 饿了么」，实现了点菜、会员折扣、菜品评价展示、推荐榜、历史订单复制、叫号排队等功能。

这份文档会从「这是什么」开始，一路讲到「每个功能背后的代码是怎么实现的」。**即使你没学过 C++ 或 Qt，也能跟着读懂整个项目。** 没写完的功能会标注出来，方便后面继续补。

---

## 目录

1. [一句话理解这个项目](#1-一句话理解这个项目)
2. [先跑起来：编译和运行](#2-先跑起来编译和运行)
3. [给小白的技术扫盲](#3-给小白的技术扫盲不懂-c--qt-也能看懂下面的代码)
4. [项目目录结构](#4-项目目录结构)
5. [整体架构：数据是怎么流动的](#5-整体架构数据是怎么流动的)
6. [数据文件格式详解](#6-数据文件格式详解)
7. [会员等级与折扣规则](#7-会员等级与折扣规则)
8. [七大功能逐条拆解（对照需求）](#8-七大功能逐条拆解对照需求)
9. [读懂代码：四个反复出现的套路](#9-读懂代码四个反复出现的套路)
10. [已知问题与待办](#10-已知问题与待办还没做完的部分)
11. [给后来开发者的约定](#11-给后来开发者的约定)

---

## 1. 一句话理解这个项目

这是一个**单机版的点餐软件**：用户打开程序后登录，浏览菜单，把菜加进购物车，享受会员折扣，结算后拿到一个取餐号去排队。所有数据（菜单、用户、评论、订单历史）都存在几个 **txt 文本文件**里，程序启动时读进内存，需要时再写回去。

它没有联网、没有数据库、没有服务器，**全部跑在你自己的电脑上**。你可以把它理解成「美团 App 的点餐部分，但是数据存在记事本里」。

---

## 2. 先跑起来：编译和运行

### 2.1 需要准备的环境

- **Qt 6**（本项目在 6.9 / 6.11 上验证过），安装时要带 **MinGW** 编译器套件。
- **CMake**（Qt 安装包里通常自带，或单独装）。

### 2.2 改一处配置（必做）

打开 `ui/CMakeLists.txt`，把第 4 行的 Qt 路径改成你电脑上的真实路径：

```cmake
set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64")   # ← 改成你自己的 Qt 路径
```

### 2.3 编译命令

在项目根目录打开终端，依次执行：

```bash
mkdir build
cd build
cmake ../ui -G "MinGW Makefiles"
cmake --build .
./order_system.exe
```

> 说明：构建的「入口」是 `ui/CMakeLists.txt`，所以 `cmake` 后面跟的是 `../ui`，而不是项目根目录。

### 2.4 可能踩的坑

| 现象 | 原因 / 解决 |
|---|---|
| 找不到 Qt | `CMAKE_PREFIX_PATH` 没改对（见 2.2） |
| 链接报错、缺少 `mingw32` | 部分机器需要在 `ui/CMakeLists.txt` 最后一行加上 `mingw32`：`target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets mingw32)` |
| 运行时提示缺少 `Qt6Core.dll` 之类 | 临时把 Qt 的 bin 目录加进 PATH（PowerShell）：`$env:PATH = "D:\Qt\6.11.1\mingw_64\bin;" + $env:PATH` |
| 新加了 `.cpp` 文件但没被编译 | 删掉 `build/` 重新 `cmake` 一次即可（详见下文构建脚本说明） |

### 2.5 默认可登录的账号

程序自带了一批测试账号（见 `storage/data/users.txt`），密码是明文。例如：

| 用户名 | 密码 | 会员等级 |
|---|---|---|
| 张三 | 12345 | PLATINUM（白金，7.5 折） |
| 李四 | 456789 | SILVER（白银，9.5 折） |
| 王五 | passwd1 | GOLD（黄金，8.5 折） |
| 赵六 | pwd2024 | REGULAR（普通，原价） |

也可以在登录页点「立即注册」创建新账号（新账号默认是 REGULAR 普通会员）。

### 2.6 发布给别人用

如果要把程序发给没装 Qt 的同学，用 Qt 自带的 `windeployqt order_system.exe`，它会自动把所有依赖的 `.dll` 复制到 exe 旁边，打包成一个能独立运行的文件夹。

---

## 3. 给小白的技术扫盲（不懂 C++ / Qt 也能看懂下面的代码）

读后面的代码讲解前，先花两分钟了解几个概念，后面就不会懵了。

### 3.1 什么是 Qt？

Qt 是一个写「图形界面程序」的工具库。你看到的每一个**按钮、文字、输入框、列表**，在代码里都是一个 Qt 提供的「控件」对象。常见的几个：

| 代码里的名字 | 是什么 | 类比 |
|---|---|---|
| `QWidget` | 最基础的「一块界面」 | 一张白纸 |
| `QLabel` | 显示文字 / 图标 | 贴在纸上的标签 |
| `QPushButton` | 可点击的按钮 | 门铃 |
| `QLineEdit` | 单行输入框 | 填空格 |
| `QFrame` | 带边框/背景的方块容器 | 卡片纸盒 |
| `QVBoxLayout` / `QHBoxLayout` | 「布局」，决定控件竖着排还是横着排 | 货架的隔板 |
| `QStackedWidget` | 一叠页面，同一时刻只显示其中一页 | 一摞 PPT，翻到哪页看哪页 |
| `QDialog` | 弹出来的小窗口 | 对话框 |

### 3.2 什么是「信号与槽」（signals & slots）？

这是 Qt 最核心的机制，也是本项目代码里出现最多的写法。一句话：**「某件事发生了」→「就去做某件事」**。

- **信号（signal）**：当某件事发生时，控件会「喊一声」。比如按钮被点击，就会发出 `clicked` 信号。
- **槽（slot）**：收到喊声后要执行的函数。
- **`connect(...)`**：把「谁喊」和「喊了之后做什么」绑在一起。

例子（来自登录页）：

```cpp
connect(loginBtn, &QPushButton::clicked, this, [this]() {
    emit loginClicked(m_nameEdit->text(), m_pwdEdit->text());
});
```

翻译成大白话：**「当 `loginBtn` 这个按钮被点击时，就把用户名和密码打包，发出一个我自己定义的 `loginClicked` 信号。」** 至于点击后真正去验证账号密码的逻辑，写在别处（主窗口）通过 `connect` 接收这个信号——这样**界面**和**业务逻辑**就分开了，互不打扰。

### 3.3 为什么很多结构体有「两个版本」（`Dish` 和 `Dish_qt`）？

项目里几乎每种数据都有两份定义，比如菜品有 `Dish` 和 `Dish_qt`：

- `Dish`：用 C++ 原生的 `std::string` 存文字。用于**读写文件**和**后端计算**。
- `Dish_qt`：把文字换成 Qt 的 `QString`。用于**界面显示**（Qt 控件只认 `QString`）。

```cpp
struct Dish {            // C++ 版：给文件读写 / 逻辑计算用
    int id;
    std::string name;    // ← std::string
    double price;
    /* ... */
};

struct Dish_qt {         // Qt 版：给界面显示用
    int id;
    QString name;        // ← QString
    double price;
    /* ... */
};
```

两者之间的互相转换函数集中放在 `FileManager` 里（如 `dish_to_qt`、`dish_to_cpp`），这样**结构体本身保持干净**，转换逻辑只在一个地方维护。

### 3.4 为什么数据存 txt 而不是数据库？

因为这是教学项目，重点是练 C++ 和 Qt，不是练数据库。所有数据用纯文本文件存储，**人可以直接打开记事本看懂、改动**，也方便老师检查。代价是没有并发、没有事务，但对单机小项目完全够用。

---

## 4. 项目目录结构

```
order_manage_system/
├── include/                     # 所有「共享头文件」（.h / .hpp），定义数据结构和类的声明
│   ├── FileManager.h            # 核心：Dish/User 等数据结构 + FileManager 类（文件读写）
│   ├── Order_mgr.hpp            # OrderService 类（购物车、折扣、结算）
│   ├── queue_msg.hpp            # QueueMsg 结构体（一条排队记录，含序列化）
│   ├── queue_service.h          # QueueService 类（排队逻辑）
│   ├── Comment_msg.hpp          # CommentMsg 结构体（一条评论，含序列化）
│   └── Comment_service.h        # CommentService 类（评论排序，尚未接入）
│
├── storage/
│   ├── FileManager.cpp          # FileManager 的实现：把 txt ↔ 内存结构体互转
│   └── data/                    # ← 所有数据都存在这里
│       ├── menu.txt             # 菜单
│       ├── users.txt            # 用户
│       ├── comment.txt          # 评论
│       ├── queue.txt            # 排队（示例数据，当前未被程序加载）
│       └── history_order.txt    # 历史订单
│
├── order/src/Order_mgr.cpp      # OrderService 的实现
├── queue/src/queue_service.cpp  # QueueService 的实现
├── comment/src/comment_service.cpp  # CommentService 的实现（未编入程序）
│
├── ui/                          # ← 界面层，程序主体
│   ├── CMakeLists.txt           # 构建脚本（唯一入口）
│   └── src/
│       ├── main.cpp             # 程序入口（main 函数）
│       ├── order_system.h/.cpp  # 主窗口：组装所有页面 + 业务逻辑总调度
│       ├── order_system.ui      # 主窗口的 Qt Designer 描述文件
│       ├── nav_bar.h/.cpp       # 顶部导航栏
│       ├── auth_page.h/.cpp     # 登录页 + 注册页（两个类合在一个文件里）
│       ├── menu_page.h/.cpp     # 菜单页（左侧分类 + 右侧菜品卡片）
│       ├── dish_card.h/.cpp     # 单个菜品卡片组件
│       ├── comment_dialog.h/.cpp# 「查看评论」弹窗
│       ├── cart_page.h/.cpp     # 购物车页
│       └── queue_page.h/.cpp    # 排队进度页
│
└── build/                       # CMake 编译输出，不进版本库
```

### 关于构建脚本的一个重点

`ui/CMakeLists.txt` 用下面这行自动收集 `ui/src/` 下所有的 `.cpp`：

```cmake
file(GLOB srcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
```

`CONFIGURE_DEPENDS` 的意思是：**以后在 `ui/src/` 里新增或删除 `.cpp` 文件，重新构建时 CMake 会自动重新扫描**，不会再出现「新加的文件没被编译、链接报 undefined reference」的坑。

不在 `ui/src/` 里的后端源文件，需要在 `CMakeLists.txt` 里手动追加（已经加好的有三个）：

```cmake
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../storage/FileManager.cpp)     # 文件读写
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../order/src/Order_mgr.cpp)     # 购物车/结算
list(APPEND srcs ${CMAKE_CURRENT_SOURCE_DIR}/../queue/src/queue_service.cpp) # 排队
```

> 注意：`comment/src/comment_service.cpp`（评论排序）**还没加进来**，所以目前 `CommentService` 类不参与编译，详见第 10 节。

---

## 5. 整体架构：数据是怎么流动的

整个程序可以分成三层，数据像「下楼梯」一样上下流动：

```
┌─────────────────────────────────────────────┐
│  界面层 (ui/src)                              │   ← 用户看得见、点得到的所有东西
│  order_system 主窗口                           │
│  ├─ NavBar 导航栏                              │
│  ├─ LoginPage / RegisterPage 登录注册          │
│  ├─ MenuPage 菜单（内含一堆 DishCard 卡片）      │
│  ├─ CartPage 购物车                            │
│  └─ QueuePage 排队                             │
└───────────────┬─────────────────────────────┘
                │  调用
                ▼
┌─────────────────────────────────────────────┐
│  服务层（业务逻辑）                             │   ← 「怎么算」的规则
│  ├─ OrderService  购物车、折扣、结算            │
│  ├─ QueueService  排队、叫号                    │
│  └─ CommentService 评论排序（暂未接入）          │
└───────────────┬─────────────────────────────┘
                │  调用
                ▼
┌─────────────────────────────────────────────┐
│  数据层  FileManager                           │   ← 「数据从哪来、到哪去」
│  负责把 txt 文件 ↔ 内存里的结构体 互相转换         │
└───────────────┬─────────────────────────────┘
                │  读写
                ▼
        storage/data/*.txt  （真正的数据）
```

### 启动流程（程序打开后发生了什么）

1. `main.cpp` 创建主窗口 `order_system` 并显示。
2. 主窗口构造时先调用 `loadData()`：通过 `FileManager` 把 `menu.txt`、`users.txt`、`comment.txt` 全部读进内存，并预先算好三个推荐榜（销量/评分/评论数 Top5）。
3. 再调用 `setupUI()`：创建所有页面控件，用 `connect` 把它们的信号接起来，全部塞进一个 `QStackedWidget`（一叠页面）。
4. 默认显示第 0 页（登录页），导航栏隐藏。登录成功后导航栏才出现，并跳到菜单页。

### 页面索引表（`QStackedWidget` 的每一页）

| 索引 | 页面 | 对应类 | 状态 |
|---|---|---|---|
| 0 | 登录页 | `LoginPage` | ✅ 完成 |
| 1 | 注册页 | `RegisterPage` | ✅ 完成 |
| 2 | 菜单页 | `MenuPage` | ✅ 完成 |
| 3 | 购物车页 | `CartPage` | ✅ 完成 |
| 4 | 排队进度页 | `QueuePage` | ✅ 完成 |

「翻页」统一由主窗口的 `switchPage(int index)` 完成，它做两件事：把 `QStackedWidget` 切到指定页，同时让导航栏高亮对应按钮。

---

## 6. 数据文件格式详解

所有文件都在 `storage/data/`。程序运行时的「当前目录」是 `build/`，所以代码里用相对路径 `../storage/data/xxx.txt` 访问它们。规则统一：**以 `#` 开头的行是注释，会被跳过；空行也跳过。**

### 6.1 menu.txt（菜单）

字段用**空格**分隔，顺序固定：

```
# id 菜名 价格 描述 销量 平均得分 类型 评论个数
1 宫保鸡丁 28 鸡肉搭配花生，微辣口味 157 4.5 热菜 2
```

| 字段 | 含义 | 用途 |
|---|---|---|
| id | 菜品编号 | 唯一标识 |
| 菜名 | 名称 | 显示 |
| 价格 | 单价（元） | 算钱 |
| 描述 | 一句话说明 | 显示 |
| 销量 | 卖出份数 | 「销量榜」排序 + 卡片显示「月售 N」 |
| 平均得分 | 平均评分 | 「好评榜」排序 + 卡片显示星级 |
| 类型 | 分类（热菜/凉菜/主食/饮品/甜品/汤羹/烧烤/小吃…） | 左侧分类栏 |
| 评论个数 | 评论数量 | 「最多评价榜」排序 |

> ⚠️ 因为字段靠空格分隔，所以**菜名和描述里不能有空格**，否则解析会错位。

### 6.2 users.txt（用户）

```
# id 用户名 密码 会员等级 总消费
1 张三 12345 PLATINUM 10057
```

`会员等级` 固定是这四个字符串之一：`REGULAR` / `SILVER` / `GOLD` / `PLATINUM`。`总消费` 决定会员等级（见第 7 节）。**密码是明文存储的**（教学项目，未做加密）。

### 6.3 comment.txt（评论）

字段用 **`|`（竖线）** 分隔，其中一条评论可以同时点评多个菜（菜品 id 用逗号分隔）：

```
# user_id|dish_ids(逗号分隔)|评论内容|评分|时间戳
1|1,4|宫保鸡丁很香，麻婆豆腐下饭|5|1779926400
```

意思是：用户 1 同时评价了菜品 1 和菜品 4，内容如上，打 5 分，时间戳是 Unix 秒数。时间戳在界面上会被转换成「MM-dd HH:mm」格式显示。

### 6.4 queue.txt（排队）

```
# queue_id|order_id|时间戳
1|101|1779926400
```

> 说明：这个文件里有示例数据，`FileManager` 也提供了读写它的函数（`LoadQueue`/`SaveQueue`），但**当前的排队功能是内存态的，启动时并没有加载这个文件**（详见第 10 节的待办）。

### 6.5 history_order.txt（历史订单）

每行一笔历史订单：**第一列是用户名**，后面跟着这笔订单点过的菜名（空格分隔，可以有任意多个）：

```
# 用户名 菜名1 菜名2 ...
张三 冰镇可乐 酸梅汤 豆浆 拍黄瓜 炸酱面 鲜肉小笼包
```

「历史订单点菜」功能就是读这个文件，把某一行的菜一键加回购物车。

---

## 7. 会员等级与折扣规则

会员等级根据**累计总消费**自动升级，不同等级享受不同折扣：

| 等级 | 升级门槛（总消费） | 折扣率 | 含义 |
|---|---|---|---|
| REGULAR | < 1000 | 1.00 | 原价 |
| SILVER | ≥ 1000 | 0.95 | 95 折 |
| GOLD | ≥ 5000 | 0.85 | 85 折 |
| PLATINUM | ≥ 10000 | 0.75 | 75 折 |

- **折扣率**由 `OrderService::getDiscountRate()` 根据等级返回。
- **等级升级**在 `OrderService::updateUser()` 里：每次结算把本单金额加到总消费上，再用上面的门槛重新判定等级，写回 `users.txt`。
- 界面上的体现：在 `DishCard` 里，非普通会员会看到「灰色删除线原价 + 金橙色会员价（`#E8960A`）」两行价格。

---

## 8. 七大功能逐条拆解（对照需求）

需求文档 `requirement.md` 列了七个功能。下面逐条说明：**需求是什么 → 涉及哪些文件 → 关键代码怎么写的 → 用大白话解释思路**。

### 功能一：菜单创建（分类）+ 菜品说明 ✅ 已完成

> 需求：有体系地按分类创建菜单，菜品除价格外还有说明。

- **数据**：`storage/data/menu.txt`，每道菜带价格、描述、分类。
- **读取**：`FileManager::LoadMenu()` 逐行解析 txt，填进内存，同时收集所有出现过的「类型」做成分类列表。
- **显示**：`MenuPage` 左侧是分类列表（`QListWidget`），右侧是菜品卡片滚动区；每道菜用一个 `DishCard` 卡片显示（图片占位、菜名、描述、评分、月售、价格、加购按钮）。

点分类时怎么只显示该类的菜？看 `MenuPage::refreshDishList`：

```cpp
for (const auto &item : m_allItems) {
    if (item.category == category || category == "全部")
        makeCard(item);   // 类别匹配（或选了"全部"）才生成卡片
}
```

### 功能二：消费价格计算 + 会员折扣 ✅ 已完成

> 需求：根据所选菜品算钱，实际消费累计到会员等级，不同等级享受不同折扣。

- **核心类**：`OrderService`（`order/src/Order_mgr.cpp`）。
- 购物车其实就是一个 `std::vector<Dish> order_`，**每加一份菜就 push 一条**，所以同一道菜点两份会有两条记录。
- 结算逻辑 `checkout()`：

```cpp
double OrderService::checkout() {
    total_price_ = calcCart() * getDiscountRate(user_);  // 总价 × 折扣率
    updateUser(user_, order_, total_price_);             // 写历史、加销量、升等级
    return total_price_;
}
```

`updateUser` 做了三件事：往 `history_order.txt` 追加这笔订单、把 `menu.txt` 里相应菜的销量 +1、更新 `users.txt` 里该用户的总消费并重新判定会员等级。

- **界面**：`CartPage` 用 `QMap<int,int>` 统计每道菜的数量做「× N」展示，右侧实时算出商品合计、会员折扣减免、实付款。

### 功能三：点餐评价 + 按评分/时间排序 ⚠️ 部分完成

> 需求：可以评价点过的餐，且能按评价高低、留言先后排序显示。

**已完成的部分（看评价）**：

- `DishCard` 上的「查看评论 ›」会打开 `CommentDialog` 弹窗。
- 弹窗读 `comment.txt`，筛选出当前这道菜的所有评论，顶部展示**平均分 + 五星分布条**，下方按卡片列出每条评论（头像、用户名、星级、时间、正文）。

**还没完成的部分（写评价 + 真正的排序）**：

- 目前评论弹窗是**只读**的，**没有「我要评价」的输入入口**。
- `CommentService`（`comment/src/comment_service.cpp`）已经写好了「按评分排序 / 按时间排序 / 取好评 Top5」等逻辑，但**它还没被编进程序**（CMakeLists 没加它），所以当前评论只是按文件里的先后顺序显示，没有用上这些排序功能。
- 写评论的底层能力其实也有了：`FileManager::AddCommentAndUpdateMenu()` 可以追加一条评论并同步更新菜品的平均分和评论数，只是界面还没调用它。

> 👉 后续要做：在评论弹窗里加一个评分+留言的输入框，提交时调用 `AddCommentAndUpdateMenu`；并把 `CommentService` 编进来负责排序。

### 功能四：推荐点餐（销量 / 评分 Top5）✅ 已完成

> 需求：统计每道菜的售卖和评价情况，把最好的 5 个菜额外标注出来。

- **预计算**：`FileManager::LoadMenu()` 在读完菜单后，立刻按销量、评分、评论数各排一次序，各取前 5 名，存成三个推荐榜：

```cpp
std::sort(copy.begin(), copy.end(),
          [](const Dish_qt &a, const Dish_qt &b){ return a.sales > b.sales; });
for (int i = 0; i < qMin(5, copy.size()); i++)
    recommend_by_sales.append(copy[i]);   // 销量前 5
```

- **「推荐」分类**：菜单左侧选「推荐」时，顶部出现三个切换按钮「销量最高 / 评分最高 / 最多评价」，对应展示这三个榜。
- **榜单徽章**：`MenuPage::refreshDishList` 会查出每道菜在销量榜/好评榜里的名次，传给 `DishCard`，卡片上就会显示「本店销量第 N」（橙色）或「好评榜第 N」（绿色）的小标签。

### 功能五：点餐记忆（复制历史订单）✅ 已完成

> 需求：根据购买记录快速复制、创建新订单。

- 入口：购物车为空时，会有一个「历史订单点菜」按钮。
- `OrderService::loadUserHistoryOrders()` 按**用户名**从 `history_order.txt` 里读出该用户的所有历史订单。
- 主窗口弹出一个对话框列出这些订单（把菜名用「、」连起来显示），用户选一条点「加入购物车」，程序就把这条订单里、**当前菜单中仍然存在**的菜逐个加回购物车（菜单里已经下架的菜会提示「X 道菜当前菜单中不存在」）。

### 功能六：排队显示（预约排队 + 取餐排队）✅ 已完成

> 需求：刷新后能实时显示预约排队进度和取餐排队进度。

- **核心类**：`QueueService`（`queue/src/queue_service.cpp`），内部维护两条队列：
  - `waiting_`（预约排队）：下单后在这里等着被叫号。
  - `taking_`（取餐排队）：被叫到号、可以取餐的。
- **下单入队**：结算成功后，主窗口调用 `m_queueService.in_queue(...)` 把订单加入 `waiting_`，并返回一个**取餐号**（从 1001 开始递增），结算成功的弹窗里会告诉用户这个号。
- **叫号前进**：`advance_queue()` 把 `waiting_` 队首的人移到 `taking_`，当前叫号数 +1。
- **界面**：`QueuePage`（美团风格）顶部显示「当前叫号」大数字，下面分左右两栏——左「预约排队中」、右「待取餐」，**自己的号会高亮成橙色**，还会显示「前面还有 N 桌」。底部有「刷新进度」「叫号（前台）」「返回菜单」三个按钮。
- 页面和后端通过信号解耦：`QueuePage` 只发 `refreshRequested / advanceRequested / backToMenuRequested` 信号，真正操作 `QueueService` 的逻辑在主窗口里。

> 小提醒：当前排队是**内存态**的——关掉程序就清空，也不会读取 `queue.txt`。如需「关掉再打开还能看到进度」，需要接上 `FileManager` 的 `LoadQueue/SaveQueue` 做持久化（见第 10 节）。

### 功能七：系统界面 ✅ 已完成

> 需求：完整的系统界面。

整套界面统一走「美团 / 饿了么」风格：圆角白卡片、蓝色主按钮、橙色价格、灰底背景。导航栏、登录注册、菜单、购物车、评论弹窗、排队页都已成型。

---

## 9. 读懂代码：四个反复出现的套路

只要掌握下面四个套路，整个项目的 UI 代码你就能读懂八成。

### 套路一：页面只「发信号」，主窗口才「做事情」

每个页面（登录、购物车、排队…）都**不直接处理业务**，只负责显示，并在用户操作时**发出信号**。主窗口 `order_system` 用 `connect` 接住所有信号，统一处理。

```cpp
// 购物车页：用户点「去结算」，只发一个信号，自己什么都不算
connect(m_checkoutBtn, &QPushButton::clicked, this, &CartPage::checkoutRequested);

// 主窗口：接住信号，才真正去结算、更新会员、入队、弹提示
connect(m_cartPage, &CartPage::checkoutRequested, this, [this]() {
    double total = m_orderService->checkout();
    /* ...更新数据、入队、弹窗... */
});
```

**好处**：界面和逻辑分家，改界面不动逻辑，改逻辑不动界面。

### 套路二：用 `QStackedWidget` 管理「翻页」

所有页面叠在一起，靠 `switchPage(index)` 切换：

```cpp
void order_system::switchPage(int index) {
    if (index == 3 /*购物车*/) { /* 切过去前先刷新购物车内容 */ }
    if (index == 4 /*排队*/)   { refreshQueuePage(); }
    m_stackedWidget->setCurrentIndex(index);  // 真正翻页
    m_navBar->setActiveNav(index);            // 导航栏高亮
}
```

### 套路三：数据变了，就「清空旧卡片、重建新卡片」

菜单和购物车的列表不是「改动单个项」，而是**整个清空重画**——简单、不易出 bug。例如菜单刷新：

```cpp
// 1. 把旧卡片全部删掉
while ((child = m_dishListLayout->takeAt(0)) != nullptr) {
    if (child->widget()) child->widget()->deleteLater();
    delete child;
}
// 2. 按当前数据重新生成卡片
for (const auto &item : *src) makeCard(item);
```

### 套路四：`std::string` 版和 `QString` 版之间来回转

界面要显示就转成 Qt 版，要写文件/算逻辑就转回 C++ 版，转换函数都在 `FileManager` 里：

```cpp
Dish d = m_fl.dish_to_cpp(item);   // Qt 版 → C++ 版，交给 OrderService 处理
m_orderService->addDish(d);
```

---

## 10. 已知问题与待办（还没做完的部分）

| 项目 | 现状 | 建议 |
|---|---|---|
| **写评论（功能三的一半）** | 评论弹窗只能看，不能写 | 在 `CommentDialog` 加评分+留言输入框，提交时调用 `FileManager::AddCommentAndUpdateMenu` |
| **评论排序 / 好评 Top5** | `CommentService` 写好了但没编进程序 | 在 `ui/CMakeLists.txt` 追加 `comment/src/comment_service.cpp`，并把弹窗的排序改成调用它 |
| **排队持久化** | 排队是内存态，关程序就清空，不读 `queue.txt` | 用 `FileManager::LoadQueue/SaveQueue` 在启动时载入、变动时写回 |
| **`addUser` 写入格式** | `FileManager::addUser` 把会员等级写成了整数 `0`，且没写总消费字段 | 改成写字符串 `"REGULAR"` 并补上 `total_spent`（如 `0`） |
| **导航栏里的历史订单按钮** | `order_system.cpp` 里建了一个隐藏的 `historyOrderBtn` 没用上 | 真正入口是购物车空状态的按钮；这个可以删掉以免混淆 |
| **菜名/描述不能含空格** | menu.txt 用空格分隔字段，菜名带空格会解析错位 | 约定不带空格，或改用其他分隔符 |

---

## 11. 给后来开发者的约定

- **加新页面 / 新组件**：在 `ui/src/` 下新建 `xxx.h/.cpp` 即可，`CMakeLists.txt` 会自动收录（`file(GLOB ... CONFIGURE_DEPENDS)`）。新增后记得重新 `cmake` 一次。
- **加新的后端模块**（不在 `ui/src/` 里的 `.cpp`）：要在 `ui/CMakeLists.txt` 里手动 `list(APPEND srcs ...)`。
- **页面与逻辑解耦**：页面只发信号、提供 `setXxx()` 接口刷新显示；所有业务逻辑写在 `order_system` 里。不要让页面直接读写文件或调用后端服务。
- **数据转换**：需要在 `std::string` 版和 `QString` 版之间转换时，一律用 `FileManager` 里现成的转换函数，不要手动逐字段复制。
- **改 Qt 路径**：每个人电脑上的 Qt 安装路径不同，改 `CMakeLists.txt` 的 `CMAKE_PREFIX_PATH` 时**不要提交覆盖别人的路径**（或在本地改、提交时还原）。

---

*本 README 基于当前代码实际实现编写。功能状态（✅ 已完成 / ⚠️ 部分完成）会随开发推进变化，改动相关代码时请同步更新本文档。*

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


# 使用说明：
 mkdir build && cd build 
 cmake ../ui -G "MinGW Makefiles"  
 cmake --build .
 ./order_system.exe

---

# 可移植打包流程（发给没装 Qt 的电脑运行）

## 原理

`windeployqt` 是 Qt 自带的部署工具，它会自动分析 exe 依赖了哪些 Qt 模块，然后把所有需要的 `.dll`、插件目录、翻译文件复制到 exe 旁边，形成一个**可以独立运行的文件夹**。

## 完整步骤

### 第一步：编译

在项目根目录打开终端：

```powershell
# 先临时把 Qt 的 bin 目录加入 PATH（编译和 windeployqt 都要用）
$env:PATH = "D:\Qt\6.9.3\mingw_64\bin;" + $env:PATH

mkdir build
cd build
cmake ../ui -G "MinGW Makefiles"
cmake --build .
```

### 第二步：部署 Qt 依赖

```powershell
# 仍在 build/ 目录下
windeployqt order_system.exe --qmldir ..\ui\src
```

这一步会自动复制以下内容到 `build/`：

| 复制内容 | 说明 |
|---|---|
| `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll` 等 | Qt 运行时库 |
| `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` | MinGW 运行时库 |
| `opengl32sw.dll`, `D3Dcompiler_47.dll` | 图形渲染回退 |
| `platforms/` | Qt 平台插件（`qwindows.dll`，必须） |
| `imageformats/` | 图片格式插件（jpg/png/gif/svg） |
| `styles/` | 界面样式插件 |
| `iconengines/`, `tls/`, `networkinformation/`, `generic/` | 其他必要插件 |
| `translations/` | Qt 内置多语言翻译 |

### 第三步：复制数据文件

程序运行时从 `../storage/data/` 加载菜单、用户、评论等数据，所以需要在 exe 旁边建同样的目录：

```powershell
mkdir storage\data
copy ..\storage\data\menu.txt          storage\data\
copy ..\storage\data\users.txt         storage\data\
copy ..\storage\data\queue.txt         storage\data\
copy ..\storage\data\comment.txt       storage\data\
copy ..\storage\data\history_order.txt storage\data\
```

### 第四步：打包成 zip

```powershell
# 回到项目根目录
cd ..

# 用 PowerShell 打包（排除编译中间文件）
powershell -Command "Compress-Archive -Path build\order_system.exe,build\Qt6Core.dll,build\Qt6Gui.dll,build\Qt6Network.dll,build\Qt6Svg.dll,build\Qt6Widgets.dll,build\libgcc_s_seh-1.dll,build\libstdc++-6.dll,build\libwinpthread-1.dll,build\opengl32sw.dll,build\D3Dcompiler_47.dll,build\generic,build\iconengines,build\imageformats,build\networkinformation,build\platforms,build\styles,build\tls,build\translations,build\storage -DestinationPath order_system_portable.zip -Force"
```

生成的 `order_system_portable.zip`（约 26MB）就是可以分发的便携包。

### 在目标电脑上使用

1. 把 `order_system_portable.zip` 解压到任意目录
2. 双击 `order_system.exe` 即可运行
3. **不需要安装 Qt**，所有依赖已包含在文件夹内

## 一键脚本（PowerShell）

把上面的步骤合并成一个脚本，放在项目根目录运行：

```powershell
# deploy.ps1 — 一键编译 + 打包
$env:PATH = "D:\Qt\6.9.3\mingw_64\bin;" + $env:PATH

# 重新编译
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
mkdir build; cd build
cmake ../ui -G "MinGW Makefiles"
cmake --build .
if ($LASTEXITCODE -ne 0) { Write-Error "编译失败"; exit 1 }

# 部署 Qt 依赖
windeployqt order_system.exe --qmldir ..\ui\src

# 复制数据文件
mkdir storage\data -Force
copy ..\storage\data\menu.txt          storage\data\
copy ..\storage\data\users.txt         storage\data\
copy ..\storage\data\queue.txt         storage\data\
copy ..\storage\data\comment.txt       storage\data\
copy ..\storage\data\history_order.txt storage\data\

cd ..

# 打包
powershell -Command "Compress-Archive -Path build\order_system.exe,build\Qt6Core.dll,build\Qt6Gui.dll,build\Qt6Network.dll,build\Qt6Svg.dll,build\Qt6Widgets.dll,build\libgcc_s_seh-1.dll,build\libstdc++-6.dll,build\libwinpthread-1.dll,build\opengl32sw.dll,build\D3Dcompiler_47.dll,build\generic,build\iconengines,build\imageformats,build\networkinformation,build\platforms,build\styles,build\tls,build\translations,build\storage -DestinationPath order_system_portable.zip -Force"

Write-Host "✓ 打包完成: order_system_portable.zip" -ForegroundColor Green
```

以后更新程序后，只需要运行一次这个脚本就能得到新的便携包。