#include "aichatdialog.h"
#include "ui_aichatdialog.h"
#include <QDateTime>
#include <QTimer>

AiChatDialog::AiChatDialog(bool isAdmin, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AiChatDialog),
    m_isAdmin(isAdmin)
{
    ui->setupUi(this);

    // 设置窗口基本属性
    this->setWindowTitle("AI 智能数据助手");
    this->resize(700, 600); // 给个合适的初始大小



    // === 初始化网络通信 ===
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AiChatDialog::onReplyFinished);

    // API Key
    m_apiKey = "050e2a0d026f4d4a9d139de49bcd6c71.YlK3LcE0eMesJONp";




    setupUI();
    setWelcomeMessage();

    // 默认隐藏加载动画
    showLoading(false);
    // 1. 设置最小尺寸，防止用户把窗口缩得太小导致控件挤在一起
    this->setMinimumSize(1167, 757);


}

AiChatDialog::~AiChatDialog()
{
    delete ui;
}

void AiChatDialog::setupUI()
{
    // ================= 0. 强制接管全局样式，阻断父窗口污染 =================
    // 给对话框自身设定一个绝对的基准字体和背景色
    this->setStyleSheet(
        "QWidget {"
        "   font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif;"
        "   font-size: 14px;"
        "   background-color: #F9FAFB;"
        "   color: #1F2937;"
        "}"
        "QTextBrowser {"
        "   font-size: 14px;"
        "   background-color: transparent;"
        "}"
        "QTextEdit {"
        "   border: 2px solid #D1D5DB;"
        "   border-radius: 6px;"
        "   padding: 5px;"
        "   background-color: #FFFFFF;"
        "}"
        "QTextEdit:focus {"
        "   border: 2px solid #10B981;"
        "}"
        );

    // ================= 1. 权限与状态徽章 (Status Badge) =================
    if (m_isAdmin) {
        ui->labStatusBadge->setText(" 🛡️ 全局管理模式 ");
        ui->labStatusBadge->setStyleSheet("background-color: #E1EFFE; color: #1E429F; border-radius: 12px; padding: 4px 8px; font-weight: bold;");

        // ================= 2. 快捷提问指令区 (Quick Prompts) - 管理员版 =================
        ui->btnPrompt1->setText("统计各部门平均薪资");
        ui->btnPrompt2->setText("本月迟到人员名单");
        ui->btnPrompt3->setText("谁的绩效最高？");
    } else {
        ui->labStatusBadge->setText(" 👤 个人自助模式 ");
        ui->labStatusBadge->setStyleSheet("background-color: #DEF7EC; color: #03543F; border-radius: 12px; padding: 4px 8px; font-weight: bold;");

        // ================= 2. 快捷提问指令区 (Quick Prompts) - 员工版 =================
        ui->btnPrompt1->setText("查一下我上个月的扣款");
        ui->btnPrompt2->setText("我的剩余休假天数");
        ui->btnPrompt3->setText("我的入职日期是什么时候");
    }

    // 绑定快捷指令按钮的点击事件到同一个槽函数
    connect(ui->btnPrompt1, &QPushButton::clicked, this, &AiChatDialog::onQuickPromptClicked);
    connect(ui->btnPrompt2, &QPushButton::clicked, this, &AiChatDialog::onQuickPromptClicked);
    connect(ui->btnPrompt3, &QPushButton::clicked, this, &AiChatDialog::onQuickPromptClicked);

    // 快捷按钮的统一薄荷风样式
    QString promptStyle = "QPushButton { background-color: #FFFFFF; border: 1px solid #10B981; color: #10B981; border-radius: 10px; padding: 5px 10px; }"
                          "QPushButton:hover { background-color: #D1FAE5; }";
    ui->btnPrompt1->setStyleSheet(promptStyle);
    ui->btnPrompt2->setStyleSheet(promptStyle);
    ui->btnPrompt3->setStyleSheet(promptStyle);

    // ================= 3. “思考中...” 动态加载动画 =================
    // 请确保你的项目资源文件(respic.qrc)中确切存在 move.gif
    m_loadingMovie = new QMovie(":/C:/images/move.gif");
    m_loadingMovie->setScaledSize(QSize(20, 20)); // 缩小一点适配文字
    ui->labLoading->setMovie(m_loadingMovie);
    ui->labLoading->setStyleSheet("color: #6B7280; font-size: 12px;");

    // ================= 4. 会话工具栏 (Chat Toolbar) 样式 =================
    ui->btnClear->setText("🧹 清空对话");
    ui->btnExport->setText("💾 导出记录");
    QString toolBtnStyle = "QToolButton { border: none; color: #6B7280; } QToolButton:hover { color: #10B981; }";
    ui->btnClear->setStyleSheet(toolBtnStyle);
    ui->btnExport->setStyleSheet(toolBtnStyle);
}

// ================= 5. 欢迎引导语 (Welcome Message) =================
void AiChatDialog::setWelcomeMessage()
{
    QString welcomeHtml = QString(
        "<div style='background-color: #F3F4F6; padding: 15px; border-radius: 8px; margin-bottom: 10px;'>"
        "<h3 style='color: #10B981; margin-top: 0;'>🤖 智慧 HR 数据助理 已就绪</h3>"
        "<p style='color: #374151;'>你好！我是基于大语言模型的数据库助手。</p>"
        "<p style='color: #6B7280; font-size: 12px;'>你可以直接用自然语言向我提问，我会自动转换为 SQL 语句为你提取精确数据。<br>"
        "<i>尝试点击下方的快捷指令开始吧！</i></p>"
        "</div>"
        );
    ui->textBrowserChat->setHtml(welcomeHtml);
}

void AiChatDialog::onQuickPromptClicked()
{
    // 获取被点击的按钮
    QPushButton *clickedBtn = qobject_cast<QPushButton *>(sender());
    if (clickedBtn) {
        // 将按钮上的文字直接填入输入框
        ui->textEditInput->setText(clickedBtn->text());
        // 可选：填入后自动点击发送
        // on_btnSend_clicked();
    }
}

void AiChatDialog::showLoading(bool show)
{
    if (show) {
        ui->labLoading->setText(" AI 正在检索数据库并生成回复...");
        ui->labLoading->show();
        m_loadingMovie->start();
        ui->btnSend->setEnabled(false); // 防止重复点击
    } else {
        ui->labLoading->hide();
        m_loadingMovie->stop();
        ui->btnSend->setEnabled(true);
    }
}

void AiChatDialog::on_btnClear_clicked()
{
    ui->textBrowserChat->clear();
    setWelcomeMessage(); // 清空后重新显示欢迎语
}

void AiChatDialog::on_btnExport_clicked()
{
    // 留作扩展：你可以调用 QFile 将 textBrowserChat->toPlainText() 写入 txt 文件
}

// void AiChatDialog::on_btnSend_clicked()
// {
//     QString content = ui->textEditInput->toPlainText().trimmed();
//     if (content.isEmpty()) return;

//     // 1. 显示用户自己的气泡 (复用你之前的 appendChatMessage 逻辑，这里略)
//     // appendChatMessage(content, true);
//     ui->textBrowserChat->append("<b>我：</b>" + content); // 临时简单展示

//     // 2. 清空输入框
//     ui->textEditInput->clear();

//     // 3. 开启“思考中”动画
//     showLoading(true);

//     // 4. TODO: 这里将是调用大模型 API 的地方！
//     // 模拟网络延迟，3秒后关闭动画 (之后这部分会被真实网络请求回调替换)
//     QTimer::singleShot(2000, this, [=]() {
//         showLoading(false);
//         // appendChatMessage("我是AI的测试回复", false);
//         ui->textBrowserChat->append("<b style='color:#10B981;'>AI助手：</b> 这是模拟的数据库查询结果。");
//     });
// }


void AiChatDialog::on_btnSend_clicked()
{
    QString content = ui->textEditInput->toPlainText().trimmed();
    if (content.isEmpty()) return;

    // 1. 显示用户的气泡
    ui->textBrowserChat->append("<div style='text-align:right; margin:5px;'><span style='background-color:#D1FAE5; padding:8px; border-radius:6px; color:black;'><b>我：</b>" + content + "</span></div>");
    ui->textEditInput->clear();
    showLoading(true);

    // ================= 核心：构建发给智谱大模型的 JSON =================
    QJsonObject json;
    json["model"] = "glm-4-flash"; // 使用免费且极速的模型
    json["temperature"] = 0.1;     // 温度调低，让 AI 回答更严谨（适合写 SQL）

    QJsonArray messagesArray;

    // 【权限隔离精髓】：通过系统提示词(System Prompt)控制 AI 的权限
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    if (m_isAdmin) {
        systemMessage["content"] = "你是一个HR系统的【超级管理员】AI助手。数据库包含 basicinfo, PayrollRecords, declarations 等表。"
                                   "请根据要求给出准确的 MySQL 查询语句。必须用 ```sql 和 ``` 将代码包裹起来。不要有任何废话。";
    } else {
        // 获取当前真实登录的工号
        int currentEmpId = UserSession::instance()->employeeID();
        systemMessage["content"] = QString("你是一个HR系统的【普通员工】AI自助助手。当前登录员工的工号是 %1。"
                                           "你【绝不可以】查询除了该员工以外的任何人。"
                                           "请给出查询语句，WHERE 条件必须且只能包含 EmployeeID = %1。"
                                           "必须用 ```sql 和 ``` 将代码包裹起来。不要有废话。").arg(currentEmpId);
    }
    messagesArray.append(systemMessage);

    // 填入用户的提问
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = content;
    messagesArray.append(userMessage);

    json["messages"] = messagesArray;
    QJsonDocument doc(json);
    QByteArray postData = doc.toJson();

    // ================= 发送 HTTP POST 请求 =================
    QUrl url("https://open.bigmodel.cn/api/paas/v4/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 设置 Authorization 请求头
    QString authHeader = "Bearer " + m_apiKey;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    // 发起异步请求，完成后会自动触发 onReplyFinished 槽函数
    m_networkManager->post(request, postData);
}


void AiChatDialog::onReplyFinished(QNetworkReply *reply)
{
    showLoading(false); // 关闭思考动画

    if (reply->error() == QNetworkReply::NoError) {
        // 读取返回的数据
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();

        // 提取 AI 回复的文本：解析 choices[0].message.content
        if (jsonObj.contains("choices") && jsonObj["choices"].isArray()) {
            QJsonArray choices = jsonObj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                QJsonObject messageObj = firstChoice["message"].toObject();
                QString aiReply = messageObj["content"].toString();

                // 【修改核心】：复制一份专门用于 UI 显示，避免污染原数据
                QString displayReply = aiReply;
                displayReply.replace("\n", "<br>");

                // 打印在界面上（使用带有 <br> 的副本 displayReply）
                ui->textBrowserChat->append("<div style='margin:5px;'><span style='background-color:#FFFFFF; border:1px solid #D1D5DB; padding:8px; border-radius:6px; color:black;'>🤖 <b>助手：</b><br>" + displayReply + "</span></div>");

                // ================= 提取并执行 SQL =================
                // 【修改核心】：使用原汁原味的 aiReply（包含原本的 \n）来提取 SQL
                QRegularExpression re("```sql\\s*(.*?)\\s*```", QRegularExpression::DotMatchesEverythingOption);
                QRegularExpressionMatch match = re.match(aiReply);

                if (match.hasMatch()) {
                    QString sql = match.captured(1).trimmed(); // 拿到纯净的 SQL 语句
                    executeSqlAndShowResult(sql); // 交给我们的新函数去执行！
                } else {
                    // 如果 AI 没按格式输出，可能只是普通聊天，就不执行查库
                }
            }
        }
    } else {
        // 处理网络错误
        ui->textBrowserChat->append("<div style='color:red;'><b>网络请求失败：</b> " + reply->errorString() + "</div>");
    }

    // 释放 reply 对象
    reply->deleteLater();
}


void AiChatDialog::executeSqlAndShowResult(const QString &sql)
{
    // 1. 安全性检查（物理防线）：绝不允许 AI 执行删除或修改操作！
    QString upperSql = sql.toUpper();
    if (upperSql.contains("DROP ") || upperSql.contains("DELETE ") || upperSql.contains("UPDATE ") || upperSql.contains("INSERT ")) {
        ui->textBrowserChat->append("<div style='color:red; margin:5px;'>⚠️ <b>系统拦截：</b> 出于安全考虑，AI 智能助手仅支持数据查询 (SELECT) 操作！</div>");
        return;
    }

    // 2. 执行查询
    QSqlQuery query;
    if (!query.exec(sql)) {
        ui->textBrowserChat->append("<div style='color:red; margin:5px;'>❌ <b>数据库查询失败：</b> " + query.lastError().text() + "</div>");
        return;
    }

    // 3. 开始构建 HTML 表格
    // 定义表格样式：边框折叠、宽度100%、单元格内边距等
    QString html = "<table border='1' cellspacing='0' cellpadding='5' style='border-collapse: collapse; width: 100%; border-color: #D1D5DB; font-size: 13px; margin-top: 10px;'>";

    // 获取字段信息（表头）
    QSqlRecord record = query.record();
    int colCount = record.count();
    if (colCount == 0) {
        ui->textBrowserChat->append("<div style='margin:5px; color:#6B7280;'><i>✅ 指令已执行，但没有返回任何数据列。</i></div>");
        return;
    }

    // 拼装表头 (使用你的薄荷绿主题色)
    html += "<tr style='background-color: #D1FAE5; color: #064E3B;'>";
    for (int i = 0; i < colCount; ++i) {
        html += "<th>" + record.fieldName(i) + "</th>";
    }
    html += "</tr>";

    // 拼装数据行
    int rowCount = 0;
    while (query.next()) {
        // 斑马线效果：偶数行纯白，奇数行极浅的灰色
        QString bgColor = (rowCount % 2 == 0) ? "#FFFFFF" : "#F9FAFB";
        html += "<tr style='background-color: " + bgColor + ";'>";
        for (int i = 0; i < colCount; ++i) {
            html += "<td>" + query.value(i).toString() + "</td>";
        }
        html += "</tr>";
        rowCount++;
    }
    html += "</table>";

    // 4. 将表格推送到聊天界面
    if (rowCount == 0) {
        ui->textBrowserChat->append("<div style='margin:5px; color:#6B7280;'><i>未找到符合条件的数据记录。</i></div>");
    } else {
        ui->textBrowserChat->append("<div style='margin:5px;'>" + html + "</div>");
    }
}
