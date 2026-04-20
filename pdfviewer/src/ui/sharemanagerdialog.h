/*
    LANShare - LAN file transfer.
    Copyright (C) 2024

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#ifndef SHAREMANAGERDIALOG_H
#define SHAREMANAGERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include "../share/sharedfilemanager.h"

namespace Ui {
class ShareManagerDialog;
}

class ShareManagerDialog : public QDialog {
    Q_OBJECT

private:
    Ui::ShareManagerDialog *ui;
    SharedFileManager* mShareManager;
    QString mCurrentShareId;

public:
    explicit ShareManagerDialog(QWidget *parent = nullptr);
    ~ShareManagerDialog();

private Q_SLOTS:
    void onAddShareClicked();
    void onRemoveShareClicked();
    void onEditShareClicked();
    void onRefreshClicked();
    void onShareSelectionChanged();
    void onActiveCheckBoxToggled(bool checked);
    void onPasswordCheckBoxToggled(bool checked);
    void updateShareList();
    void showShareDetails(const QString& shareId);

private:
    void setupUI();
    void connectSignals();
    void clearDetails();
};

#endif // SHAREMANAGERDIALOG_H