#pragma once

#include <QWidget>
#include <QLineEdit>

/// 登录页
class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);

signals:
    void loginClicked(const QString &name, const QString &password);
    void toRegisterClicked();

private:
    void setupUI();
    QLineEdit *m_nameEdit;
    QLineEdit *m_pwdEdit;
};

/// 注册页
class RegisterPage : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterPage(QWidget *parent = nullptr);

signals:
    void registerClicked(const QString &name, const QString &password);
    void toLoginClicked();

private:
    void setupUI();
    QLineEdit *m_nameEdit;
    QLineEdit *m_pwdEdit;
    QLineEdit *m_pwdConfirmEdit;
};
