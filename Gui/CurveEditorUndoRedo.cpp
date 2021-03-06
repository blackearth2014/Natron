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

#include "CurveEditorUndoRedo.h"

#include <QDebug>

#include "Global/GlobalDefines.h"

#include "Gui/CurveEditor.h"
#include "Gui/KnobGui.h"
#include "Gui/CurveWidget.h"

#include "Engine/Knob.h"
#include "Engine/Curve.h"
#include "Engine/KnobTypes.h"


//////////////////////////////ADD MULTIPLE KEYS COMMAND//////////////////////////////////////////////
AddKeysCommand::AddKeysCommand(CurveWidget *editor, CurveGui* curve,
                               const std::vector< KeyFrame >& keys,
                               QUndoCommand *parent)
: QUndoCommand(parent)
, _curve(curve)
, _keys(keys)
, _curveWidget(editor)
{
}

void AddKeysCommand::addOrRemoveKeyframe(bool add) {
    boost::shared_ptr<Parametric_Knob> isParametric = boost::dynamic_pointer_cast<Parametric_Knob>(_curve->getKnob()->getKnob());
    
    for(U32 i = 0 ; i < _keys.size();++i){
        _curve->getKnob()->getKnob()->beginValueChange(Natron::USER_EDITED);
        if(add){
            if (isParametric) {
                Natron::Status st = isParametric->addControlPoint(_curve->getDimension(), _keys[i].getTime(),_keys[i].getValue());
                assert(st == Natron::StatOK);
            }else{
                _curve->getKnob()->setKeyframe(_keys[i].getTime(), _curve->getDimension());
            }
        }else{
            if (isParametric) {
                Natron::Status st = isParametric->deleteControlPoint(_curve->getDimension(),
                                                                     _curve->getInternalCurve()->keyFrameIndex(_keys[i].getTime()));
                assert(st == Natron::StatOK);
            }else{
                _curve->getKnob()->removeKeyFrame(_keys[i].getTime(), _curve->getDimension());
            }
        }
    }
    for(U32 i = 0 ; i < _keys.size();++i){
        _curve->getKnob()->getKnob()->endValueChange();
    }
    
    _curveWidget->update();
    
    setText(QObject::tr("Add multiple keyframes"));
}

void AddKeysCommand::undo(){
    addOrRemoveKeyframe(false);
}

void AddKeysCommand::redo(){
    addOrRemoveKeyframe(true);
}

//////////////////////////////REMOVE  MULTIPLE KEYS COMMAND//////////////////////////////////////////////
RemoveKeysCommand::RemoveKeysCommand(CurveWidget* editor,const std::vector<std::pair<CurveGui*,KeyFrame> >& keys,QUndoCommand *parent )
: QUndoCommand(parent)
, _keys(keys)
, _curveWidget(editor)
{
}

void RemoveKeysCommand::addOrRemoveKeyframe(bool add){
    ///this can be called either by a parametric curve widget or by the global curve editor, so we need to handle the different
    ///cases here. Maybe this is bad design, i don't know how we could change it otherwise.
    
    for(U32 i = 0 ; i < _keys.size();++i){
        _keys[i].first->getKnob()->getKnob()->beginValueChange(Natron::USER_EDITED);
        if(add){
            if (_keys[i].first->getKnob()->getKnob()->typeName() == Parametric_Knob::typeNameStatic()) {
                boost::shared_ptr<Parametric_Knob> knob = boost::dynamic_pointer_cast<Parametric_Knob>(_keys[i].first->getKnob()->getKnob());
                Natron::Status st = knob->addControlPoint(_keys[i].first->getDimension(), _keys[i].second.getTime(),_keys[i].second.getValue());
                assert(st == Natron::StatOK);
            }else{
                _keys[i].first->getKnob()->setKeyframe(_keys[i].second.getTime(), _keys[i].first->getDimension());
            }
        }else{
            if (_keys[i].first->getKnob()->getKnob()->typeName() == Parametric_Knob::typeNameStatic()) {
                boost::shared_ptr<Parametric_Knob> knob = boost::dynamic_pointer_cast<Parametric_Knob>(_keys[i].first->getKnob()->getKnob());
                Natron::Status st = knob->deleteControlPoint(_keys[i].first->getDimension(),
                                                             _keys[i].first->getInternalCurve()->keyFrameIndex(_keys[i].second.getTime()));
                assert(st == Natron::StatOK);
            }else{
                _keys[i].first->getKnob()->removeKeyFrame(_keys[i].second.getTime(), _keys[i].first->getDimension());
            }
        }
    }
    for(U32 i = 0 ; i < _keys.size();++i){
        _keys[i].first->getKnob()->getKnob()->endValueChange();
    }
    
    _curveWidget->update();
    setText(QObject::tr("Remove multiple keyframes"));
}

void RemoveKeysCommand::undo(){
    addOrRemoveKeyframe(true);
}
void RemoveKeysCommand::redo(){
    addOrRemoveKeyframe(false);
}

//////////////////////////////MOVE MULTIPLE KEYS COMMAND//////////////////////////////////////////////
MoveKeysCommand::MoveKeysCommand(CurveWidget* editor, const KeyMoveV &keys, double dt, double dv,bool updateOnFirstRedo,
                                 QUndoCommand *parent )
: QUndoCommand(parent)
, _firstRedoCalled(false)
, _updateOnFirstRedo(updateOnFirstRedo)
, _merge(!keys.empty())
, _dt(dt)
, _dv(dv)
, _keys(keys)
, _curveWidget(editor)
{
}

static void
moveKey(const KeyMove&k, double dt, double dv, bool isundo, std::vector<int>& newKeyIndexes)
{
    k.curve->getKnob()->getKnob()->beginValueChange(Natron::USER_EDITED);
    
    std::pair<double,double> curveYRange = k.curve->getInternalCurve()->getCurveYRange();
    
    double newX = k.key.getTime() + dt;
    double newY = k.key.getValue() + dv;
    
    if (newY > curveYRange.second) {
        newY = k.key.getValue();
    } else if (newY < curveYRange.first) {
        newY = k.key.getValue();
    }
    
    int keyframeIndex = k.curve->getInternalCurve()->keyFrameIndex(isundo ? newX : k.key.getTime());
    int newIndex;
    
    k.curve->getInternalCurve()->setKeyFrameValueAndTime(isundo ? k.key.getTime() : newX,
                                                         isundo ? k.key.getValue() : newY,
                                                         keyframeIndex, &newIndex);
    newKeyIndexes.push_back(newIndex);
}

void MoveKeysCommand::move(double dt, double dv, bool isundo)
{
    SelectedKeys newSelectedKeys;
    std::vector<int> newKeyIndexes;
    try {
        if(dt < 0) {
            for (KeyMoveV::iterator it = _keys.begin(); it!= _keys.end(); ++it) {
                moveKey(*it, dt, dv, isundo, newKeyIndexes);
            }
        } else {
            for(KeyMoveV::reverse_iterator it = _keys.rbegin(); it!= _keys.rend(); ++it){
                moveKey(*it, dt, dv, isundo, newKeyIndexes);
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "The keyframe set has changed since this action. This is probably because another user interaction is not "
        "linked to undo/redo stack.";
        return;
    }
    
    
    //copy back the modified keyframes to the selectd keys
    assert(newKeyIndexes.size() == _keys.size());
    int i = 0;
    for (KeyMoveV::iterator it = _keys.begin(); it!= _keys.end(); ++it, ++i) {
        ///we must find the keyframe that has just been inserted into the curve
        const Curve& c = *(it->curve->getInternalCurve());
        KeyFrame foundKey;
        bool found = c.getKeyFrameWithIndex(newKeyIndexes[i], &foundKey);
        if (found) {
            newSelectedKeys.insert(SelectedKey(it->curve, foundKey));
        }
        it->curve->getKnob()->getKnob()->endValueChange();
    }
    if (!_keys.empty()) {
        _curveWidget->setSelectedKeys(newSelectedKeys);
    }
}

void MoveKeysCommand::undo()
{
    move(_dt,_dv,true);
    setText(QObject::tr("Move multiple keys"));
}

void MoveKeysCommand::redo()
{
    ///For each knob, the old value of "evaluateOnChange" that we remember to set it back after the move.
    std::map<KnobGui*, bool> oldEvaluateOnChange;
    if (!_firstRedoCalled && !_updateOnFirstRedo) {
        ///set all the knobs to setEvaluateOnChange(false) so that the node doesn't get to render.
        ///remember to set it back afterwards
        for (KeyMoveV::iterator it = _keys.begin(); it!= _keys.end(); ++it) {
            KnobGui* knob = it->curve->getKnob();
            assert(knob);
            oldEvaluateOnChange.insert(std::make_pair(knob, knob->getKnob()->getEvaluateOnChange()));
        }
        
        ///now set to false the evaluateOnChange
        for (std::map<KnobGui*, bool>::iterator it = oldEvaluateOnChange.begin(); it!=oldEvaluateOnChange.end(); ++it) {
            it->first->getKnob()->setEvaluateOnChange(false);
        }
    }
    move(_dt,_dv,false);
    if (!_firstRedoCalled && !_updateOnFirstRedo) {
        for (std::map<KnobGui*, bool>::iterator it = oldEvaluateOnChange.begin(); it!=oldEvaluateOnChange.end(); ++it) {
            it->first->getKnob()->setEvaluateOnChange(it->second);
        }

    }
    _firstRedoCalled = true;
    setText(QObject::tr("Move multiple keys"));
    
}

bool MoveKeysCommand::mergeWith(const QUndoCommand * command)
{
    const MoveKeysCommand* cmd = dynamic_cast<const MoveKeysCommand*>(command);
    if (cmd && cmd->id() == id()) {
        
        //both commands are placeholders to tell we should stop merging, we merge them into one
        if (!_merge && !cmd->_merge) {
            return true;
        } else if (!_merge && cmd->_merge) {
            //the new cmd is not a placeholder, we break merging.
            return false;
        } else if (!cmd->_merge) {
            _merge = false;
            //notify that we don't want to merge after this
        }
        
        _dt += cmd->_dt;
        _dv += cmd->_dv;
        return true;
    } else {
        return false;
    }
}

int  MoveKeysCommand::id() const
{
    return kCurveEditorMoveMultipleKeysCommandCompressionID;
}


//////////////////////////////SET MULTIPLE KEYS INTERPOLATION COMMAND//////////////////////////////////////////////
SetKeysInterpolationCommand::SetKeysInterpolationCommand(CurveWidget* editor,const std::vector< KeyInterpolationChange >& keys,
                                                         QUndoCommand *parent)
: QUndoCommand(parent)
, _oldInterp(keys)
, _curveWidget(editor)
{
}

void SetKeysInterpolationCommand::setNewInterpolation(bool undo)
{
    SelectedKeys newSelectedKeys;
    for (U32 i = 0; i < _oldInterp.size();++i) {
        _oldInterp[i].curve->getKnob()->getKnob()->beginValueChange(Natron::USER_EDITED);
        int keyframeIndex = _oldInterp[i].curve->getInternalCurve()->keyFrameIndex(_oldInterp[i].key.getTime());
        if (undo) {
            _oldInterp[i].key =  _oldInterp[i].curve->getInternalCurve()->setKeyFrameInterpolation(_oldInterp[i].oldInterp, keyframeIndex);
        } else {
            _oldInterp[i].key = _oldInterp[i].curve->getInternalCurve()->setKeyFrameInterpolation(_oldInterp[i].newInterp, keyframeIndex);
        }
        newSelectedKeys.insert(SelectedKey(_oldInterp[i].curve,_oldInterp[i].key));
    }
    for (U32 i = 0; i < _oldInterp.size();++i) {
        _oldInterp[i].curve->getKnob()->getKnob()->endValueChange();
    }
    _curveWidget->setSelectedKeys(newSelectedKeys);
    setText(QObject::tr("Set multiple keys interpolation"));
}

void SetKeysInterpolationCommand::undo(){
    setNewInterpolation(true);
}

void SetKeysInterpolationCommand::redo(){
    setNewInterpolation(false);
}
