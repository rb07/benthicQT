/* Copyright 2010 NVIDIA Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Developed by Mustard Seed Software, LLC 
 * http://mseedsoft.com
 */

#include <osg/Drawable>
#include <osg/ShapeDrawable>
#include <osg/PolygonMode>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/CullFace>
#include <osgDB/ReadFile>

#include <osgManipulator/Translate2DDragger>
#include "PositionHandler.h"
#include "MeshGeom.h"
#include "PotentialUpdater.h"
#include "BQTDebug.h"
#include "BarrierSet.h"
#include "SimulationState.h"
#include "ProgressBar.h"

namespace ews {
    namespace app {
        namespace drawable {
            using namespace osg;
            using namespace osgManipulator;
            using namespace app::model;
            

            
            /** Primary constructor. */
            MeshGeom::MeshGeom(MeshFile& dataModel)
            : DrawableQtAdapter(), _dataModel(dataModel), _switch(new Switch),
            _meshGeom(new PositionAttitudeTransform)
       {
                
//qDebug() << " In Mesh Geom const";

                _switch->setNewChildDefaultValue(true);
                addChild(_switch.get());
                _switch->addChild(_meshGeom.get());

                addEventCallback(new PositionHandler(&_dataModel,_dataModel.getLatOrigin(),_dataModel.getLongOrigin()));
                
              updateGeom();
//                qDebug() << "In contstrt mesh gerom";
                // Callback to detect when we've been moved
                // and update the databmodel.
         //       setUpdateCallback(new PotentialUpdater);
                
                respondToSignals(true);
            }
            
            
            MeshGeom::~MeshGeom() {
            }
            
            void MeshGeom::respondToSignals(bool respond) {
                if(respond) {
               //    connect(&_dataModel, SIGNAL(dataChanged()), SLOT(updateGeom()));//, Qt::UniqueConnection);
                }
                else {
                    _dataModel.disconnect(this);
                }
            }
            


        void MeshGeom::changeOverlay(int){

        }

            void MeshGeom::setEnabled(bool enabled) {
                if(enabled) {
                    _switch->setAllChildrenOn();
                }
                else {
                    _switch->setAllChildrenOff();
                }
            }
            
            void MeshGeom::updateGeom() {
                //qDebug() << " In Mesh Geom update";

                _meshGeom->removeChildren(0, _meshGeom->getNumChildren());
                QStringList list=_dataModel.getFileNames();
                qDebug() << "Number of files  "<<_dataModel.getFileNames().size();
                QStringList::Iterator it = list.begin();
                while( it != list.end() ) {
                   //_dataModel.getPBarD()->reset();

                    qDebug() << "Loading " << *it;
                string filename = it->toStdString();
                //osgDB::Registry::instance()->setReadFileCallback(new MyReadCallback(*sseh));
                osg::ref_ptr<osgDB::ReaderWriter> rw = osgDB::Registry::instance()->getReaderWriterForExtension(osgDB::getLowerCaseFileExtension(filename));
                std::auto_ptr<progbuf> pb(new progbuf(osgDB::findDataFile(filename),_dataModel.getPBarD()));
                if (!rw || !pb->is_open())
                {
                    string errmsg= "Error: could not open file `" + filename +"'" +" Might be plugin not found\n";
                    QString errStr=errmsg.c_str();
                    std::cerr << errmsg;
                    errorD.showMessage(errStr);
                    errorD.exec();
                    ++it;
                    continue;
                }
                _dataModel.getPBarD()->setLabelText("Loading Mesh: "+*it);
                 _dataModel.getPBarD()->setRange(0,100);
                _dataModel.getPBarD()->show();
                osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = new osgDB::ReaderWriter::Options;
                local_opt->setDatabasePath(osgDB::getFilePath(filename));
                std::istream mis(pb.get());

                osgDB::ReaderWriter::ReadResult rr = rw->readNode(mis,local_opt);
                if (rr.validNode()) {
                    osg::ref_ptr<osg::Node> node = rr.getNode();
                    if(!node.valid()){
                        string errmsg= "Invalid Node: could not open file `" + filename +"'";
                        QString errStr=errmsg.c_str();
                        std::cerr << errmsg;
                        errorD.showMessage(errStr);
                        errorD.exec();
                    }
                    osg::StateSet *ss=_meshGeom->getOrCreateStateSet();
                    if(ss)
                    ss->addUniform(_dataModel.getShaderOutUniform());
                    _meshGeom->addChild(node.get());
                    setEnabled(true);

                }else{
                    if (rr.error()) {
                        string errmsg= "Error: could not open file `" + filename +"'" +rr.message();
                        QString errStr=errmsg.c_str();
                        std::cerr << errmsg;
                        errorD.showMessage(errStr);
                        errorD.exec();
                    }
                }
                ++it;
            }



            }


        }
    }
}
