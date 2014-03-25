//  Natron
//
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
*Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012. 
*contact: immarespond at gmail dot com
*
*/

#ifndef NATRON_GUI_SETTINGSPANEL_H_
#define NATRON_GUI_SETTINGSPANEL_H_

#include <map>

#include "Global/Macros.h"
CLANG_DIAG_OFF(deprecated)
#include <QFrame>
CLANG_DIAG_ON(deprecated)

#include <boost/shared_ptr.hpp>

#include "Global/GlobalDefines.h"

class LineEdit;
class Knob;
class KnobGui;
class KnobHolder;
class NodeGui;
class Gui;
class QVBoxLayout;
class QTabWidget;
class Button;
class QHBoxLayout;
class QLabel;
class QUndoStack;
class QUndoCommand;
class FloatingWidget;
namespace Natron{
    class Project;
    class Node;
};

/**
 * @brief An abstract class that defines a dockable properties panel that can be found in the Property bin pane.
**/
class DockablePanel : public QFrame {
    Q_OBJECT
public:
    
    enum HeaderMode{
        FULLY_FEATURED = 0,
        READ_ONLY_NAME,
        NO_HEADER
    };
    
    explicit DockablePanel(Gui* gui
                  ,KnobHolder* holder
                  ,QVBoxLayout* container
                  ,HeaderMode headerMode
                  ,bool useScrollAreasForTabs
                  ,const QString& initialName = QString()
                  ,const QString& helpToolTip = QString()
                  ,bool createDefaultTab = false
                  ,const QString& defaultTab = QString()
                  ,QWidget *parent = 0);
    
    virtual ~DockablePanel() OVERRIDE;
    
    bool isMinimized() const {return _minimized;}
    
    /*inserts a new tab to the dockable panel.*/
    void addTab(const QString& name);

    const std::map<boost::shared_ptr<Knob>,KnobGui*>& getKnobs() const { return _knobs; }
    
    /*Creates a new button and inserts it in the header
     at position headerPosition. You can then take
     the pointer to the Button to customize it the
     way you want. The ownership of the Button remains
     to the DockablePanel.*/
    Button* insertHeaderButton(int headerPosition);
    
    QVBoxLayout* getContainer() const {return _container;}
    
    void pushUndoCommand(QUndoCommand* cmd);

    /*Search an existing knob GUI in the map, otherwise creates
     the gui for the knob.*/
    KnobGui* findKnobGuiOrCreate(boost::shared_ptr<Knob> knob,bool makeNewLine,QWidget* lastRowWidget,
                                 const std::vector< boost::shared_ptr< Knob > >& knobsOnSameLine = std::vector< boost::shared_ptr< Knob > >());
    
    QUndoStack* getUndoStack() const { return _undoStack; }
    
    const QUndoCommand* getLastUndoCommand() const;
    
    Gui* getGui() const;
    
    void insertHeaderWidget(int index,QWidget* widget);
    
    void appendHeaderWidget(QWidget* widget);
    
    QWidget* getHeaderWidget() const;

public slots:
    
    /*Internal slot, not meant to be called externally.*/
    void closePanel();
    
    /*Internal slot, not meant to be called externally.*/
    void minimizeOrMaximize(bool toggled);
    
    /*Internal slot, not meant to be called externally.*/
    void showHelp();
    
    /*You can connect to this when you want to change
     the name externally.*/
    void onNameChanged(const QString& str);
    
    /*initializes the knobs GUI*/
    void initializeKnobs();
    
    /*Internal slot, not meant to be called externally.*/
    void onKnobDeletion();
    
    /*Internal slot, not meant to be called externally.*/
    void onUndoClicked();
    
    /*Internal slot, not meant to be called externally.*/
    void onRedoPressed();
    
    void onRestoreDefaultsButtonClicked();

    void onLineEditNameEditingFinished();
    
    void floatPanel();
signals:
    
    /*emitted when the panel is clicked*/
    void selected();
    
    /*emitted when the name changed on the line edit*/
    void nameChanged(QString);
    
    /*emitted when the user pressed undo*/
    void undoneChange();
    
    /*emitted when the user pressed redo*/
    void redoneChange();
    
    /*emitted when the panel is minimized*/
    void minimized();
    
    /*emitted when the panel is maximized*/
    void maximized();
    
private:

    virtual void mousePressEvent(QMouseEvent* e) OVERRIDE FINAL {
        emit selected();
        QFrame::mousePressEvent(e);
    }

    
private:
    
    void initializeKnobVector(const std::vector< boost::shared_ptr< Knob> >& knobs,bool onlyTopLevelKnobs);
    
    // FIXME: PIMPL
    Gui* _gui;
    
    QVBoxLayout* _container; /*!< ptr to the layout containing this DockablePanel*/

    /*global layout*/
    QVBoxLayout* _mainLayout;

    /*Header related*/
    QFrame* _headerWidget;
    QHBoxLayout *_headerLayout;

    LineEdit* _nameLineEdit; /*!< if the name is editable*/
    QLabel* _nameLabel; /*!< if the name is read-only*/

    /*Tab related*/
    QTabWidget* _tabWidget;

    Button* _helpButton;
    Button* _minimize;
    Button* _floatButton;
    Button* _cross;

    Button* _undoButton;
    Button* _redoButton;
    Button* _restoreDefaultsButton;

    bool _minimized; /*!< true if the panel is minimized*/
    QUndoStack* _undoStack; /*!< undo/redo stack*/
    
    bool _floating; /*!< true if the panel is floating*/
    FloatingWidget* _floatingWidget;

    /*a map storing for each knob a pointer to their GUI.*/
    std::map<boost::shared_ptr<Knob>,KnobGui*> _knobs;
    KnobHolder* _holder;

    /* map<tab name, pair<tab , row count> >*/
    std::map<QString,std::pair<QWidget*,int> > _tabs;

    QString _defaultTabName;
    
    bool _useScrollAreasForTabs;
    
    HeaderMode _mode;
};

class NodeSettingsPanel : public DockablePanel
{
    Q_OBJECT
    
    Q_PROPERTY( bool _selected READ isSelected WRITE setSelected)
    
    /*Pointer to the node GUI*/
    NodeGui* _nodeGUI;
    
    bool _selected;
    
    Button* _centerNodeButton;

public:

    explicit NodeSettingsPanel(Gui* gui,NodeGui* NodeUi,QVBoxLayout* container, QWidget *parent = 0);
    
    virtual ~NodeSettingsPanel();
    
    void setSelected(bool s);
    
    bool isSelected() const {return _selected;}
    
public slots:
    
    void centerNode();
  
};

#endif // NATRON_GUI_SETTINGSPANEL_H_
