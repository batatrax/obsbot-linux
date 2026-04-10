#pragma once
#include <QWidget>
class QPushButton; class QLabel; class QLineEdit; class QFrame;

class PresetWidget : public QWidget {
    Q_OBJECT
public:
    explicit PresetWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
    void refresh();
private slots:
    void onSave(int id);
    void onLoad(int id);
    void onDelete(int id);
    void onRename(int id);
    void onSetBoot(int id);
private:
    void buildUi();
    QFrame* buildPresetCard(int id);
    QLabel  *m_nameLbl[3];
    QPushButton *m_loadBtn[3], *m_saveBtn[3], *m_delBtn[3];
};
