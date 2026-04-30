#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <functional>
#include "SaveFile.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openFile();
    void saveFile();
    void convertToRepPlus();

private:
    SaveFile currentSave;
    std::string activeFilePath;

    QTabWidget* tabWidget;
    
    // UI Builders
    QWidget* createStatsTab();
    QWidget* createChallengesTab();
    QWidget* createGridTab(int count, int cols,
                           std::function<QString(int)> iconPathProvider,
                           std::function<QString(int)> textProvider,
                           std::function<bool(int)> getter, 
                           std::function<void(int, bool)> setter);

    void refreshUI();

    // Stats Spinboxes
    QSpinBox* spinDonation;
    QSpinBox* spinGreed;
    QSpinBox* spinEden;
    QSpinBox* spinDeaths;
    QSpinBox* spinMomKills;
};