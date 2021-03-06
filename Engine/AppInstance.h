//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 *Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 *contact: immarespond at gmail dot com
 *
 */

#ifndef APPINSTANCE_H
#define APPINSTANCE_H


#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <QStringList>

#include "Global/GlobalDefines.h"

namespace boost {
    namespace archive {
        class xml_iarchive;
        class xml_oarchive;
    }
}

class NodeSerialization;
class TimeLine;
struct AppInstancePrivate;
class ProcessHandler;
class VideoEngine;
namespace Natron {
    class Node;
    class Project;
    class EffectInstance;
    class OutputEffectInstance;
}


class AppInstance : public QObject , public boost::noncopyable
{
    Q_OBJECT
public:
    
    
    
    AppInstance(int appID);
    
    virtual ~AppInstance();
    
    virtual void aboutToQuit(){}
    
    virtual void load(const QString& projectName = QString(),const QStringList& writers = QStringList());
    
    int getAppID() const;
        
    /** @brief Create a new node  in the node graph.
     * The name passed in parameter must match a valid node name,
     * otherwise an exception is thrown. You should encapsulate the call
     * by a try-catch block.
     * If the majorVersion is not -1 then this function will attempt to find a plugin with the matching
     * majorVersion, or otherwise it will throw an exception.
     * If the minorVersion is not -1 then this function will attempt to load a plugin with the greatest minorVersion
     * greater or equal to this minorVersion.
     * By default this function also create the node's graphical user interface and attempts to automatically
     * connect this node to other nodes selected.
     * If requestedByLoad is true then it will never attempt to do this auto-connection.
     * If openImageFileDialog is true then if the node has a file knob indicating an image file it will automatically
     * prompt the user with a file dialog.
     *
     * @param createGui If false, the node GUI (in the nodegraph and in the properties panel) will not be created.
     * The node animation curves will still be available in the curve editor though.
     *
     * You can use this function to create backdrops also which are purely GUI stuff. In this case the pointer returned will
     * be NULL.
     **/
    boost::shared_ptr<Natron::Node> createNode(const QString& name,bool createGui = true,int majorVersion = -1,int minorVersion = -1,
                                               bool openImageFileDialog = true);
    
    ///Same as createNode but used when loading a project
    boost::shared_ptr<Natron::Node> loadNode(const QString& name,bool createGui,int majorVersion,int minorVersion,const NodeSerialization& serialization, bool dontLoadName);

    void getActiveNodes(std::vector<boost::shared_ptr<Natron::Node> > *activeNodes) const;

    boost::shared_ptr<Natron::Project> getProject() const;

    boost::shared_ptr<TimeLine> getTimeLine() const;

    /*true if the user is NOT scrubbing the timeline*/
    virtual bool shouldRefreshPreview() const { return false; }
    
    void connectViewersToViewerCache();

    void disconnectViewersFromViewerCache();

    virtual void errorDialog(const std::string& title,const std::string& message) const;

    virtual void warningDialog(const std::string& title,const std::string& message) const;

    virtual void informationDialog(const std::string& title,const std::string& message) const;

    virtual Natron::StandardButton questionDialog(const std::string& title,const std::string& message,Natron::StandardButtons buttons =
                                      Natron::StandardButtons(Natron::Yes | Natron::No),
                                      Natron::StandardButton defaultButton = Natron::NoButton) const WARN_UNUSED_RETURN;
    
    virtual void loadProjectGui(boost::archive::xml_iarchive& /*archive*/) const {}

    virtual void saveProjectGui(boost::archive::xml_oarchive& /*archive*/) {}
    
    virtual void setupViewersForViews(int /*viewsCount*/) {}
    
    virtual void notifyRenderProcessHandlerStarted(const QString& /*sequenceName*/,
                                                   int /*firstFrame*/,int /*lastFrame*/,
                                                   const boost::shared_ptr<ProcessHandler>& /*process*/) {}

    virtual bool isShowingDialog() const { return false; }
    
    virtual void startProgress(Natron::EffectInstance* /*effect*/,const std::string& /*message*/) {}
    
    virtual void endProgress(Natron::EffectInstance* /*effect*/) {}
    
    virtual bool progressUpdate(Natron::EffectInstance* /*effect*/,double /*t*/) {return true;}
    
    /**
     * @brief Checks for a new version of Natron
     **/
    void checkForNewVersion() const;
    
    virtual void onMaxPanelsOpenedChanged(int /*maxPanels*/) {}
    
    Natron::ViewerColorSpace getDefaultColorSpaceForBitDepth(Natron::ImageBitDepth bitdepth) const;
    
    int getMainView() const;
    
    /**
     * @brief This is to overcome possible deadlocks when a plug-in is trying to show a dialog
     * in the render thread (hence requesting the main-thread to show it) and the main thread
     * is trying to abort that exact render thread.
     **/
    virtual void registerVideoEngineBeingAborted(VideoEngine* /*engine*/) {}
    virtual void unregisterVideoEngineBeingAborted(VideoEngine* /*engine*/) {}
    
public slots:
    
    void quit();
    
    /* The following methods are forwarded to the model */
    void checkViewersConnection();
    
    void redrawAllViewers();
    
    void triggerAutoSave();
    
    /*Used in background mode only*/
    void startWritersRendering(const QStringList& writers);
    
    void clearOpenFXPluginsCaches();
    
    void clearAllLastRenderedImages();
    
    void clearViewersLastRenderedTexture();
    
    void newVersionCheckDownloaded();
    
    void newVersionCheckError();
    
signals:
    
    void pluginsPopulated();
    
protected:
    
    virtual void createNodeGui(boost::shared_ptr<Natron::Node> /*node*/,bool /*createGui*/,bool /*loadRequest*/,bool /*openImageFileDialog*/) {}
    
    virtual void startRenderingFullSequence(Natron::OutputEffectInstance* writer);

private:
    
    /**
     * @brief Override to create a backdrop node. This is purely GUI.
     **/
    virtual void createBackDrop() {}
    
    
    boost::shared_ptr<Natron::Node> createNodeInternal(const QString& pluginID,bool createGui,int majorVersion,int minorVersion,
                                     bool requestedByLoad,bool openImageFileDialog,const NodeSerialization& serialization,bool dontLoadName);
    
    boost::scoped_ptr<AppInstancePrivate> _imp;
    
};


#endif // APPINSTANCE_H
