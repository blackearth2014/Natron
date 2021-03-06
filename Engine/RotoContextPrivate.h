//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 *Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 *contact: immarespond at gmail dot com
 *
 */

#ifndef ROTOCONTEXTPRIVATE_H
#define ROTOCONTEXTPRIVATE_H

#include <list>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include <QMutex>
#include <QCoreApplication>
#include <QThread>
#include <QReadWriteLock>

#include <cairo/cairo.h>


#include "Engine/Curve.h"
#include "Engine/KnobTypes.h"
#include "Engine/Node.h"
#include "Engine/EffectInstance.h"
#include "Engine/AppManager.h"

#include "Global/GlobalDefines.h"

class Bezier;

struct BezierCPPrivate
{
    Bezier* holder;
    
    ///the animation curves for the position in the 2D plane
    Curve curveX,curveY;
    double x,y; //< used when there is no keyframe
    
    ///the animation curves for the derivatives
    Curve curveLeftBezierX,curveRightBezierX,curveLeftBezierY,curveRightBezierY;
    double leftX,rightX,leftY,rightY; //< used when there is no keyframe
    
    mutable QReadWriteLock masterMutex;
    Double_Knob* masterTrack; //< is this point linked to a track ?
    
    BezierCPPrivate(Bezier* curve)
    : holder(curve)
    , curveX()
    , curveY()
    , x(0)
    , y(0)
    , curveLeftBezierX()
    , curveRightBezierX()
    , curveLeftBezierY()
    , curveRightBezierY()
    , leftX(0)
    , rightX(0)
    , leftY(0)
    , rightY(0)
    , masterMutex()
    , masterTrack(NULL)
    {
        
    }
    
};

class BezierCP;
typedef std::list< boost::shared_ptr<BezierCP> > BezierCPs;


struct BezierPrivate
{
    
    BezierCPs points; //< the control points of the curve
    BezierCPs featherPoints; //< the feather points, the number of feather points must equal the number of cp.
    bool finished; //< when finished is true, the last point of the list is connected to the first point of the list.
    
    BezierPrivate()
    : points()
    , featherPoints()
    , finished(false)
    {
    }
    
    bool hasKeyframeAtTime(int time) const
    {
        // PRIVATE - should not lock
        
        if (points.empty()) {
            return false;
        } else {
            KeyFrame k;
            return points.front()->hasKeyFrameAtTime(time);
        }
    }
    
    void getKeyframeTimes(std::set<int>* times) const
    {
        // PRIVATE - should not lock
       
        if (points.empty()) {
            return ;
        }
        points.front()->getKeyframeTimes(times);
    }
    
    BezierCPs::const_iterator atIndex(int index) const
    {
        // PRIVATE - should not lock
        
        if (index >= (int)points.size() || index < 0) {
            throw std::out_of_range("RotoSpline::atIndex: non-existent control point");
        }
        
        BezierCPs::const_iterator it = points.begin();
        std::advance(it, index);
        return it;
    }
    
    BezierCPs::iterator atIndex(int index)
    {
        // PRIVATE - should not lock
        
        if (index >= (int)points.size() || index < 0) {
            throw std::out_of_range("RotoSpline::atIndex: non-existent control point");
        }
        
        BezierCPs::iterator it = points.begin();
        std::advance(it, index);
        return it;
    }
    
    BezierCPs::const_iterator findControlPointNearby(double x,double y,double acceptance,int time,int* index) const
    {
         // PRIVATE - should not lock
        int i = 0;
        for (BezierCPs::const_iterator it = points.begin(); it != points.end(); ++it,++i) {
            double pX,pY;
            (*it)->getPositionAtTime(time, &pX, &pY);
            if (pX >= (x - acceptance) && pX <= (x + acceptance) && pY >= (y - acceptance) && pY <= (y + acceptance)) {
                *index = i;
                return it;
            }
        }
        return points.end();
    }
    
    BezierCPs::const_iterator findFeatherPointNearby(double x,double y,double acceptance,int time,int* index) const
    {
         // PRIVATE - should not lock
        int i = 0;
        for (BezierCPs::const_iterator it = featherPoints.begin(); it != featherPoints.end(); ++it,++i) {
            double pX,pY;
            (*it)->getPositionAtTime(time, &pX, &pY);
            if (pX >= (x - acceptance) && pX <= (x + acceptance) && pY >= (y - acceptance) && pY <= (y + acceptance)) {
                *index = i;
                return it;
            }
        }
        return featherPoints.end();
    }
};

class RotoLayer;
struct RotoItemPrivate
{
    RotoContext* context;
    std::string name;
    RotoLayer* parentLayer;
    
    ////This controls whether the item (and all its children if it is a layer)
    ////should be visible/rendered or not at any time.
    ////This is different from the "activated" knob for RotoDrawableItem's which in that
    ////case allows to define a life-time
    bool globallyActivated;
    
    ////A locked item should not be modifiable by the GUI
    bool locked;
    
    RotoItemPrivate(RotoContext* context,const std::string& n,RotoLayer* parent)
    : context(context)
    , name(n)
    , parentLayer(parent)
    , globallyActivated(true)
    , locked(false)
    {}
    
    

};

typedef std::list< boost::shared_ptr<RotoItem> > RotoItems;

struct RotoLayerPrivate
{
    RotoItems items;
    
    RotoLayerPrivate()
    : items()
    {
        
    }
};

struct RotoDrawableItemPrivate
{
    double overlayColor[4]; //< the color the shape overlay should be drawn with, defaults to smooth red
    
    
    boost::shared_ptr<Double_Knob> opacity; //< opacity of the rendered shape between 0 and 1
    boost::shared_ptr<Int_Knob> feather;//< number of pixels to add to the feather distance (from the feather point), between -100 and 100
    boost::shared_ptr<Double_Knob> featherFallOff; //< the rate of fall-off for the feather, between 0 and 1,  0.5 meaning the
                                                   //alpha value is half the original value when at half distance from the feather distance
    boost::shared_ptr<Bool_Knob> activated; //< should the curve be visible/rendered ? (animable)
    boost::shared_ptr<Bool_Knob> inverted; //< invert the rendering
    
    RotoDrawableItemPrivate()
    : opacity(new Double_Knob(NULL,"Opacity",1,false))
    , feather(new Int_Knob(NULL,"Feather",1,false))
    , featherFallOff(new Double_Knob(NULL,"Feather fall-off",1,false))
    , activated(new Bool_Knob(NULL,"Activated",1,false))
    , inverted(new Bool_Knob(NULL,"Inverted",1,false))
    {
        opacity->populate();
        opacity->setDefaultValue(1.);
        {
            boost::shared_ptr<KnobSignalSlotHandler> handler(new KnobSignalSlotHandler(opacity));
            opacity->setSignalSlotHandler(handler);
        }
        
        feather->populate();
        feather->setDefaultValue(0);
        {
            boost::shared_ptr<KnobSignalSlotHandler> handler(new KnobSignalSlotHandler(feather));
            feather->setSignalSlotHandler(handler);
        }

        featherFallOff->populate();
        featherFallOff->setDefaultValue(1.);
        {
            boost::shared_ptr<KnobSignalSlotHandler> handler(new KnobSignalSlotHandler(featherFallOff));
            featherFallOff->setSignalSlotHandler(handler);
        }
        
        activated->populate();
        activated->setDefaultValue(true);
        {
            boost::shared_ptr<KnobSignalSlotHandler> handler(new KnobSignalSlotHandler(activated));
            activated->setSignalSlotHandler(handler);
        }

        inverted->populate();
        inverted->setDefaultValue(false);
        {
            boost::shared_ptr<KnobSignalSlotHandler> handler(new KnobSignalSlotHandler(inverted));
            inverted->setSignalSlotHandler(handler);
        }
       
        
        overlayColor[0] = 0.85164;
        overlayColor[1] = 0.196936;
        overlayColor[2] = 0.196936;
        overlayColor[3] = 1.;
        
    }

    ~RotoDrawableItemPrivate() {}
};

struct RotoContextPrivate
{
    
    mutable QMutex rotoContextMutex;
    std::list< boost::shared_ptr<RotoLayer> > layers;
    bool autoKeying;
    bool rippleEdit;
    bool featherLink;
    
    Natron::Node* node;
    U64 age;
    
    ///These are knobs that take the value of the selected splines infos.
    ///Their value changes when selection changes.
    boost::shared_ptr<Double_Knob> opacity;
    boost::shared_ptr<Int_Knob> feather;
    boost::shared_ptr<Double_Knob> featherFallOff;
    boost::shared_ptr<Bool_Knob> activated; //<allows to disable a shape on a specific frame range
    boost::shared_ptr<Bool_Knob> inverted;
    
    ////For each base item ("Rectangle","Ellipse","Bezier", etc...) a basic countr
    ////to give a unique default name to each shape
    std::map<std::string, int> itemCounters;
    
    
    ///This keeps track  of the items linked to the context knobs
    std::list<boost::shared_ptr<RotoItem> > selectedItems;
    
    boost::shared_ptr<RotoItem> lastInsertedItem;
    boost::shared_ptr<RotoItem> lastLockedItem;
    
    
    QMutex lastRenderArgsMutex; //< protects lastRenderArgs & lastRenderedImage
    U64 lastRenderHash;
    boost::shared_ptr<Natron::Image> lastRenderedImage;
    
    RotoContextPrivate(Natron::Node* n )
    : rotoContextMutex()
    , layers()
    , autoKeying(true)
    , rippleEdit(false)
    , featherLink(true)
    , node(n)
    , age(0)
    {
        
        assert(n && n->getLiveInstance());
        Natron::EffectInstance* effect = n->getLiveInstance();
        opacity = Natron::createKnob<Double_Knob>(effect, "Opacity",1,false);
        opacity->setHintToolTip("Controls the opacity of the selected shape(s).");
        opacity->setMinimum(0.);
        opacity->setMaximum(1.);
        opacity->setDisplayMinimum(0.);
        opacity->setDisplayMaximum(1.);
        opacity->setDefaultValue(1.);
        opacity->setAllDimensionsEnabled(false);
        opacity->setIsPersistant(false);
        feather = Natron::createKnob<Int_Knob>(effect,"Feather",1,false);
        feather->setHintToolTip("Controls the distance of feather (in pixels) to add around the selected shape(s)");
        feather->setMinimum(-100);
        feather->setMaximum(100);
        feather->setDisplayMinimum(-100);
        feather->setDisplayMaximum(100);
        feather->setDefaultValue(0);
        feather->setAllDimensionsEnabled(false);
        feather->setIsPersistant(false);
        featherFallOff = Natron::createKnob<Double_Knob>(effect, "Feather fall-off",1,false);
        featherFallOff->setHintToolTip("Controls the rate at which the feather is applied on the selected shape(s).");
        featherFallOff->setMinimum(0.2);
        featherFallOff->setMaximum(5.);
        featherFallOff->setDisplayMinimum(0.2);
        featherFallOff->setDisplayMaximum(5.);
        featherFallOff->setDefaultValue(1.);
        featherFallOff->setAllDimensionsEnabled(false);
        featherFallOff->setIsPersistant(false);
        activated = Natron::createKnob<Bool_Knob>(effect,"Activated",1,false);
        activated->setHintToolTip("Controls whether the selected shape(s) should be visible and rendered or not."
                                  "Note that you can animate this parameter so you can activate/deactive the shape "
                                  "throughout the time.");
        activated->turnOffNewLine();
        activated->setDefaultValue(true);
        activated->setAllDimensionsEnabled(false);
        activated->setIsPersistant(false);
        inverted = Natron::createKnob<Bool_Knob>(effect, "Inverted",1,false);
        inverted->setHintToolTip("Controls whether the selected shape(s) should be inverted. When inverted everything "
                                 "outside the shape will be set to 1 and everything inside the shape will be set to 0.");
        inverted->setDefaultValue(false);
        inverted->setAllDimensionsEnabled(false);
        inverted->setIsPersistant(false);

    }
    
    /**
     * @brief Call this after any change to notify the mask has changed for the cache.
     **/
    void incrementRotoAge()
    {
        ///MT-safe: only called on the main-thread
        assert(QThread::currentThread() == qApp->thread());
        
        QMutexLocker l(&rotoContextMutex);
        ++age;
    }
    
   
    void renderInternal(cairo_t* cr,cairo_surface_t* cairoImg,const std::list< boost::shared_ptr<Bezier> >& splines,
                        unsigned int mipmapLevel,int time);
        
    
};


#endif // ROTOCONTEXTPRIVATE_H
