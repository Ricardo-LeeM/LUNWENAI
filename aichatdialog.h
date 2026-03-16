#ifndef AICHATDIALOG_H
#define AICHATDIALOG_H

#include <QDialog>
#include <QMovie>


// 网络模块
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "usersession.h"

namespace Ui {
class AiChatDialog;
}

class AiChatDialog : public QDialog
{
    Q_OBJECT

public:
    // 修改构造函数，增加 isAdmin 参数，默认为 false
    explicit AiChatDialog(bool isAdmin = false, QWidget *parent = nullptr);
    ~AiChatDialog();

private slots:
    void on_btnSend_clicked();
    void on_btnClear_clicked();
    void on_btnExport_clicked();
    void onQuickPromptClicked(); // 处理快捷指令点击

    // === 处理网络返回数据的槽函数 ===
    void onReplyFinished(QNetworkReply *reply);

private:
    Ui::AiChatDialog *ui;
    bool m_isAdmin;          // 记录当前权限状态
    QMovie *m_loadingMovie;  // 用于播放加载动画

    // === 网络管理器与 API Key ===
    QNetworkAccessManager *m_networkManager;
    QString m_apiKey;
    void executeSqlAndShowResult(const QString &sql); // 执行SQL并渲染表格

    void setupUI();          // 初始化 UI 和 样式
    void setWelcomeMessage();// 设置欢迎语
    void showLoading(bool show); // 控制加载动画的显示与隐藏
    void appendChatMessage(const QString &text, bool isMine); // 气泡渲染逻辑
};

#endif // AICHATDIALOG_H
