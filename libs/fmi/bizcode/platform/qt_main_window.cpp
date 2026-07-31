#ifdef HAVE_QT6

#include "qt_main_window.h"

#include "qt_map_view.h"
#include "weather_data_model.h"

#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiInfoData.h>

#include <QAction>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>

#include <filesystem>

namespace
{
    // Item data role marking a tree item as a data (value = data index) or as a
    // parameter of the data in its parent item (value = parameter index).
    constexpr int kIndexRole = Qt::UserRole + 1;
    constexpr int kIsParamRole = Qt::UserRole + 2;
}

SmartMetMainWindow::SmartMetMainWindow(WeatherDataModel& model, QWidget* parent)
    : QMainWindow(parent), model_(model)
{
    mapView_ = new SmartMetMapView(model_, this);
    setCentralWidget(mapView_);

    createActions();
    createDataDock();   // creates dockAction_, which the View menu needs
    createMenus();
    createToolBar();
    createStatusBar();

    connect(mapView_, &SmartMetMapView::cursorReadout,
            this, &SmartMetMainWindow::onCursorReadout);
    connect(mapView_, &SmartMetMapView::viewChanged,
            this, &SmartMetMainWindow::updateStatusInfo);

    resize(1200, 800);
    rebuildDataTree();
    syncFromModel();
    mapView_->setFocus();
}

SmartMetMainWindow::~SmartMetMainWindow() = default;

void SmartMetMainWindow::createActions()
{
    openAction_ = new QAction("&Open querydata...", this);
    openAction_->setShortcut(QKeySequence::Open);
    connect(openAction_, &QAction::triggered, this, &SmartMetMainWindow::onOpenData);

    quitAction_ = new QAction("&Quit", this);
    quitAction_->setShortcut(QKeySequence::Quit);
    connect(quitAction_, &QAction::triggered, this, &QWidget::close);

    // Navigation shortcuts are scoped to the map view. A window-wide arrow key shortcut
    // would swallow the arrow keys of the data tree and the toolbar combo boxes.
    auto makeNavigationAction = [this](const QString& text, QKeySequence shortcut,
                                       void (SmartMetMainWindow::*slot)())
    {
        auto* action = new QAction(text, this);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        connect(action, &QAction::triggered, this, slot);
        mapView_->addAction(action);
        return action;
    };

    nextDataAction_  = makeNavigationAction("&Next data", QKeySequence(Qt::Key_N),
                                            &SmartMetMainWindow::onNextData);
    prevDataAction_  = makeNavigationAction("&Previous data", QKeySequence(Qt::Key_P),
                                            &SmartMetMainWindow::onPrevData);
    prevTimeAction_  = makeNavigationAction("Previous &time step", QKeySequence(Qt::Key_Left),
                                            &SmartMetMainWindow::onPrevTime);
    nextTimeAction_  = makeNavigationAction("Next t&ime step", QKeySequence(Qt::Key_Right),
                                            &SmartMetMainWindow::onNextTime);
    nextParamAction_ = makeNavigationAction("Next p&arameter", QKeySequence(Qt::Key_Up),
                                            &SmartMetMainWindow::onNextParam);
    prevParamAction_ = makeNavigationAction("Previous para&meter", QKeySequence(Qt::Key_Down),
                                            &SmartMetMainWindow::onPrevParam);
    nextLevelAction_ = makeNavigationAction("Next &level", QKeySequence(Qt::Key_PageUp),
                                            &SmartMetMainWindow::onNextLevel);
    prevLevelAction_ = makeNavigationAction("Previous le&vel", QKeySequence(Qt::Key_PageDown),
                                            &SmartMetMainWindow::onPrevLevel);

    resetZoomAction_ = new QAction("&Reset zoom", this);
    resetZoomAction_->setShortcut(QKeySequence(Qt::Key_R));
    resetZoomAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(resetZoomAction_, &QAction::triggered, this,
            [this]() { mapView_->resetZoom(); });
    mapView_->addAction(resetZoomAction_);

    legendAction_ = new QAction("Show &legend", this);
    legendAction_->setCheckable(true);
    legendAction_->setChecked(true);
    connect(legendAction_, &QAction::toggled, this,
            [this](bool on) { mapView_->setLegendVisible(on); });

    aboutAction_ = new QAction("&About SmartMet", this);
    connect(aboutAction_, &QAction::triggered, this, [this]()
    {
        QMessageBox::about(this, "About SmartMet",
            "SmartMet Workstation\n\n"
            "Linux port (Qt6). Weather data rendering via Cairo and Trax.\n"
            "Finnish Meteorological Institute.");
    });
}

void SmartMetMainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(openAction_);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction_);

    QMenu* dataMenu = menuBar()->addMenu("&Data");
    dataMenu->addAction(prevDataAction_);
    dataMenu->addAction(nextDataAction_);
    dataMenu->addSeparator();
    dataMenu->addAction(prevTimeAction_);
    dataMenu->addAction(nextTimeAction_);
    dataMenu->addSeparator();
    dataMenu->addAction(prevParamAction_);
    dataMenu->addAction(nextParamAction_);
    dataMenu->addSeparator();
    dataMenu->addAction(prevLevelAction_);
    dataMenu->addAction(nextLevelAction_);

    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(resetZoomAction_);
    viewMenu->addSeparator();
    viewMenu->addAction(legendAction_);
    viewMenu->addAction(dockAction_);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAction_);
}

void SmartMetMainWindow::createToolBar()
{
    QToolBar* toolBar = addToolBar("Navigation");
    toolBar->setObjectName("NavigationToolBar");
    toolBar->setMovable(false);

    toolBar->addAction(prevTimeAction_);
    toolBar->addAction(nextTimeAction_);
    toolBar->addSeparator();

    toolBar->addWidget(new QLabel(" Parameter ", this));
    paramCombo_ = new QComboBox(this);
    paramCombo_->setMinimumWidth(180);
    connect(paramCombo_, &QComboBox::currentIndexChanged,
            this, &SmartMetMainWindow::onParamComboChanged);
    toolBar->addWidget(paramCombo_);

    toolBar->addWidget(new QLabel(" Level ", this));
    levelCombo_ = new QComboBox(this);
    levelCombo_->setMinimumWidth(110);
    connect(levelCombo_, &QComboBox::currentIndexChanged,
            this, &SmartMetMainWindow::onLevelComboChanged);
    toolBar->addWidget(levelCombo_);

    toolBar->addWidget(new QLabel(" Time ", this));
    timeCombo_ = new QComboBox(this);
    timeCombo_->setMinimumWidth(160);
    connect(timeCombo_, &QComboBox::currentIndexChanged,
            this, &SmartMetMainWindow::onTimeComboChanged);
    toolBar->addWidget(timeCombo_);
}

void SmartMetMainWindow::createDataDock()
{
    auto* dock = new QDockWidget("Data", this);
    dock->setObjectName("DataDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    dataTree_ = new QTreeWidget(dock);
    dataTree_->setHeaderLabels({"Loaded data"});
    dataTree_->setMinimumWidth(200);
    connect(dataTree_, &QTreeWidget::itemSelectionChanged,
            this, &SmartMetMainWindow::onDataTreeSelection);

    dock->setWidget(dataTree_);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dockAction_ = dock->toggleViewAction();
    dockAction_->setText("Show &data panel");
}

void SmartMetMainWindow::createStatusBar()
{
    cursorLabel_ = new QLabel(this);
    cursorLabel_->setMinimumWidth(320);
    infoLabel_ = new QLabel(this);

    statusBar()->addWidget(cursorLabel_);
    statusBar()->addPermanentWidget(infoLabel_);
}

void SmartMetMainWindow::syncFromModel()
{
    updatingUi_ = true;

    const bool hasData = model_.hasData();
    mapView_->setDemoMode(!hasData);

    if(hasData)
    {
        setWindowTitle(QString("SmartMet - %1: %2 [%3]")
            .arg(QString::fromStdString(model_.dataName()))
            .arg(QString::fromStdString(model_.paramName()))
            .arg(QString::fromStdString(model_.timeString())));
        rebuildCombos();
    }
    else
    {
        setWindowTitle("SmartMet (demo)");
    }

    // Select the current data/parameter in the tree without recursing back in
    if(dataTree_)
    {
        const QSignalBlocker blocker(dataTree_);
        for(int i = 0; i < dataTree_->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem* dataItem = dataTree_->topLevelItem(i);
            const bool isCurrent = dataItem->data(0, kIndexRole).toInt() == model_.currentDataIndex();
            for(int j = 0; j < dataItem->childCount(); ++j)
            {
                QTreeWidgetItem* paramItem = dataItem->child(j);
                paramItem->setSelected(isCurrent &&
                    paramItem->data(0, kIndexRole).toInt() == model_.currentParamIndex());
            }
            dataItem->setSelected(false);
            if(isCurrent)
                dataItem->setExpanded(true);
        }
    }

    updateStatusInfo();
    updatingUi_ = false;

    mapView_->refresh();
}

void SmartMetMainWindow::rebuildDataTree()
{
    if(!dataTree_) return;

    const QSignalBlocker blocker(dataTree_);
    dataTree_->clear();

    const auto& datas = model_.loader().loadedData();
    for(int i = 0; i < static_cast<int>(datas.size()); ++i)
    {
        auto* dataItem = new QTreeWidgetItem(dataTree_);
        dataItem->setText(0, QString::fromStdString(datas[i].name));
        dataItem->setToolTip(0, QString::fromStdString(datas[i].fileName));
        dataItem->setData(0, kIndexRole, i);
        dataItem->setData(0, kIsParamRole, false);

        // Parameter names come from the data's own info, not the current selection
        NFmiFastQueryInfo* info = datas[i].info.get();
        if(!info) continue;
        const unsigned long saved = info->ParamIndex();
        for(unsigned long p = 0; p < info->SizeParams(); ++p)
        {
            if(!info->ParamIndex(p)) continue;
            auto* paramItem = new QTreeWidgetItem(dataItem);
            paramItem->setText(0, QString(info->Param().GetParamName().CharPtr()));
            paramItem->setData(0, kIndexRole, static_cast<int>(p));
            paramItem->setData(0, kIsParamRole, true);
        }
        info->ParamIndex(saved);
    }
}

void SmartMetMainWindow::rebuildCombos()
{
    const QSignalBlocker paramBlocker(paramCombo_);
    const QSignalBlocker levelBlocker(levelCombo_);
    const QSignalBlocker timeBlocker(timeCombo_);

    paramCombo_->clear();
    for(const auto& name : model_.paramNames())
        paramCombo_->addItem(QString::fromStdString(name));
    paramCombo_->setCurrentIndex(model_.currentParamIndex());

    levelCombo_->clear();
    for(const auto& name : model_.levelNames())
        levelCombo_->addItem(QString::fromStdString(name));
    levelCombo_->setCurrentIndex(model_.currentLevelIndex());
    levelCombo_->setEnabled(levelCombo_->count() > 1);

    timeCombo_->clear();
    for(const auto& name : model_.timeNames())
        timeCombo_->addItem(QString::fromStdString(name));
    timeCombo_->setCurrentIndex(model_.currentTimeIndex());
}

void SmartMetMainWindow::updateStatusInfo()
{
    if(!infoLabel_) return;

    if(!model_.hasData())
    {
        infoLabel_->setText("demo mode - no querydata loaded");
        return;
    }

    const QString zoomText = mapView_->isZoomed()
        ? QString("   Zoom %1x").arg(mapView_->zoomFactor(), 0, 'f', 1)
        : QString();

    infoLabel_->setText(QString("Data %1/%2   Grid %3x%4   %5%6")
        .arg(model_.currentDataIndex() + 1)
        .arg(model_.dataCount())
        .arg(model_.gridWidth())
        .arg(model_.gridHeight())
        .arg(model_.validValueCount() > 0
                ? QString("Range [%1, %2]")
                      .arg(static_cast<double>(model_.dataMin()), 0, 'f', 1)
                      .arg(static_cast<double>(model_.dataMax()), 0, 'f', 1)
                : QString("no values"))
        .arg(zoomText));
}

void SmartMetMainWindow::onOpenData()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this, "Open querydata", QString(), "QueryData (*.sqd *.fqd);;All files (*)");
    if(fileName.isEmpty()) return;

    const std::string path = fileName.toStdString();
    if(!model_.loader().loadFile(path,
                                 std::filesystem::path(path).filename().string(),
                                 NFmiInfoData::kViewable))
    {
        QMessageBox::warning(this, "Open querydata",
            QString("Could not read %1.\nSee the terminal output for details.").arg(fileName));
        return;
    }

    rebuildDataTree();
    model_.selectData(model_.dataCount() - 1);
    syncFromModel();
}

void SmartMetMainWindow::onDataTreeSelection()
{
    if(updatingUi_) return;

    const auto selected = dataTree_->selectedItems();
    if(selected.isEmpty()) return;

    QTreeWidgetItem* item = selected.first();
    const bool isParam = item->data(0, kIsParamRole).toBool();
    const int index = item->data(0, kIndexRole).toInt();

    if(isParam)
    {
        QTreeWidgetItem* dataItem = item->parent();
        if(!dataItem) return;
        const int dataIndex = dataItem->data(0, kIndexRole).toInt();
        if(dataIndex != model_.currentDataIndex())
            model_.selectData(dataIndex);
        model_.selectParam(index);
    }
    else
    {
        model_.selectData(index);
    }
    syncFromModel();
}

void SmartMetMainWindow::onParamComboChanged(int index)
{
    if(updatingUi_ || index < 0) return;
    model_.selectParam(index);
    syncFromModel();
}

void SmartMetMainWindow::onLevelComboChanged(int index)
{
    if(updatingUi_ || index < 0) return;
    model_.selectLevel(index);
    syncFromModel();
}

void SmartMetMainWindow::onTimeComboChanged(int index)
{
    if(updatingUi_ || index < 0) return;
    model_.selectTime(index);
    syncFromModel();
}

void SmartMetMainWindow::onCursorReadout(const QString& text)
{
    if(cursorLabel_)
        cursorLabel_->setText(text);
}

void SmartMetMainWindow::onNextData()  { if(model_.nextData())  syncFromModel(); }
void SmartMetMainWindow::onPrevData()  { if(model_.prevData())  syncFromModel(); }
void SmartMetMainWindow::onNextTime()  { if(model_.nextTime())  syncFromModel(); }
void SmartMetMainWindow::onPrevTime()  { if(model_.prevTime())  syncFromModel(); }
void SmartMetMainWindow::onNextParam() { if(model_.nextParam()) syncFromModel(); }
void SmartMetMainWindow::onPrevParam() { if(model_.prevParam()) syncFromModel(); }
void SmartMetMainWindow::onNextLevel() { if(model_.nextLevel()) syncFromModel(); }
void SmartMetMainWindow::onPrevLevel() { if(model_.prevLevel()) syncFromModel(); }

#endif // HAVE_QT6
