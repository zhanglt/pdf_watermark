/**
 * @file WatermarkController.cpp
 * @brief 水印控制器实现
 */

#include "WatermarkController.h"
#include "ui_mainwindow.h"
#include "../../include/mark/watermarkThreadSingle.h"
#include "../../include/mark/multiWatermarkThreadSingle.h"
#include "../../include/mark/mark.h"
#include "../../include/textedit/CustomTextEdit.h"
#include <QMessageBox>
#include <QColorDialog>
#include <QFileInfo>
#include <QElapsedTimer>

/**
 * @brief 构造函数
 */
WatermarkController::WatermarkController(Ui::MainWindow *ui, QThreadPool &threadPool, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
    , m_threadPool(threadPool)
    , m_wmThreadSingle(nullptr)
    , m_mwmThreadSingle(nullptr)
{
}

/**
 * @brief 析构函数
 */
WatermarkController::~WatermarkController()
{
}

/**
 * @brief 初始化控制器
 */
void WatermarkController::initialize()
{
    // 连接信号槽
    connectSignals();
    
    // 初始状态下导出PDF按钮不可用
    m_ui->btnExportPDF->setEnabled(false);
}

/**
 * @brief 连接信号槽
 */
void WatermarkController::connectSignals()
{
    // 文本变动时更新预览
    connect(m_ui->lineEditWaterText, &CustomTextEdit::dataChanged, 
            this, &WatermarkController::viewWatermark);
    
    // 滑块改变时更新预览
    connect(m_ui->sliderOpacity, &QSlider::sliderReleased, 
            this, &WatermarkController::viewWatermark);
    connect(m_ui->sliderRotate, &QSlider::sliderReleased, 
            this, &WatermarkController::viewWatermark);
    connect(m_ui->sliderFontsize, &QSlider::sliderReleased, 
            this, &WatermarkController::viewWatermark);
    
    // 更新数值显示
    connect(m_ui->sliderRotate, &QSlider::valueChanged, [this]() {
        m_ui->lineEditRotate->setText(QString::number(m_ui->sliderRotate->value()));
    });
    connect(m_ui->sliderOpacity, &QSlider::valueChanged, [this]() {
        m_ui->lineEditOpacity->setText(QString::number(m_ui->sliderOpacity->value()));
    });
    connect(m_ui->sliderFontsize, &QSlider::valueChanged, [this]() {
        m_ui->lineEdit_fs->setText(QString::number(m_ui->sliderFontsize->value()));
    });
}

/**
 * @brief 验证水印输入参数
 */
bool WatermarkController::validateWatermarkInput()
{
    // 验证水印文本
    QString text = m_ui->lineEditWaterText->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(nullptr, "提示", "水印文本不能为空");
        return false;
    }
    
    // 验证输入目录
    QString inputDir = m_ui->lineEditInput->text();
    QFileInfo dir(inputDir);
    if (inputDir.isEmpty() || !dir.exists()) {
        QMessageBox::information(nullptr, "提示", "输入目录或者选择正确目录");
        m_ui->lineEditInput->setFocus();
        m_ui->lineEditInput->setText("");
        return false;
    }
    
    // 验证输出目录
    QString outputDir = m_ui->lineEditOutput->text();
    QFileInfo output(outputDir);
    if (outputDir.isEmpty() || !output.isDir()) {
        QMessageBox::information(nullptr, "提示", "输入目录或者选择正确目录");
        m_ui->lineEditOutput->setFocus();
        m_ui->lineEditOutput->setText("");
        return false;
    }
    
    return true;
}

/**
 * @brief 获取当前选择的字体
 */
QString WatermarkController::getCurrentFont() const
{
    int fontIndex = m_ui->cBoxFont->currentIndex();
    QString font;
    
    switch (fontIndex) {
        case 0:
            font = "NSimSun";
            break;
        case 1:
            font = "simkai";
            break;
        case 2:
            font = "simfang";
            break;
        case 3:
            font = "simhei";
            break;
        default:
            font = "simkai";
    }
    
    return font;
}

/**
 * @brief 水印预览功能
 */
void WatermarkController::viewWatermark()
{
    QElapsedTimer time;
    time.start();
    
    QString text = m_ui->lineEditWaterText->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(nullptr, "提示", "水印文本不能为空");
        return;
    }
    
    QString color = m_ui->lineEditColor->text();
    QString opacity = m_ui->lineEditOpacity->text() + "%";
    QString rotate = m_ui->lineEditRotate->text();
    QString font = getCurrentFont();
    
    // 多行文本处理
    if (m_ui->lineEditWaterText->document()->blockCount() > 1) {
        addWatermark_multiline(
            "doc/1.pdf", "doc/2.pdf", m_ui->lineEditWaterText->toPlainText(),
            m_ui->cBoxFont->currentText(), m_ui->lineEdit_fs->text().toInt(),
            m_ui->lineEditColor->text(), m_ui->lineEditRotate->text().toDouble(),
            m_ui->lineEditOpacity->text().toDouble() / 100);
        
        // 由于多行文本用create_annotation处理，在预览中不显示，所以做转换处理
        pdf2image("doc/2.pdf", "doc/", 72);
        image2pdf("doc/0.png", "doc/2.pdf");
    } else {
        // 单行文本处理
        addWatermark("doc/1.pdf", "doc/2.pdf", text, opacity, color, "-" + rotate, font);
    }
    
    // 发送信号请求打开预览文件
    emit requestOpenPdf(QUrl::fromLocalFile("doc/2.pdf"));
    emit requestFitToWidth();
}

/**
 * @brief 添加水印按钮点击处理
 */
void WatermarkController::onBtnAddWaterClicked()
{
    if (!validateWatermarkInput()) {
        return;
    }
    
    QString text = m_ui->lineEditWaterText->toPlainText();
    QString inputDir = m_ui->lineEditInput->text();
    QString outputDir = m_ui->lineEditOutput->text();
    QString color = m_ui->lineEditColor->text();
    QString opacity = m_ui->lineEditOpacity->text();
    QString rotate = m_ui->lineEditRotate->text();
    QString font = getCurrentFont();
    QString fontsize = m_ui->lineEdit_fs->text();
    
    QFileInfo dir(inputDir);
    QElapsedTimer time;
    time.start();
    
    // 多行水印文本处理
    if (m_ui->lineEditWaterText->document()->blockCount() > 1) {
        if (dir.isFile() && dir.suffix().toLower() == "pdf") {
            addWatermark_multiline(
                inputDir.toStdString(), outputDir.toStdString() + "/_out_/", text,
                font, fontsize.toInt(), color, rotate.toDouble(), opacity.toDouble());
        } else {
            addWatermarkSingle(text, inputDir, outputDir, color, opacity, rotate,
                             m_ui->cBoxFont->currentText(), fontsize);
        }
    } else {
        // 单行水印文本处理
        if (dir.isFile() && dir.suffix().toLower() == "pdf") {
            addWatermark(inputDir, outputDir + "/_out_/" + dir.fileName(), text,
                       opacity + "%", color, "-" + rotate, font);
        } else {
            addWatermarkSingle(text, inputDir, outputDir, color, opacity,
                             "-" + rotate, font, fontsize);
        }
    }
    
    QString message = QString("增加水印用时:%1毫秒,结果文件保存在:%2/_out_/")
                      .arg(time.elapsed())
                      .arg(m_ui->lineEditOutput->text());
    
    m_ui->textEditLog->append(message);
    emit exportButtonStateChanged(true);
}

/**
 * @brief 颜色选择按钮点击处理
 */
void WatermarkController::onBtnColorSelectClicked()
{
    QColorDialog dialog(Qt::red);
    dialog.setOption(QColorDialog::ShowAlphaChannel);
    dialog.exec();
    
    QColor color = dialog.currentColor();
    if (color.isValid()) {
        // 提取RGB颜色分量
        int m_red = color.red();
        int m_blue = color.blue();
        int m_green = color.green();
        
        // 转换为16进制格式
        QString rStr = QString("%1").arg(m_red & 0xFF, 2, 16, QChar('0'));
        QString bStr = QString("%1").arg(m_blue & 0xFF, 2, 16, QChar('0'));
        QString gStr = QString("%1").arg(m_green & 0xFF, 2, 16, QChar('0'));
        
        QString colorCode = "#" + rStr + gStr + bStr;
        m_ui->lineEditColor->setText(colorCode);
        m_ui->lineEditColor->setStyleSheet("color:" + colorCode);
        
        viewWatermark();
    }
}

/**
 * @brief 字体选择变更处理
 */
void WatermarkController::onFontChanged()
{
    viewWatermark();
}

/**
 * @brief 批量添加水印
 */
void WatermarkController::addWatermarkSingle(QString text, QString inputDir,
                                            QString outputDir, QString color,
                                            QString opacity, QString rotate,
                                            QString font, QString fontSize)
{
    QDir dir(inputDir);
    QStringList files;
    traverseDirectory(dir, files, "pdf", "_out_");
    wMap map;
    
    if (files.length() == 0) {
        QMessageBox::information(nullptr, "提示！", "输入目录中文件为空");
        return;
    }
    
    for (const auto &file : files) {
        QString outfile = pathChange(inputDir, outputDir, file, "_out_").replace("//", "/");
        
        if (m_ui->lineEditWaterText->document()->blockCount() > 1) {
            m_mwmThreadSingle = new multiWatermarkThreadSingle(&m_mutex, &map);
            m_mwmThreadSingle->setText(text);
            m_mwmThreadSingle->setFont(font);
            m_mwmThreadSingle->setColor(color);
            m_mwmThreadSingle->setRotate(rotate);
            m_mwmThreadSingle->setOpacity(opacity);
            m_mwmThreadSingle->setFontsize(fontSize);
            m_mwmThreadSingle->setInputFilename(file);
            m_mwmThreadSingle->setOutputFilename(outfile);
            m_threadPool.start(m_mwmThreadSingle);
        } else {
            m_wmThreadSingle = new watermarkThreadSingle(&m_mutex, &map);
            m_wmThreadSingle->setText(text);
            m_wmThreadSingle->setFont(font);
            m_wmThreadSingle->setColor(color);
            m_wmThreadSingle->setRotate(rotate);
            m_wmThreadSingle->setOpacity(opacity);
            m_wmThreadSingle->setFontsize(fontSize);
            m_wmThreadSingle->setInputFilename(file);
            m_wmThreadSingle->setOutputFilename(outfile);
            m_threadPool.start(m_wmThreadSingle);
        }
    }
    
    m_threadPool.waitForDone();
    
    QString message = QString("增加水印完成！<br>共%1个文件参与处理，<br>实际处理文件%2个")
                      .arg(files.length())
                      .arg(map.size());
    
    QMessageBox::information(nullptr, "提示！", message);
    emit watermarkCompleted(message);
}