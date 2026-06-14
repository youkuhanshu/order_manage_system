#include "auth_page.h"
#include "FileManager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>

QFrame *makeCard(QWidget *parent, int height)
{
    auto *card = new QFrame(parent);
    card->setFixedSize(400, height);
    card->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px;"
        "border: 1px solid #E8E8E8; }");
    return card;
}

QLineEdit *makeInput(QWidget *parent, const QString &placeholder, bool password)
{
    auto *edit = new QLineEdit(parent);
    edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(40);
    edit->setStyleSheet(
        R"(
            QLineEdit {
                border: 1px solid #E0E0E0; border-radius: 6px;
                padding: 0 12px; font-size: 14px; color: #333333;
                background: #FAFAFA;
            }
            QLineEdit:focus { border-color: #0085FF; background: #FFFFFF; }
        )"
    );
    if (password) edit->setEchoMode(QLineEdit::Password);
    return edit;
}

//  登录页
LoginPage::LoginPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LoginPage::setupUI()
{
    setStyleSheet("background: #F5F5F5;");

    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setAlignment(Qt::AlignCenter);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    auto *card = makeCard(this, 420);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setContentsMargins(50, 40, 50, 40);
    cardLayout->setSpacing(16);

    auto *appTitle = new QLabel("饱了么", card);
    appTitle->setAlignment(Qt::AlignCenter);
    appTitle->setStyleSheet(
        "font-size: 30px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;"
    );
    cardLayout->addWidget(appTitle);

    cardLayout->addSpacing(10);

    auto *pageTitle = new QLabel("登录", card);
    pageTitle->setAlignment(Qt::AlignCenter);
    pageTitle->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;"
    );
    cardLayout->addWidget(pageTitle);

    cardLayout->addSpacing(20);

    m_nameEdit = makeInput(card, "请输入用户名", false);
    cardLayout->addWidget(m_nameEdit);

    m_pwdEdit = makeInput(card, "请输入密码", true);
    cardLayout->addWidget(m_pwdEdit);

    cardLayout->addSpacing(4);

    auto *loginBtn = new QPushButton("登  录", card);
    loginBtn->setFixedHeight(42);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(
        R"(
            QPushButton {
                background: #0085FF; color: #FFFFFF; border: none;
                border-radius: 6px; font-size: 16px; font-weight: bold;
            }
            QPushButton:hover  { background: #0073E6; }
            QPushButton:pressed { background: #0060BF; }
        )"
    );
    cardLayout->addWidget(loginBtn);

    auto *toRegBtn = new QPushButton("没有账号？立即注册", card);
    toRegBtn->setCursor(Qt::PointingHandCursor);
    toRegBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 12px; color: #999999; }"
        "QPushButton:hover { color: #0085FF; }"
    );
    cardLayout->addWidget(toRegBtn);

    connect(loginBtn, &QPushButton::clicked, this, [this]() {
        emit loginClicked(m_nameEdit->text(), m_pwdEdit->text());
    });
    connect(toRegBtn, &QPushButton::clicked, this, [this]() {
        emit toRegisterClicked();
    });

    pageLayout->addWidget(card);
}

//  注册页
RegisterPage::RegisterPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void RegisterPage::setupUI()
{
    setStyleSheet("background: #F5F5F5;");

    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setAlignment(Qt::AlignCenter);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    auto *card = makeCard(this, 480);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setContentsMargins(50, 36, 50, 36);
    cardLayout->setSpacing(14);

    auto *icon = new QLabel(QString::fromUtf8("🍽"), card);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("font-size: 36px; border: none; background: transparent;");
    cardLayout->addWidget(icon);

    auto *pageTitle = new QLabel("注册", card);
    pageTitle->setAlignment(Qt::AlignCenter);
    pageTitle->setStyleSheet(
        "font-size: 30px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;"
    );
    cardLayout->addWidget(pageTitle);

    cardLayout->addSpacing(6);

    m_nameEdit = makeInput(card, "请输入用户名", false);
    cardLayout->addWidget(m_nameEdit);

    m_pwdEdit = makeInput(card, "请设置密码", true);
    cardLayout->addWidget(m_pwdEdit);

    m_pwdConfirmEdit = makeInput(card, "请再次输入密码", true);
    cardLayout->addWidget(m_pwdConfirmEdit);

    cardLayout->addSpacing(2);

    auto *regBtn = new QPushButton("注  册", card);
    regBtn->setFixedHeight(42);
    regBtn->setCursor(Qt::PointingHandCursor);
    regBtn->setStyleSheet(
        R"(
            QPushButton {
                background: #0085FF; color: #FFFFFF; border: none;
                border-radius: 6px; font-size: 16px; font-weight: bold;
            }
            QPushButton:hover  { background: #0073E6; }
            QPushButton:pressed { background: #0060BF; }
        )"
    );
    cardLayout->addWidget(regBtn);

    auto *toLoginBtn = new QPushButton("已有账号？立即登录", card);
    toLoginBtn->setCursor(Qt::PointingHandCursor);
    toLoginBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 12px; color: #999999; }"
        "QPushButton:hover { color: #0085FF; }"
    );
    cardLayout->addWidget(toLoginBtn);

    connect(regBtn, &QPushButton::clicked, this, [this]() {
        FileManager fl;
        fl.LoadUsers();
        std::vector<User> u = fl.getUsers_cpp();
        for (size_t i = 0;i < u.size();i++) {
            if (m_nameEdit->text() == u[i].name) {
                QMessageBox::warning(this, "注册失败", "该用户已存在！");
                return;
            }
        }
        if (m_pwdEdit->text() != m_pwdConfirmEdit->text()) {
            QMessageBox::warning(this, "注册失败", "两次输入的密码不相同！");
        } 
        else {
            emit registerClicked(m_nameEdit->text(), m_pwdEdit->text());
        }
    });
    connect(toLoginBtn, &QPushButton::clicked, this, [this]() {
        emit toLoginClicked();
    });

    pageLayout->addWidget(card);
}
