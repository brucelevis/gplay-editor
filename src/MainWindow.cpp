#include "MainWindow.h"
#include "GplayDevice.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>
#include <QMenuBar>
#include <QApplication>
#include <QVBoxLayout>

#include "node-editor/common/Nodestyle.h"
#include "node-editor/spark-nodes/SparkNodesRegistry.h"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //setNodeStyle();
    createWidgets();
    createActions();
    createMenus();

    // create Nodes registry
    _sparkNodesRegistry = registerSparkNodesDataModels();
    _nodeFlowScene->setRegistry(_sparkNodesRegistry);

    // create gplay render view into _renderView widget
    GplayDevice::getInstance()->createRenderWindow((void*)_renderView);

    _gameLoopTimerId = startTimer(0);
}

MainWindow::~MainWindow()
{
    delete _nodeFlowScene;
    delete _nodeFlowView;
    delete _renderView;
    delete _viewportContainer;
    delete _dockView;
    delete _dockNodeFlowView;
}

void MainWindow::createWidgets()
{
    // create gplay viewport widget
    _renderView = new QWidget(this);
    _renderView->setMouseTracking(true);
    _renderView->setFocusPolicy(Qt::StrongFocus);

    _viewportContainer = new QWidget(this);
    _viewportContainer->setLayout(new QVBoxLayout());
    _viewportContainer->layout()->addWidget(_renderView);

    _dockView = new QDockWidget("Viewport", this);
    _dockView->setWidget(_viewportContainer);
    _dockView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::TopDockWidgetArea, _dockView);

    _nodeFlowScene = new CustomFlowScene();

    _nodeFlowView = new FlowView(_nodeFlowScene);
    _nodeFlowView->setWindowTitle("Node-based flow editor");
    _nodeFlowView->resize(800, 600);
    _nodeFlowView->show();
    _nodeFlowView->scale(0.9, 0.9);

    _dockNodeFlowView = new QDockWidget("NodeGraph", this);
    _dockNodeFlowView->setWidget(_nodeFlowView);
    _dockNodeFlowView->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, _dockNodeFlowView);

    _pathView = new GraphView(this);

    _dockGraph = new QDockWidget("Graph", this);
    _dockGraph->setWidget(_pathView);
    _dockGraph->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::TopDockWidgetArea, _dockGraph);


    // make some connections

    connect(_nodeFlowScene, &CustomFlowScene::showPathNodeRequest, _pathView, &GraphView::setPathNode);

    // connect to FlowView deleteSelectionAction a method to delete comments graphics items.
    QAction* deleteAction = _nodeFlowView->deleteSelectionAction();
    connect(deleteAction, &QAction::triggered, _nodeFlowScene, &CustomFlowScene::deleteSelectedComments);
}

void MainWindow::createActions()
{
    _newAct = new QAction(tr("&New"), this);
    _newAct->setShortcuts(QKeySequence::New);
    _newAct->setStatusTip(tr("New"));
    connect(_newAct, &QAction::triggered, this, &MainWindow::newFile);

    _openAct = new QAction(tr("&Open"), this);
    _openAct->setShortcuts(QKeySequence::Open);
    _openAct->setStatusTip(tr("Open"));
    connect(_openAct, &QAction::triggered, this, &MainWindow::open);

    _saveAct = new QAction(tr("&Save"), this);
    _saveAct->setShortcuts(QKeySequence::Save);
    _saveAct->setStatusTip(tr("Save"));
    connect(_saveAct, &QAction::triggered, this, &MainWindow::save);
}

void MainWindow::createMenus()
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileMenu->addAction(_newAct);
    _fileMenu->addAction(_openAct);
    _fileMenu->addAction(_saveAct);
}

void MainWindow::newFile()
{
    _nodeFlowScene->clearScene();
    _nodeFlowScene->clearComments();
}

void MainWindow::open()
{
    _nodeFlowScene->load();
}

void MainWindow::save()
{
    _nodeFlowScene->save();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, qApp->applicationName(),
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
    if(resBtn != QMessageBox::Yes)
    {
        event->ignore();
    }
    else
    {
        event->accept();
        shutdown();
    }
}

void MainWindow::shutdown()
{
    killTimer(_gameLoopTimerId);
    GplayDevice::getInstance()->stop();
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    QMainWindow::timerEvent(event);
    GplayDevice::getInstance()->runFrame();
}
