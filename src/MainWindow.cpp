#include "MainWindow.hpp"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QDir>
#include <QSignalBlocker>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("TBOI Repentance Save Editor (Qt)");
    resize(1024, 768);

    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("Open Save (.dat)", this, &MainWindow::openFile);
    fileMenu->addAction("Save", this, &MainWindow::saveFile);
    fileMenu->addAction("Save As Repentance+ (Online Beta)", this, &MainWindow::convertToRepPlus);

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    tabWidget->addTab(createStatsTab(), "Stats & Misc");
    
    // For now placeholder, implement in the future
    tabWidget->addTab(new QWidget(), "Completion Marks");
    tabWidget->addTab(new QWidget(), "Bestiary");

    tabWidget->addTab(createGridTab(732, 15,
        [](int i) { return QString("assets/gfx/items/collectibles/%1.png").arg(i, 3, 10, QChar('0')); }, 
        [](int i) { return QString::number(i); },
        [this](int id) { return currentSave.getItem(id); }, 
        [this](int id, bool val) { currentSave.setItem(id, val); }), "Items");

    tabWidget->addTab(createGridTab(641, 15,
        [](int i) { return QString("assets/gfx/achievements/%1.png").arg(i); }, 
        [](int i) { return QString::number(i); },
        [this](int id) { return currentSave.getAchievement(id); }, 
        [this](int id, bool val) { currentSave.setAchievement(id, val); }), "Achievements");

    tabWidget->addTab(createChallengesTab(), "Challenges");

    for(int i = 0; i < tabWidget->count(); ++i) {
        tabWidget->setTabEnabled(i, false);
    }
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Save File", QDir::currentPath(), "DAT Files (*.dat)");
    if (!fileName.isEmpty()) {
        if (currentSave.load(fileName.toStdString())) {
            activeFilePath = fileName.toStdString();
            for(int i = 0; i < tabWidget->count(); ++i) {
                if (tabWidget->tabText(i).contains("(TODO)")) {
                     tabWidget->setTabEnabled(i, false);
                } else {
                     tabWidget->setTabEnabled(i, true);
                }
            }
            refreshUI();
            QMessageBox::information(this, "Success", "Loaded Save File Successfully!");
        } else {
            QMessageBox::critical(this, "Error", "Invalid save file header or corrupted file.");
        }
    }
}

void MainWindow::saveFile() {
    if (currentSave.isLoaded()) {
        if (currentSave.save(activeFilePath)) {
            QMessageBox::information(this, "Success", "Saved Successfully");
        }
    }
}

void MainWindow::convertToRepPlus() {
    if (currentSave.isLoaded()) {
        QMessageBox::warning(this, "Steam Cloud Warning", "Ensure Steam Cloud is OFF before replacing rep+ files!");
        QString newPath = QFileDialog::getSaveFileName(this, "Save As Repentance+", QDir::currentPath(), "rep+persistentgamedata*.dat");
        if (!newPath.isEmpty() && currentSave.save(newPath.toStdString())) {
            QMessageBox::information(this, "Success", "Saved as Repentance+ format");
        }
    }
}

QWidget* MainWindow::createStatsTab() {
    QWidget* widget = new QWidget;
    QFormLayout* layout = new QFormLayout(widget);

    spinDonation = new QSpinBox; spinDonation->setMaximum(999);
    spinGreed = new QSpinBox; spinGreed->setMaximum(999);
    spinEden = new QSpinBox; spinEden->setMaximum(999);
    spinDeaths = new QSpinBox; spinDeaths->setMaximum(9999);
    spinMomKills = new QSpinBox; spinMomKills->setMaximum(9999);

    layout->addRow("Donation Machine:", spinDonation);
    layout->addRow("Greed Machine:", spinGreed);
    layout->addRow("Eden Tokens:", spinEden);
    layout->addRow("Total Deaths:", spinDeaths);
    layout->addRow("Mom Kills:", spinMomKills);

    connect(spinDonation, &QSpinBox::valueChanged, [this](int val) { currentSave.setStat(SaveFile::STAT_DONATION, val); });
    connect(spinGreed, &QSpinBox::valueChanged, [this](int val) { currentSave.setStat(SaveFile::STAT_GREED_MACHINE, val); });
    connect(spinEden, &QSpinBox::valueChanged, [this](int val) { currentSave.setStat(SaveFile::STAT_EDEN_TOKENS, val); });
    connect(spinDeaths, &QSpinBox::valueChanged, [this](int val) { currentSave.setStat(SaveFile::STAT_DEATHS, val); });
    connect(spinMomKills, &QSpinBox::valueChanged, [this](int val) { currentSave.setStat(SaveFile::STAT_MOM_KILLS, val); });

    return widget;
}

QWidget* MainWindow::createChallengesTab() {
    QWidget* container = new QWidget;
    QHBoxLayout* mainLayout = new QHBoxLayout(container);

    std::vector<QString> dlcTitles = {"Rebirth", "Afterbirth", "Afterbirth+", "Repentance"};
    std::vector<int> challengesPerDlc = {20, 10, 5, 10};
    
    std::vector<QString> challengeNames = {
        "Pitch Black", "High Brow", "Head Trauma", "Darkness Falls", "The Tank", "Solar System", "Suicide King", "Cat Got Your Tongue", "Demo Man", "Cursed!", "Glass Cannon", "When Life Gives You Lemons", "Beans!", "It's in the Cards", "Slow Roll", "Computer Savvy", "Waka Waka", "The Host", "The Family Man", "Purist",
        "XXXXXXXXL", "SPEED!", "Blue Bomber", "PAY TO PLAY", "Have a Heart", "I RULE!", "BRAINS!", "PRIDE DAY!", "Onan's Streak", "The Guardian",
        "Backasswards", "Aprils Fool", "Pokey Mans", "Ultra Hard", "Pong",
        "Scat Man", "Bloody Mary", "Baptism by Fire", "Isaac's Awakening", "Seeing Double", "Pica Run", "Hot Potato", "Cantripped!", "Red Redemption", "DELETE THIS"
    };

    int challengeIndex = 1;
    for (size_t i = 0; i < dlcTitles.size(); ++i) {
        QGroupBox* groupBox = new QGroupBox(dlcTitles[i]);
        QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);
        
        for (int j = 0; j < challengesPerDlc[i]; ++j) {
            QCheckBox* chk = new QCheckBox(challengeNames[challengeIndex - 1]);
            groupLayout->addWidget(chk);
            
            connect(chk, &QCheckBox::toggled, [this, challengeIndex](bool checked) {
                currentSave.setChallenge(challengeIndex, checked);
            });
            chk->setObjectName(QString("chk_challenge_%1").arg(challengeIndex));
            challengeIndex++;
        }
        groupLayout->addStretch();
        mainLayout->addWidget(groupBox);
    }

    return container;
}

QWidget* MainWindow::createGridTab(int count, int cols,
                                   std::function<QString(int)> iconPathProvider,
                                   std::function<QString(int)> textProvider,
                                   std::function<bool(int)> getter, 
                                   std::function<void(int, bool)> setter) {
    QScrollArea* scroll = new QScrollArea;
    QWidget* container = new QWidget;
    QGridLayout* layout = new QGridLayout(container);
    
    for (int i = 1; i <= count; i++) {
        QCheckBox* chk = new QCheckBox;
        
        QString imgPath = iconPathProvider(i);
        QPixmap pixmap(imgPath);
        
        if(!pixmap.isNull()) {
            chk->setIcon(QIcon(pixmap));
            chk->setIconSize(QSize(48, 48));
            chk->setToolTip(textProvider(i));
        } else {
            chk->setText(textProvider(i));
        }

        layout->addWidget(chk, (i-1) / cols, (i-1) % cols);

        connect(chk, &QCheckBox::toggled, [=](bool checked) {
            setter(i, checked);
        });

        chk->setObjectName(QString("chk_item_%1").arg(i)); 
    }

    container->setLayout(layout);
    scroll->setWidget(container);
    scroll->setWidgetResizable(true);
    return scroll;
}


void MainWindow::refreshUI() {
    if (!currentSave.isLoaded()) return;

    { QSignalBlocker b(spinDonation); spinDonation->setValue(currentSave.getStat(SaveFile::STAT_DONATION)); }
    { QSignalBlocker b(spinGreed); spinGreed->setValue(currentSave.getStat(SaveFile::STAT_GREED_MACHINE)); }
    { QSignalBlocker b(spinEden); spinEden->setValue(currentSave.getStat(SaveFile::STAT_EDEN_TOKENS)); }
    { QSignalBlocker b(spinDeaths); spinDeaths->setValue(currentSave.getStat(SaveFile::STAT_DEATHS)); }
    { QSignalBlocker b(spinMomKills); spinMomKills->setValue(currentSave.getStat(SaveFile::STAT_MOM_KILLS)); }

    auto refreshGrid = [this](int tabIndex, int count, const QString& nameTemplate, std::function<bool(int)> getter) {
        QWidget* tab = tabWidget->widget(tabIndex);
        if (!tab) return;
        for (int i = 1; i <= count; i++) {
            QCheckBox* chk = tab->findChild<QCheckBox*>(nameTemplate.arg(i));
            if (chk) {
                QSignalBlocker blocker(chk);
                chk->setChecked(getter(i));
            }
        }
    };
    
    refreshGrid(3, 732, "chk_item_%1", [this](int id){ return currentSave.getItem(id); });
    refreshGrid(4, 641, "chk_item_%1", [this](int id){ return currentSave.getAchievement(id); });
    refreshGrid(5, 45, "chk_challenge_%1", [this](int id){ return currentSave.getChallenge(id); });
}