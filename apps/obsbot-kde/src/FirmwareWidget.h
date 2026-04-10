#pragma once
#include <QWidget>
class QLabel; class QPushButton; class QProgressBar;
class FirmwareWidget : public QWidget {
    Q_OBJECT
public:
    explicit FirmwareWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
private slots:
    void onBrowse();
    void onProgress(int percent, const QString &msg);
    void onFinished(bool ok, const QString &msg);
private:
    void buildUi();
    QLabel *m_versionLbl, *m_statusLbl, *m_fileLbl;
    QPushButton *m_browseBtn, *m_upgradeBtn;
    QProgressBar *m_progress;
    QString m_selectedFile;
};
