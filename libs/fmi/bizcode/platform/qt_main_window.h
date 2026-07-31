#pragma once
// SmartMet main window for Linux builds: menu bar, toolbar, data dock and status bar
// around the map view. The Windows build uses the MFC CMainFrame instead.

#ifdef HAVE_QT6

#include <QMainWindow>

class QAction;
class QComboBox;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class SmartMetMapView;
class WeatherDataModel;

class SmartMetMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SmartMetMainWindow(WeatherDataModel& model, QWidget* parent = nullptr);
    ~SmartMetMainWindow() override;

    SmartMetMapView* mapView() const { return mapView_; }

private slots:
    void onOpenData();
    void onDataTreeSelection();
    void onParamComboChanged(int index);
    void onLevelComboChanged(int index);
    void onTimeComboChanged(int index);
    void onCursorReadout(const QString& text);

    void onNextData();
    void onPrevData();
    void onNextTime();
    void onPrevTime();
    void onNextParam();
    void onPrevParam();
    void onNextLevel();
    void onPrevLevel();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createDataDock();
    void createStatusBar();

    // Pushes the model state into the window: title, tree selection, combos, status bar
    // and a map redraw. Combo signals are blocked while this runs.
    void syncFromModel();
    void rebuildDataTree();
    void rebuildCombos();
    void updateStatusInfo();

    WeatherDataModel& model_;
    SmartMetMapView* mapView_ = nullptr;

    QTreeWidget* dataTree_ = nullptr;
    QComboBox* paramCombo_ = nullptr;
    QComboBox* levelCombo_ = nullptr;
    QComboBox* timeCombo_ = nullptr;

    QLabel* cursorLabel_ = nullptr;
    QLabel* infoLabel_ = nullptr;

    QAction* openAction_ = nullptr;
    QAction* quitAction_ = nullptr;
    QAction* nextDataAction_ = nullptr;
    QAction* prevDataAction_ = nullptr;
    QAction* nextTimeAction_ = nullptr;
    QAction* prevTimeAction_ = nullptr;
    QAction* nextParamAction_ = nullptr;
    QAction* prevParamAction_ = nullptr;
    QAction* nextLevelAction_ = nullptr;
    QAction* prevLevelAction_ = nullptr;
    QAction* legendAction_ = nullptr;
    QAction* dockAction_ = nullptr;
    QAction* aboutAction_ = nullptr;

    // Guards syncFromModel() against the combo signals it triggers itself
    bool updatingUi_ = false;
};

#endif // HAVE_QT6
