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

#ifndef NATRON_GUI_KNOBGUI_H_
#define NATRON_GUI_KNOBGUI_H_

#include "Global/Macros.h"
CLANG_DIAG_OFF(deprecated)
CLANG_DIAG_OFF(uninitialized)
#include <QtCore/QObject>
#include <QDialog>
CLANG_DIAG_ON(deprecated)
CLANG_DIAG_ON(uninitialized)

#include <boost/shared_ptr.hpp>

#include "Global/GlobalDefines.h"
#include "Engine/Knob.h"
#include "Engine/Curve.h"
#include "Engine/KnobGuiI.h"

// Qt
class QUndoCommand; //used by KnobGui
class QVBoxLayout; //used by KnobGui
class QHBoxLayout; //used by KnobGui
class QFormLayout;
class QMenu;
class QLabel;

// Engine
class Variant; //used by KnobGui
class KeyFrame;

// Gui
class ComboBox;
class Button;
class AnimationButton; //used by KnobGui
class DockablePanel; //used by KnobGui
class Gui;

class KnobGui : public QObject,public KnobGuiI
{
    Q_OBJECT

public:
    
    
    KnobGui(boost::shared_ptr<KnobI> knob,DockablePanel* container);
    
    virtual ~KnobGui() OVERRIDE;
        
   
    
    /**
     * @brief This function must return a pointer to the internal knob. 
     * This is virtual as it is easier to hold the knob in the derived class
     * avoiding many dynamic_cast in the deriving class.
     **/
    virtual boost::shared_ptr<KnobI> getKnob() const = 0;
    
    bool triggerNewLine() const;
    
    void turnOffNewLine();
    
    /*Set the spacing between items in the layout*/
    void setSpacingBetweenItems(int spacing);
    
    int getSpacingBetweenItems() const ;
    
    bool hasWidgetBeenCreated() const ;
    
    void createGUI(QFormLayout* containerLayout,
                   QWidget* fieldContainer,
                   QWidget* label,
                   QHBoxLayout* layout,
                   int row,
                   bool isOnNewLine,
                   const std::vector< boost::shared_ptr< KnobI > >& knobsOnSameLine);

        
    void pushUndoCommand(QUndoCommand* cmd);
    
    const QUndoCommand* getLastUndoCommand() const;
    
    
    void setKeyframe(double time,int dimension);

    void removeKeyFrame(double time,int dimension);
    
    QString toolTip() const;
    
    bool hasToolTip() const;
    
    Gui* getGui() const;
    
    void enableRightClickMenu(QWidget* widget,int dimension);

    virtual bool showDescriptionLabel() const { return true; }
    
    QWidget* getFieldContainer() const;

    /**
     * @brief Returns the row index of the knob in the layout. 
     * The knob MUST be in the layout for this function to work,
     * that is, it must be visible.
     **/
    int getActualIndexInLayout() const;
    
    bool isOnNewLine() const;
    
    ////calls setReadOnly and also set the label black
    void setReadOnly_(bool readOnly,int dimension);
 
    int getKnobsCountOnSameLine() const;
    
    
    void removeAllKeyframeMarkersOnTimeline(int dim);
    void setAllKeyframeMarkersOnTimeline(int dim);
    void setKeyframeMarkerOnTimeline(int time);
    
    /*This function is used by KnobUndoCommand. Calling this in a onInternalValueChanged/valueChanged
     signal/slot sequence can cause an infinite loop.*/
    template<typename T>
    int setValue(int dimension,const T& v,KeyFrame* newKey,bool refreshGui)
    {
        Knob<T>* knob = dynamic_cast<Knob<T>*>(getKnob().get());
        KnobHelper::ValueChangedReturnCode ret = knob->onValueChanged(dimension, v, newKey);
        if(ret > 0){
            assert(newKey);
            setKeyframeMarkerOnTimeline(newKey->getTime());
            emit keyFrameSet();
        }
        if (refreshGui) {
            updateGUI(dimension);
        }
        checkAnimationLevel(dimension);
        return (int)ret;
    }
    
    virtual void swapOpenGLBuffers() OVERRIDE FINAL;
    virtual void redraw() OVERRIDE FINAL;
    virtual void getViewportSize(double &width, double &height) const OVERRIDE FINAL;
    virtual void getPixelScale(double& xScale, double& yScale) const  OVERRIDE FINAL;
    virtual void getBackgroundColour(double &r, double &g, double &b) const OVERRIDE FINAL;
    
    ///Should set to the underlying knob the gui ptr
    virtual void setKnobGuiPointer() OVERRIDE FINAL;

    
public slots:
    
    
    /**
     * @brief Called when the internal value held by the knob is changed. It calls updateGUI().
     **/
    void onInternalValueChanged(int dimension);
    
    void onInternalKeySet(SequenceTime time,int dimension);

    void onInternalKeyRemoved(SequenceTime time,int dimension);
    
    void onInternalAnimationRemoved();

    void setSecret();

    void showAnimationMenu();
    
    void onRightClickClicked(const QPoint& pos);
    
    void showRightClickMenuForDimension(const QPoint& pos,int dimension);
    
    void setEnabledSlot();
    
    void hide();
    
    ///if index is != -1 then it will reinsert the knob at this index in the layout
    void show(int index = -1);
        
    void onSetKeyActionTriggered();
    
    void onRemoveKeyActionTriggered();
    
    void onShowInCurveEditorActionTriggered();
    
    void onRemoveAnyAnimationActionTriggered();

    void onConstantInterpActionTriggered();
    
    void onLinearInterpActionTriggered();
    
    void onSmoothInterpActionTriggered();
    
    void onCatmullromInterpActionTriggered();
    
    void onCubicInterpActionTriggered();
    
    void onHorizontalInterpActionTriggered();
    
    
    void onCopyValuesActionTriggered();
    void copyValues(int dimension);
    
    void onPasteValuesActionTriggered();
    void pasteValues(int dimension);
    
    
    void onCopyAnimationActionTriggered();
    
    void onPasteAnimationActionTriggered();
    
    void onLinkToActionTriggered();
    void linkTo(int dimension);
    
    void onUnlinkActionTriggered();
    void unlink(int dimension);
    
    void onResetDefaultValuesActionTriggered();
    void resetDefault(int dimension);
    
    void onKnobSlavedChanged(int dimension,bool b);

    void onSetValueUsingUndoStack(const Variant& v,int dim);
    
    void onSetDirty(bool d);
signals:
    
    void knobUndoneChange();
    
    void knobRedoneChange();

    void keyFrameSetByUser(SequenceTime,int);

    void keyFrameRemovedByUser(SequenceTime,int);

    /**
     *@brief Emitted whenever a keyframe is set by the user or by the plugin.
    **/
    void keyFrameSet();

    /**
     *@brief Emitted whenever a keyframe is removed by the user or by the plugin.
    **/
    void keyFrameRemoved();
    
    /**
     *@brief Emitted whenever a keyframe's interpolation method is changed by the user or by the plugin.
     **/
    void keyInterpolationChanged();
    
    ///emitted when the description label is clicked
    void labelClicked(bool);

   
protected:
    
    /**
     * @brief Called when the internal value held by the knob is changed, you must implement
     * it to update the interface to reflect the new value. You can query the new value
     * by calling knob->getValue()
     **/
    virtual void updateGUI(int dimension) = 0;
    
private:

    void updateGuiInternal(int dimension);
    
    void copyToClipBoard(int dimension,bool copyAnimation);
    
    void pasteClipBoard(int dimension);

    virtual void _hide() = 0;

    virtual void _show() = 0;

    virtual void setEnabled() = 0;
    
    virtual void setReadOnly(bool readOnly,int dimension) = 0;
    
    virtual void setDirty(bool dirty) = 0;
    
    /**
     * @brief Must fill the horizontal layout with all the widgets composing the knob.
     **/
    virtual void createWidget(QHBoxLayout* layout) = 0;
    
 
    
    /*Called right away after updateGUI(). Depending in the animation level
     the widget for the knob could display its gui a bit differently.
     */
    virtual void reflectAnimationLevel(int /*dimension*/,Natron::AnimationLevel /*level*/) {}

    /*Calls reflectAnimationLevel with good parameters. Called right away after updateGUI() */
    void checkAnimationLevel(int dimension);
    
    void createAnimationMenu(QMenu* menu);
    
    void createAnimationButton(QHBoxLayout* layout);
    
 
    
    void setInterpolationForDimensions(const std::vector<int>& dimensions,Natron::KeyframeType interp);
    
    
private:
    
    struct KnobGuiPrivate;
    boost::scoped_ptr<KnobGuiPrivate> _imp;
};


class LinkToKnobDialog : public QDialog {
    
    Q_OBJECT
public:
    
    LinkToKnobDialog(KnobGui* from,QWidget* parent);
    
    virtual ~LinkToKnobDialog() OVERRIDE { _allKnobs.clear(); }
    
    std::pair<int ,boost::shared_ptr<KnobI> > getSelectedKnobs() const;

private:
    // FIXME: PIMPL
    QVBoxLayout* _mainLayout;
    QHBoxLayout* _firstLineLayout;
    QWidget* _firstLine;
    QLabel* _selectKnobLabel;
    ComboBox* _selectionCombo;

    QWidget* _buttonsWidget;
    Button* _cancelButton;
    Button* _okButton;
    QHBoxLayout* _buttonsLayout;

    std::map<QString,std::pair<int,boost::shared_ptr<KnobI > > > _allKnobs;
};

#endif // NATRON_GUI_KNOBGUI_H_
