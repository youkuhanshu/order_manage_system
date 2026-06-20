# Qt 6 基础教程 —— 以「订单管理系统」项目为例

> 本教程以你眼前的项目代码为实例，从零讲解 Qt 的核心知识。
> 阅读方式：**对着实际代码看**，每学一个概念就去项目里找到对应的文件印证。

---

## 目录

1. [第一部分：Qt 是什么](#第一部分qt-是什么)
2. [第二部分：信号与槽 —— Qt 的灵魂](#第二部分信号与槽--qt-的灵魂)
3. [第三部分：父子对象与内存管理](#第三部分父子对象与内存管理)
4. [第四部分：项目是怎么跑起来的](#第四部分项目是怎么跑起来的)
5. [第五部分：窗口与布局](#第五部分窗口与布局)
6. [第六部分：常用控件详解](#第六部分常用控件详解)
7. [第七部分：自定义控件（DishCard 实例）](#第七部分自定义控件dishcard-实例)
8. [第八部分：Qt 样式表（QSS）](#第八部分qt-样式表qss)
9. [第九部分：文件读写与数据模型](#第九部分文件读写与数据模型)
10. [第十部分：对照作业需求分析](#第十部分对照作业需求分析)

---

## 第一部分：Qt 是什么

### 1.1 一句话理解

**Qt 是一个用 C++ 写桌面应用（GUI）的框架。** 它提供了一套现成的"控件"（按钮、列表、输入框等），让你不用从零画界面。

可以类比：
- 你用 Windows 记事本 —— 那是 Win32 API 写的，代码极其繁琐
- 你用 VS Code —— 那是 Electron（HTML/CSS/JS），吃内存
- **Qt 是 C++ 写桌面应用的最佳选择之一**，性能好、跨平台（Windows/Mac/Linux）

### 1.2 我们用的是 Qt 6

查看你电脑上的版本：打开 [ui/CMakeLists.txt](ui/CMakeLists.txt)，看到：

```cmake
set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64")
find_package(Qt6 COMPONENTS Widgets REQUIRED)
```

- `Qt 6.11.1` —— 版本号
- `mingw_64` —— 编译器是 MinGW（Windows 上的 GCC）
- `Widgets` —— 我们只用了 Qt 的 Widgets 模块（还有 Qt Quick/QML 等其他模块）

### 1.3 Qt 程序的最小框架

看 [ui/src/main.cpp](ui/src/main.cpp)，这是**每个 Qt 程序的入口**：

```cpp
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);   // ① 创建应用程序对象（必须）
    order_system w;               // ② 创建主窗口
    w.show();                     // ③ 显示窗口
    return a.exec();              // ④ 进入事件循环，等待用户操作
}
```

**关键理解**：`a.exec()` 是一个**死循环**（事件循环），它不停检查有没有用户操作（鼠标点击、键盘输入等），有就分发给对应的控件处理。程序直到窗口关闭才退出这个循环。

---

## 第二部分：信号与槽 —— Qt 的灵魂

### 2.1 问题引入

假设用户点击了分类列表中的"热菜"，右边的菜品列表需要更新。**怎么通知？**

传统做法（回调函数）：把函数指针传来传去，代码散落各处，难维护。

Qt 的做法：**信号与槽（Signals & Slots）**

### 2.2 一句话理解

> **信号（signal）**：某个事件发生了，喊一声
> **槽（slot）**：听到喊声后要做的动作
> **connect**：把喊的人和听的人连起来

### 2.3 实际例子（对着代码看）

打开 [ui/src/order_system.cpp](ui/src/order_system.cpp)，找到这行：

```cpp
connect(m_categoryList, &QListWidget::currentRowChanged,
        this,           &order_system::onCategoryChanged);
```

翻译成人话：
- `m_categoryList`（分类列表）的 `currentRowChanged` 信号发出时
- `this`（主窗口）的 `onCategoryChanged` 槽函数被执行

**信号** `currentRowChanged` 是 `QListWidget` 自带的，当用户点击不同行时自动发射。

**槽** `onCategoryChanged` 是我们自己写的：

```cpp
void order_system::onCategoryChanged(int row)
{
    // row 是信号传过来的参数：当前选中的是第几行
    const QString cat = (row == 0) ? QString() 
                                   : m_categoryList->item(row)->text();
    refreshDishList(cat);  // 刷新右侧菜品列表
}
```

### 2.4 自定义信号（DishCard 的例子）

打开 [ui/src/dish_card.h](ui/src/dish_card.h)：

```cpp
class DishCard : public QFrame
{
    Q_OBJECT          // ← 必须有这个宏，信号/槽才能用

signals:              // ← 声明信号（只声明，不实现）
    void addClicked(int dishId);
};
```

在 [ui/src/dish_card.cpp](ui/src/dish_card.cpp) 中发射信号：

```cpp
connect(m_addBtn, &QPushButton::clicked, this, [this]() {
    emit addClicked(m_item.id);   // ← emit 发射自定义信号
});
```

然后在主窗口中接收：

```cpp
connect(card, &DishCard::addClicked,   // DishCard 的信号
        this, &order_system::onAddDish); // 主窗口的槽
```

### 2.5 信号与槽的规则

| 规则 | 说明 |
|------|------|
| 信号和槽的参数**类型和数量要匹配** | 槽的参数可以比信号少（忽略多余的） |
| 一个信号可以连多个槽 | 全部都会被调用 |
| 多个信号可以连一个槽 | 任一信号触发都会调用 |
| `Q_OBJECT` 宏 | 使用了 signals/slots 的类**必须**在类声明中写这个宏 |
| 跨线程也是安全的 | 初学者先不用管这个 |

### 2.6 Lambda 写法（C++11）

除了连到成员函数，也可以直接写 lambda：

```cpp
connect(m_addBtn, &QPushButton::clicked, this, [this]() {
    emit addClicked(m_item.id);
});
```

`[this]()` 表示捕获 `this` 指针（可以访问成员变量），括号里是参数。

---

## 第三部分：父子对象与内存管理

### 3.1 Qt 的自动内存管理

看代码时你可能会疑惑：`new` 了那么多对象，怎么从来不见 `delete`？

```cpp
auto *nameLabel = new QLabel("宫保鸡丁", this);  // ↑ this 是父对象
// 没有 delete nameLabel —— 它不会内存泄漏！
```

**规则**：Qt 对象在构造时指定了父对象（`this`），当父对象被销毁时，所有子对象自动被 `delete`。

所以：
- 窗口关闭 → `order_system` 析构 → 所有控件自动销毁
- 你只需要 `new`，不需要手动 `delete`（大多数情况）

### 3.2 注意

- 父对象必须是 `QObject`（或其子类）
- 用 `new` 分配在堆上（不要用栈对象）
- 布局（Layout）不算父对象，但控件加到布局后，布局所在的 Widget 会成为它们的父对象

---

## 第四部分：项目是怎么跑起来的

### 4.1 从源码到可执行文件

```
源代码                    构建工具              产物
───────                  ────────              ────
main.cpp        ─┐
order_system.cpp ─┤
order_system.h   ─┤     CMake 配置
order_system.ui  ─┤  →  Ninja 编译  →  order_system.exe
dish_card.cpp    ─┤
dish_card.h      ─┤
FileManager.cpp  ─┘
```

### 4.2 三个自动工具（重点理解）

打开 [ui/CMakeLists.txt](ui/CMakeLists.txt)：

```cmake
set(CMAKE_AUTOMOC ON)   # ① 自动 MOC
set(CMAKE_AUTOUIC ON)   # ② 自动 UIC
set(CMAKE_AUTORCC ON)   # ③ 自动 RCC
```

| 工具 | 作用 | 通俗解释 |
|------|------|---------|
| **MOC**（元对象编译器） | 处理 `Q_OBJECT`、`signals`、`slots` | Qt 扩展了 C++ 语法（信号/槽），MOC 把它翻译成标准 C++ |
| **UIC**（UI 编译器） | 把 `.ui` 文件编译成 C++ 头文件 | 把可视化设计的界面转成代码 |
| **RCC**（资源编译器） | 把图片、图标等编译进程序 | 打包资源文件 |

**关键流程**：

```
dish_card.h（含 Q_OBJECT）
       │
       ▼  MOC 处理
moc_dish_card.cpp（生成的标准 C++ 代码）
       │
       ▼  编译器
dish_card.obj  ← 和其他 .obj 一起链接
```

```
order_system.ui（XML 界面文件）
       │
       ▼  UIC 处理
ui_order_system.h（生成的 C++ 类 Ui_order_system）
       │
       ▼  被 order_system.h #include
       │
       ▼  编译器
order_system.obj
```

### 4.3 CMakeLists.txt 逐行解释

```cmake
cmake_minimum_required(VERSION 3.5)          # CMake 最低版本
project(order_system LANGUAGES CXX)          # 项目名 + C++ 语言

set(CMAKE_PREFIX_PATH "D:/Qt/6.11.1/mingw_64")  # Qt 安装路径
set(CMAKE_AUTOUIC ON)    # 自动编译 .ui 文件
set(CMAKE_AUTOMOC ON)    # 自动处理 Q_OBJECT
set(CMAKE_AUTORCC ON)    # 自动编译资源文件

set(CMAKE_CXX_STANDARD 11)          # 使用 C++11 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制要求 C++11

find_package(Qt6 COMPONENTS Widgets REQUIRED)  # 找到 Qt6 的 Widgets 模块

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)  # 添加头文件路径

aux_source_directory(./src srcs)    # 自动收集 ./src 下所有 .cpp 文件
list(APPEND srcs ../storage/FileManager.cpp)  # 额外添加 storage 目录的源文件

add_executable(${PROJECT_NAME} WIN32 ${srcs})  # 生成可执行文件
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)  # 链接 Qt 库
```

---

## 第五部分：窗口与布局

### 5.1 QMainWindow 的结构

每个 Qt 主窗口（QMainWindow）有一个固定的结构：

```
┌──────────────────────────────────────┐
│              Menu Bar（菜单栏）        │
├──────────────────────────────────────┤
│                                      │
│          Central Widget（中心区）      │
│          ← 我们所有内容都放这里        │
│                                      │
├──────────────────────────────────────┤
│            Status Bar（状态栏）        │
└──────────────────────────────────────┘
```

对应代码（[order_system.cpp](ui/src/order_system.cpp)）：

```cpp
// setupUi 之后，ui 里已经有了这三个区域
ui->centralwidget   // QWidget —— 我们往里加布局和控件
ui->menubar         // QMenuBar
ui->statusbar       // QStatusBar
```

### 5.2 布局管理器（Layout）—— 最重要的概念

**不要用绝对坐标！** 如果你的窗口缩放，`(100, 50)` 位置的按钮就不在原来的地方了。

Qt 的解决方案：**布局管理器**，控件会自动排列、自动适应窗口大小。

#### QVBoxLayout（垂直排列）

```cpp
auto *vLayout = new QVBoxLayout(parentWidget);
vLayout->addWidget(widget1);  // 在最上面
vLayout->addWidget(widget2);  // 在中间
vLayout->addWidget(widget3);  // 在最下面
vLayout->addStretch();        // 弹簧，把上面的内容顶到顶部
```

效果：
```
┌──────────────┐
│   widget1    │
│   widget2    │
│   widget3    │
│              │  ← stretch 撑开空白
│              │
└──────────────┘
```

#### QHBoxLayout（水平排列）

```cpp
auto *hLayout = new QHBoxLayout();
hLayout->addWidget(leftWidget);
hLayout->addWidget(centerWidget, 1);  // stretch factor = 1（会拉伸占满剩余空间）
hLayout->addWidget(rightWidget);
```

效果：
```
┌────────┬──────────────────┬────────┐
│  left  │  center（拉伸）   │ right  │
└────────┴──────────────────┴────────┘
```

#### 嵌套布局（项目中的实际例子）

看 [order_system.cpp](ui/src/order_system.cpp) 的 `setupUI()`：

```cpp
// 整体：垂直布局
auto *mainLayout = new QVBoxLayout(ui->centralwidget);

    // ① 顶部标题栏（一个 QFrame）
    mainLayout->addWidget(topBar);

    // ② 内容区：水平布局（左分类 + 右菜品）
    auto *contentLayout = new QHBoxLayout();

        contentLayout->addWidget(m_categoryList);   // 左侧分类
        contentLayout->addWidget(m_scrollArea, 1);  // 右侧滚动区（stretch=1）

    mainLayout->addLayout(contentLayout, 1);  // stretch=1，填满剩余空间
```

布局嵌套的树形结构：

```
QVBoxLayout（主布局）
├── QFrame（顶部标题栏）
└── QHBoxLayout（内容区）
    ├── QListWidget（左侧分类列表，固定宽度 150px）
    └── QScrollArea（右侧滚动区，stretch=1 占满）
        └── dishContainer（QVBoxLayout）
            ├── DishCard（菜品 1）
            ├── DishCard（菜品 2）
            └── ... Stretch
```

### 5.3 QScrollArea（滚动区域）

当菜品很多时，需要滚动查看。`QScrollArea` 就是干这个的：

```cpp
m_scrollArea = new QScrollArea();
m_scrollArea->setWidgetResizable(true);  // 内部 widget 可以自动调整大小

m_dishContainer = new QWidget();          // 这个是真正装内容的容器
m_dishListLayout = new QVBoxLayout(m_dishContainer);  // 给它一个布局

m_scrollArea->setWidget(m_dishContainer); // 把容器放进滚动区
```

理解：`QScrollArea` 像一个"窗口"，透过它看 `m_dishContainer`。当 `m_dishContainer` 比窗口高时，就出现滚动条。

---

## 第六部分：常用控件详解

### 6.1 QLabel（标签 —— 显示文字）

最基本也最常用的控件。

```cpp
// 创建
auto *label = new QLabel("宫保鸡丁", parent);

// 设置样式（后面细讲）
label->setStyleSheet("font-size: 16px; color: #333333;");

// 设置对齐
label->setAlignment(Qt::AlignCenter);

// 设置最大宽度
label->setMaximumWidth(350);
```

项目中几乎所有的文字显示都是 QLabel：
- 菜名 → QLabel
- 描述 → QLabel
- 价格 → QLabel
- 评分 → QLabel

### 6.2 QPushButton（按钮）

```cpp
auto *btn = new QPushButton("+", parent);
btn->setFixedSize(30, 30);          // 固定大小
btn->setCursor(Qt::PointingHandCursor);  // 鼠标悬停时变成手型

// 点击事件（三种写法）
// 写法一：连到槽函数
connect(btn, &QPushButton::clicked, this, &MyClass::onClicked);

// 写法二：Lambda
connect(btn, &QPushButton::clicked, this, [this]() {
    // 点击后做什么
});
```

### 6.3 QListWidget（列表）

项目中左侧分类列表用的就是这个。

```cpp
m_categoryList = new QListWidget();

// 添加项目
m_categoryList->addItem("全部");
m_categoryList->addItem("热菜");
m_categoryList->addItem("甜品");

// 设置默认选中第一项
m_categoryList->setCurrentRow(0);

// 监听选中变化
connect(m_categoryList, &QListWidget::currentRowChanged,
        this, &order_system::onCategoryChanged);
```

### 6.4 QFrame（带边框的容器）

`DishCard` 继承自 `QFrame`。`QFrame` 比 `QWidget` 多了边框和形状支持，适合做卡片。

```cpp
class DishCard : public QFrame  // ← 继承 QFrame
{
    Q_OBJECT
    // ...
};
```

### 6.5 QStatusBar（状态栏）

主窗口底部的信息栏：

```cpp
ui->statusbar->showMessage("就绪");                    // 一直显示
ui->statusbar->showMessage("已将宫保鸡丁加入购物车", 2000); // 2 秒后消失
```

---

## 第七部分：自定义控件（DishCard 实例）

### 7.1 为什么要自定义控件

Qt 没有现成的"菜品卡片"。我们需要把图片、菜名、描述、价格、按钮组合成一个可复用的卡片。

### 7.2 步骤拆解

打开 [ui/src/dish_card.h](ui/src/dish_card.h) 和 [ui/src/dish_card.cpp](ui/src/dish_card.cpp)。

#### 第一步：创建头文件，声明类

```cpp
class DishCard : public QFrame      // ① 继承 QFrame
{
    Q_OBJECT                         // ② 必须写，信号/槽才能用

public:
    explicit DishCard(const MenuItem &item, QWidget *parent = nullptr);

signals:
    void addClicked(int dishId);     // ③ 自定义信号

private:
    void setupUI(const MenuItem &item);  // ④ 构建界面的私有方法

    // ⑤ 控件的成员变量（需要在多个方法中访问，所以存为成员）
    QLabel *m_nameLabel;
    QLabel *m_priceLabel;
    QPushButton *m_addBtn;
    MenuItem m_item;                 // ⑥ 数据
};
```

#### 第二步：构造函数中调用 setupUI

```cpp
DishCard::DishCard(const MenuItem &item, QWidget *parent)
    : QFrame(parent), m_item(item)
{
    setupUI(item);  // 构建整个卡片的界面
}
```

#### 第三步：setupUI 中搭布局

```cpp
void DishCard::setupUI(const MenuItem &item)
{
    // A. 设置卡片自身的样式
    setFixedHeight(120);          // 卡片高度固定
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //  ↑ 宽度随父容器拉伸

    // B. 创建主布局（水平：左图 + 中间信息 + 右价格）
    auto *mainLayout = new QHBoxLayout(this);

    // C. 左侧图片占位
    auto *imageFrame = new QFrame();
    imageFrame->setFixedSize(94, 94);

    // D. 中间信息区（垂直布局）
    auto *infoLayout = new QVBoxLayout();
    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(descLabel);
    infoLayout->addStretch();  // 弹簧
    infoLayout->addLayout(statsLayout);  // 嵌套：评分+销量

    // E. 右侧价格 + 按钮（垂直布局）
    auto *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(priceLabel);
    rightLayout->addWidget(addBtn);

    // F. 把三个区域加入主布局
    mainLayout->addWidget(imageFrame);      // 左
    mainLayout->addLayout(infoLayout, 1);   // 中（stretch=1）
    mainLayout->addLayout(rightLayout);     // 右
}
```

布局树：
```
QHBoxLayout（主布局）
├── QFrame（图片占位，94×94 固定）
├── QVBoxLayout（信息区，stretch=1）
│   ├── QLabel（菜名）
│   ├── QLabel（描述）
│   ├── Stretch
│   └── QHBoxLayout（评分+销量）
│       ├── QLabel（★ 4.8）
│       └── QLabel（月售 156）
└── QVBoxLayout（价格区）
    ├── QLabel（¥28）
    └── QPushButton（+）
```

### 7.3 在别处使用 DishCard

```cpp
// 创建一张卡片
auto *card = new DishCard(menuItem, m_dishContainer);

// 连接它的信号
connect(card, &DishCard::addClicked, this, &order_system::onAddDish);

// 加到布局中
m_dishListLayout->addWidget(card);
```

---

## 第八部分：Qt 样式表（QSS）

### 8.1 它是什么

Qt Style Sheets（QSS）的语法**几乎和 CSS 一模一样**。如果你知道一点网页的 CSS，上手会非常快。

### 8.2 基本语法

```cpp
widget->setStyleSheet(R"(
    选择器 {
        属性: 值;
        属性: 值;
    }
)");
```

`R"(...)"` 是 C++11 的**原始字符串**，可以写多行，不需要转义引号。

### 8.3 选择器类型（对着代码找）

| 选择器 | 写法 | 项目中实例 |
|--------|------|-----------|
| 类型选择器 | `QFrame { ... }` | 选中所有 QFrame |
| ID 选择器 | `#dishCard { ... }` | 选中 objectName 为 "dishCard" 的那个控件 |
| 伪状态 | `QPushButton:hover { ... }` | 鼠标悬停时的按钮 |

### 8.4 项目中的实际例子

#### 卡片样式（[dish_card.cpp](ui/src/dish_card.cpp)）

```cpp
setStyleSheet(R"(
    #dishCard {
        background: #FFFFFF;       /* 白色背景 */
        border-radius: 10px;       /* 圆角 */
        border: 1px solid #F0F0F0; /* 浅灰边框 */
    }
    #dishCard:hover {
        border: 1px solid #FFD666; /* 悬停时边框变黄 */
        background: #FFFEFA;       /* 悬停时背景微黄 */
    }
)");
```

#### 按钮样式

```cpp
m_addBtn->setStyleSheet(R"(
    QPushButton {
        background: #FFC300;       /* 美团黄 */
        color: #FFFFFF;            /* 白色文字 */
        border: none;              /* 无边框 */
        border-radius: 15px;       /* 圆形（宽高 30，圆角 15） */
        font-size: 20px;
        font-weight: bold;
    }
    QPushButton:hover  { background: #FFB800; }  /* 悬停深一点 */
    QPushButton:pressed { background: #E5A800; } /* 按下更深 */
)");
```

#### 分类列表样式（[order_system.cpp](ui/src/order_system.cpp)）

```cpp
m_categoryList->setStyleSheet(R"(
    QListWidget {
        background: #FFFFFF;
        border: none;
        font-size: 14px;
    }
    QListWidget::item {
        padding: 14px 22px;
        color: #666666;
    }
    QListWidget::item:selected {
        color: #FFC300;               /* 选中变黄 */
        font-weight: bold;
        background: #FFF9E6;          /* 选中背景浅黄 */
        border-left: 3px solid #FFC300; /* 左侧黄色指示条 */
    }
    QListWidget::item:hover {
        color: #FFC300;               /* 悬停也变黄 */
    }
)");
```

### 8.5 常用属性速查

| 属性 | 说明 | 示例 |
|------|------|------|
| `background` / `background-color` | 背景色 | `#FFFFFF` |
| `color` | 文字颜色 | `#333333` |
| `font-size` | 字号 | `16px` |
| `font-weight` | 粗细 | `bold` / `600` |
| `border` | 边框 | `1px solid #EEE` |
| `border-radius` | 圆角 | `10px` |
| `padding` | 内边距 | `14px 22px` |
| `margin` | 外边距 | `8px` |

---

## 第九部分：文件读写与数据模型

### 9.1 数据结构定义

打开 [include/FileManager.h](include/FileManager.h)：

```cpp
struct MenuItem {
    int     id;           // 编号
    QString name;         // 菜名 —— QString 是 Qt 的字符串类
    double  price;        // 价格
    QString description;  // 说明
    int     sales;        // 销量
    double  rating;       // 评分
    QString category;     // 分类
};
```

**QString vs std::string**：
- Qt 里**永远用 `QString`**，它原生支持 Unicode（中文），和所有 Qt API 无缝对接
- 转换：`QString s = QString::fromStdString(stdStr);`

### 9.2 文件读取

打开 [storage/FileManager.cpp](storage/FileManager.cpp)：

```cpp
QList<MenuItem> FileManager::loadMenu(const QString &filePath)
{
    QList<MenuItem> items;

    // ① 打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return items;
    }

    // ② 用 QTextStream 读取
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();  // 读一行，去掉首尾空白

        if (line.isEmpty()) continue;  // 跳过空行

        // ③ 按空格拆分
        QStringList tokens = line.split(
            QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        // ④ 逐字段解析
        MenuItem item;
        item.id = tokens[0].toInt();
        item.name = tokens[1];
        item.price = tokens[2].toDouble();
        // ... 更多解析逻辑

        items.append(item);  // QList 类似 std::vector
    }

    file.close();  // 用完了关掉
    return items;
}
```

### 9.3 Qt 容器类

| Qt 容器 | 等价 STL | 说明 |
|---------|---------|------|
| `QList<T>` | `std::vector<T>` | 动态数组，最常用 |
| `QStringList` | `std::vector<QString>` | 字符串列表 |
| `QMap<K,V>` | `std::map<K,V>` | 有序字典 |
| `QHash<K,V>` | `std::unordered_map<K,V>` | 哈希字典 |

### 9.4 调试输出

```cpp
#include <QDebug>

qDebug() << "调试信息:" << value;
qWarning() << "警告:" << errorMsg;
qInfo() << "提示信息";
```

比 `std::cout` 方便，自动带换行和类型转换。

---

## 第十部分：对照作业需求分析

回顾 [requirement.md](requirement.md) 的七个功能：

### ✅ 已实现的基础

| 需求 | 当前状态 | 对应文件 |
|------|---------|---------|
| **功能七：系统界面** | ✅ 美团风格菜品展示窗口 | order_system.cpp, dish_card.cpp |
| **功能一：菜单创建(分类)** | ✅ 基础架构就绪 | FileManager 可解析菜单文件 |

### 🔨 需要你继续开发的功能

#### 功能一（完整版）：菜单创建与管理

你需要添加：
- 一个"添加菜品"的对话框（QDialog / QInputDialog）
- 把新菜品写入 menu.txt（QFile::WriteOnly 模式）
- 表单验证（价格必须是数字等）

**涉及的 Qt 知识**：QDialog、QFormLayout、QLineEdit、QFile 写模式

#### 功能二：价格计算与会员折扣

你需要添加：
- 购物车（QList 存储已选菜品）
- 底部显示总价
- 会员等级系统（普通/银卡/金卡，不同折扣）

**涉及的 Qt 知识**：数据模型、信号/槽传数据、底部浮动栏

#### 功能三：点餐评价

你需要添加：
- 评价对话框（评分 + 文字评论）
- 评价列表（显示在某处）
- 排序（按评分高低、时间先后）

**涉及的 Qt 知识**：QDialog、QComboBox（下拉选择排序方式）、QDateTime

#### 功能四：推荐点餐

你需要添加：
- 统计 sales 和 rating 最高的 5 道菜
- 在菜单中给它们特殊标记（如"🔥 热销"徽章）
- DishCard 的样式需要支持"推荐"状态

**涉及的 Qt 知识**：排序算法、条件样式

#### 功能五：点餐记忆

你需要添加：
- 历史订单存储（QFile 写 JSON 或用 SQLite）
- "再来一单"按钮
- 从历史订单快速复制

**涉及的 Qt 知识**：QJsonDocument、QSqlDatabase

#### 功能六：排队显示

你需要添加：
- 排队界面（可能是另一个 Tab 或窗口）
- QTimer 定时刷新（模拟实时更新）
- QTableWidget 或自定义列表显示排队进度

**涉及的 Qt 知识**：QTimer、QTabWidget、QTableWidget

---

## 附录：常用快捷键和调试技巧

### 命名约定（项目遵循的）

| 前缀 | 含义 | 示例 |
|------|------|------|
| `m_` | 成员变量 | `m_nameLabel` |
| 无前缀 | 局部变量 | `auto *card = ...` |
| 首字母大写 | 类名 | `DishCard` |
| 首字母小写 | 函数/变量 | `setupUI()` |

### 常见错误排查

| 现象 | 可能原因 |
|------|---------|
| 编译报 `undefined reference to vtable` | 类有 `Q_OBJECT` 但没被 MOC 处理，检查 CMake 中 `CMAKE_AUTOMOC ON` |
| 控件不显示 | ① 没加到布局 ② 父对象不对 ③ `show()` 没调 |
| 信号不触发 | `connect` 的参数类型不匹配 |
| 中文乱码 | 文件编码不是 UTF-8 |
| 窗口布局错乱 | Layout 的 stretch 参数设置不对 |

### 下一步学习路线

1. **跑起来** → 先确保项目能编译运行，改改样式参数看效果
2. **加功能** → 从功能一（添加菜品）开始，涉及 QDialog 和文件写入
3. **学 QDialog** → 模态对话框、表单布局、数据验证
4. **学数据持久化** → QJsonDocument 或 SQLite 存订单数据
5. **完成作业** → 逐个实现七个功能

---

> **最重要的建议**：不要只看，一定要动手改。试着把卡片的背景色改一下，把按钮的圆角调大一点，在分类列表里加一个新分类——每次成功的改动都会加深理解。
