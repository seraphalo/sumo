/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEVehicleFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2018
/// @version $Id$
///
// The Widget for add Vehicles/Flows/Trips/etc. elements
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/netelements/GNEEdge.h>
#include <netedit/netelements/GNELane.h>
#include <netedit/netelements/GNEConnection.h>
#include <netedit/demandelements/GNEVehicle.h>
#include <netedit/demandelements/GNERouteHandler.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>

#include "GNEVehicleFrame.h"

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEVehicleFrame::VTypeSelector) VTypeSelectorMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_SET_TYPE,    GNEVehicleFrame::VTypeSelector::onCmdSelectVType),
};

// Object implementation
FXIMPLEMENT(GNEVehicleFrame::VTypeSelector, FXGroupBox, VTypeSelectorMap,   ARRAYNUMBER(VTypeSelectorMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEVehicleFrame::VTypeSelector - methods
// ---------------------------------------------------------------------------

GNEVehicleFrame::VTypeSelector::VTypeSelector(GNEVehicleFrame* vehicleFrameParent) :
    FXGroupBox(vehicleFrameParent->myContentFrame, "Vehicle Type", GUIDesignGroupBoxFrame),
    myVehicleFrameParent(vehicleFrameParent),
    myCurrentVehicleType(nullptr) {
    // Create FXComboBox
    myTypeMatchBox = new FXComboBox(this, GUIDesignComboBoxNCol, this, MID_GNE_SET_TYPE, GUIDesignComboBox);
    /*
    // fill myTypeMatchBox with list of tags
    for (const auto& i : myListOfTags) {
        myTypeMatchBox->appendItem(toString(i).c_str());
    }
    */
    // Set visible items
    myTypeMatchBox->setNumVisible((int)myTypeMatchBox->getNumItems());
    // VTypeSelector is always shown
    show();
}


GNEVehicleFrame::VTypeSelector::~VTypeSelector() {}


const GNEVehicleType*
GNEVehicleFrame::VTypeSelector::getCurrentVehicleType() const {
    return myCurrentVehicleType;
}


void 
GNEVehicleFrame::VTypeSelector::showVTypeSelector() {
    show();
}


void 
GNEVehicleFrame::VTypeSelector::hideVTypeSelector() {
    hide();
}


long
GNEVehicleFrame::VTypeSelector::onCmdSelectVType(FXObject*, FXSelector, void*) {
    // Check if value of myTypeMatchBox correspond of an allowed additional tags
    /*
    for (const auto& i : myListOfTags) {
        if (toString(i) == myTypeMatchBox->getText().text()) {
            // set color of myTypeMatchBox to black (valid)
            myTypeMatchBox->setTextColor(FXRGB(0, 0, 0));
            // Set new current type
            myCurrentTagProperties = GNEAttributeCarrier::getTagProperties(i);
            // show moduls if selected item is valid
            myFrameParent->enableModuls(myCurrentTagProperties);
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(("Selected item '" + myTypeMatchBox->getText() + "' in VTypeSelector").text());
            return 1;
        }
    }

    // if additional name isn't correct, set SUMO_TAG_NOTHING as current type
    myCurrentTagProperties = myInvalidTagProperty;
    */
    // hide all moduls if selected item isn't valid
    myVehicleFrameParent->disableModuls();
    // set color of myTypeMatchBox to red (invalid)
    myTypeMatchBox->setTextColor(FXRGB(255, 0, 0));
    // Write Warning in console if we're in testing mode
    WRITE_DEBUG("Selected invalid item in VTypeSelector");
    return 1;
}

// ---------------------------------------------------------------------------
// GNEVehicleFrame::HelpCreation - methods
// ---------------------------------------------------------------------------

GNEVehicleFrame::HelpCreation::HelpCreation(GNEVehicleFrame* vehicleFrameParent) :
    FXGroupBox(vehicleFrameParent->myContentFrame, "Help", GUIDesignGroupBoxFrame),
    myVehicleFrameParent(vehicleFrameParent) {
}


GNEVehicleFrame::HelpCreation::~HelpCreation() {}


void 
GNEVehicleFrame::HelpCreation::showHelpCreation() {
    show();
}


void 
GNEVehicleFrame::HelpCreation::hideHelpCreation() {
    hide();
}

void 
GNEVehicleFrame::HelpCreation::updateHelpCreation() {

}

// ---------------------------------------------------------------------------
// GNEVehicleFrame - methods
// ---------------------------------------------------------------------------

GNEVehicleFrame::GNEVehicleFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Vehicles") {

    // Create item Selector modul for vehicles
    myItemSelector = new ItemSelector(this, GNEAttributeCarrier::TAGProperty::TAGPROPERTY_VEHICLE);

    // Create vehicle type selector
    myVTypeSelector = new VTypeSelector(this);

    // Create vehicle parameters
    myVehicleAttributes = new ACAttributes(this);

    // Create Netedit parameter
    myNeteditAttributes = new NeteditAttributes(this);

    // Create Help Creation Modul
    myHelpCreation = new HelpCreation(this);

    // set Vehicle as default vehicle
    myItemSelector->setCurrentTypeTag(SUMO_TAG_VEHICLE);
}


GNEVehicleFrame::~GNEVehicleFrame() {}


void
GNEVehicleFrame::show() {
    // refresh item selector
    myItemSelector->refreshTagProperties();
    // show frame
    GNEFrame::show();
}


bool
GNEVehicleFrame::addVehicle(const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // first check that current selected vehicle is valid
    if (myItemSelector->getCurrentTagProperties().getTag() == SUMO_TAG_NOTHING) {
        myViewNet->setStatusBarText("Current selected vehicle isn't valid.");
        return false;
    }

    // obtain tagproperty (only for improve code legibility)
    const auto& tagValues = myItemSelector->getCurrentTagProperties();

    // Declare map to keep attributes from Frames from Frame
    std::map<SumoXMLAttr, std::string> valuesMap = myVehicleAttributes->getAttributesAndValues();

    // fill netedit attributes
    if (!myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, objectsUnderCursor.getLaneFront())) {
        return false;
    }

    // If consecutive Lane Selector is enabled, it means that either we're selecting lanes or we're finished or we'rent started
    if (tagValues.canBePlacedOverEdge()) {
        return buildVehicleOverEdge(valuesMap, objectsUnderCursor.getLaneFront(), tagValues);
    } else if (tagValues.canBePlacedOverLane()) {
        return buildVehicleOverLane(valuesMap, objectsUnderCursor.getLaneFront(), tagValues);
    } else {
        return buildVehicleOverView(valuesMap, tagValues);
    }
}


void
GNEVehicleFrame::enableModuls(const GNEAttributeCarrier::TagProperties& tagProperties) {
    // show vehicle type selector modul
    myVTypeSelector->showVTypeSelector();
    // show vehicle attributes modul
    myVehicleAttributes->showACAttributesModul(tagProperties);
    // show netedit attributes
    myNeteditAttributes->showNeteditAttributesModul(tagProperties);
}


void
GNEVehicleFrame::disableModuls() {
    // hide all moduls if vehicle isn't valid
    myVTypeSelector->hideVTypeSelector();
    myVehicleAttributes->hideACAttributesModul();
    myNeteditAttributes->hideNeteditAttributesModul();
}


std::string
GNEVehicleFrame::generateID(GNENetElement* netElement) const {
    // obtain current number of vehicles to generate a new index faster
    int vehicleIndex = myViewNet->getNet()->getNumberOfDemandElements(myItemSelector->getCurrentTagProperties().getTag());
    // obtain tag Properties (only for improve code legilibility
    const auto& tagProperties = myItemSelector->getCurrentTagProperties();
    if (netElement) {
        // generate ID using netElement
        while (myViewNet->getNet()->retrieveDemandElement(tagProperties.getTag(), tagProperties.getTagStr() + "_" + netElement->getID() + "_" + toString(vehicleIndex), false) != nullptr) {
            vehicleIndex++;
        }
        return tagProperties.getTagStr() + "_" + netElement->getID() + "_" + toString(vehicleIndex);
    } else {
        // generate ID without netElement
        while (myViewNet->getNet()->retrieveDemandElement(tagProperties.getTag(), tagProperties.getTagStr() + "_" + toString(vehicleIndex), false) != nullptr) {
            vehicleIndex++;
        }
        return tagProperties.getTagStr() + "_" + toString(vehicleIndex);
    }
}


bool
GNEVehicleFrame::buildVehicleCommonAttributes(std::map<SumoXMLAttr, std::string>& valuesMap, const GNEAttributeCarrier::TagProperties& tagValues) {
    // If vehicle has a interval defined by a begin or end, check that is valid
    if (tagValues.hasAttribute(SUMO_ATTR_STARTTIME) && tagValues.hasAttribute(SUMO_ATTR_END)) {
        double begin = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_STARTTIME]);
        double end = GNEAttributeCarrier::parse<double>(valuesMap[SUMO_ATTR_END]);
        if (begin > end) {
            myVehicleAttributes->showWarningMessage("Attribute '" + toString(SUMO_ATTR_STARTTIME) + "' cannot be greater than attribute '" + toString(SUMO_ATTR_END) + "'.");
            return false;
        }
    }
    // If vehicle own the attribute SUMO_ATTR_FILE but was't defined, will defined as <ID>.xml
    if (tagValues.hasAttribute(SUMO_ATTR_FILE) && valuesMap[SUMO_ATTR_FILE] == "") {
        if ((myItemSelector->getCurrentTagProperties().getTag() != SUMO_TAG_CALIBRATOR) && (myItemSelector->getCurrentTagProperties().getTag() != SUMO_TAG_REROUTER)) {
            // SUMO_ATTR_FILE is optional for calibrators and rerouters (fails to load in sumo when given and the file does not exist)
            valuesMap[SUMO_ATTR_FILE] = (valuesMap[SUMO_ATTR_ID] + ".xml");
        }
    }
    // all ok, continue building vehicles
    return true;
}


bool
GNEVehicleFrame::buildVehicleOverEdge(std::map<SumoXMLAttr, std::string>& valuesMap, GNELane* lane, const GNEAttributeCarrier::TagProperties& tagValues) {
    // check that edge exist
    if (lane) {
        // Get attribute lane's edge
        valuesMap[SUMO_ATTR_EDGE] = lane->getParentEdge().getID();
        // Generate id of element based on the lane's edge
        valuesMap[SUMO_ATTR_ID] = generateID(&lane->getParentEdge());
    } else {
        return false;
    }
    // parse common attributes
    if (!buildVehicleCommonAttributes(valuesMap, tagValues)) {
        return false;
    }
    // show warning dialogbox and stop check if input parameters are valid
    if (!myVehicleAttributes->areValuesValid()) {
        myVehicleAttributes->showWarningMessage();
        return false;
    } else {
        
        // GNEVehicleHandler::buildVehicle(myViewNet, true, myItemSelector->getCurrentTagProperties().getTag(), valuesMap) != nullptr) {

        return true;
    }
}


bool
GNEVehicleFrame::buildVehicleOverLane(std::map<SumoXMLAttr, std::string>& valuesMap, GNELane* lane, const GNEAttributeCarrier::TagProperties& tagValues) {
    // check that lane exist
    if (lane != nullptr) {
        // Get attribute lane
        valuesMap[SUMO_ATTR_LANE] = lane->getID();
        // Generate id of element based on the lane
        valuesMap[SUMO_ATTR_ID] = generateID(lane);
    } else {
        return false;
    }
    // Obtain position of the mouse over lane (limited over grid)
    double mousePositionOverLane = lane->getShape().nearest_offset_to_point2D(myViewNet->snapToActiveGrid(myViewNet->getPositionInformation())) / lane->getLengthGeometryFactor();
    // set attribute position as mouse position over lane
    valuesMap[SUMO_ATTR_POSITION] = toString(mousePositionOverLane);
    // parse common attributes
    if (!buildVehicleCommonAttributes(valuesMap, tagValues)) {
        return false;
    }
    // show warning dialogbox and stop check if input parameters are valid
    if (!myVehicleAttributes->areValuesValid()) {
        myVehicleAttributes->showWarningMessage();
        return false;
    } else {

        /// if (GNEVehicleHandler::buildVehicle(myViewNet, true, myItemSelector->getCurrentTagProperties().getTag(), valuesMap)
       
        return true;
    }
}


bool
GNEVehicleFrame::buildVehicleOverView(std::map<SumoXMLAttr, std::string>& valuesMap, const GNEAttributeCarrier::TagProperties& tagValues) {
    // Generate id of element
    valuesMap[SUMO_ATTR_ID] = generateID(nullptr);
    // Obtain position as the clicked position over view
    valuesMap[SUMO_ATTR_POSITION] = toString(myViewNet->snapToActiveGrid(myViewNet->getPositionInformation()));
    // parse common attributes
    if (!buildVehicleCommonAttributes(valuesMap, tagValues)) {
        return false;
    }
    // show warning dialogbox and stop check if input parameters are valid
    if (myVehicleAttributes->areValuesValid() == false) {
        myVehicleAttributes->showWarningMessage();
        return false;
    } else { 
        
        // (GNEVehicleHandler::buildVehicle(myViewNet, true, myItemSelector->getCurrentTagProperties().getTag(), valuesMap)) {
        return true;
    }
}

/****************************************************************************/