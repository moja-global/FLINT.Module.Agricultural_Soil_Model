#include "moja/flint/example/agri/spatialtransform.h"

#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/ivariable.h"
#include "moja/flint/spatiallocationinfo.h"
#include "moja/flint/flintexceptions.h"

#include <moja/datarepository/datarepository.h>
#include <moja/datarepository/iproviderspatialrasterinterface.h>

#include <boost/algorithm/string.hpp>

namespace moja {
namespace flint {
namespace example {
namespace agri {

// --------------------------------------------------------------------------------------------
void SpatialTransform::configure(DynamicObject config, const ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   _landUnitController = &landUnitController;
   _spatialLocationInfo = std::static_pointer_cast<SpatialLocationInfo>(
       landUnitController.getVariable("spatialLocationInfo")->value().extract<std::shared_ptr<flint::IFlintData>>());

   _dataPropertyName = "data";
   if (config.contains("data_property")) {
      _dataPropertyName = config["data_property"].extract<std::string>();
   }
}

void SpatialTransform::controllerChanged(const ILandUnitController& landUnitController) {
   _landUnitController = &landUnitController;
   _spatialLocationInfo = std::static_pointer_cast<SpatialLocationInfo>(
       landUnitController.getVariable("spatialLocationInfo")->value().extract<std::shared_ptr<flint::IFlintData>>());
};

const DynamicVar& SpatialTransform::value() const {
   /*datarepository::CellIdx cell(_spatialLocationInfo->_tileIdx, _spatialLocationInfo->_blockIdx,
                                _spatialLocationInfo->_cellIdx, *_indexer); */
   _cachedValue = nullptr;
   DynamicVector events;

   if (_dataPropertyName == "harvest") {
      DynamicObject event1;
      event1.insert<std::string>("type", "agri.PlantEvent");
      event1.insert<DateTime>("date", DateTime(2002, 3, 1));
      event1.insert<std::string>("name", "Winter Wheat");

      DynamicObject event2;
      event2.insert<std::string>("type", "agri.HarvestEvent");
      event2.insert<DateTime>("date", DateTime(2002, 7, 1));
      event2.insert<double>("frac_burnt", 0);
      event2.insert<double>("frac_remove", 0);
      event2.insert<double>("frac_renew", 0);

      events.emplace_back(event1);
      events.emplace_back(event2);
      _cachedValue = events;
   } else if (_dataPropertyName == "fertilizer") {
      DynamicObject event1;
      event1.insert<std::string>("type", "agri.NFertEvent");
      event1.insert<DateTime>("date", DateTime(2000, 1, 1));
      event1.insert<std::string>("name", "Synthetic fertilizer");
      event1.insert<double>("quantity", 100);
      event1.insert<double>("runtime", 5);

      DynamicObject event2;
      event2.insert<std::string>("type", "agri.NFertEvent");
      event2.insert<DateTime>("date", DateTime(2001, 1, 1));
      event2.insert<std::string>("name", "Organic fertilizer");
      event2.insert<double>("quantity", 100);
      event2.insert<double>("runtime", 5);

      events.emplace_back(event1);
      events.emplace_back(event2);
      _cachedValue = events;
   } else if (_dataPropertyName == "landuse") {
      DynamicObject change1;
      change1.insert<std::string>("type", "landuse");
      change1.insert<DateTime>("date", DateTime(2001, 1, 1));
      change1.insert<std::string>("landuse", "F");
      change1.insert<std::string>("management", "");
      change1.insert<std::string>("input", "");

      DynamicObject change2;
      change2.insert<std::string>("type", "landuse");
      change2.insert<DateTime>("date", DateTime(2003, 1, 1));
      change2.insert<std::string>("landuse", "C");
      change2.insert<std::string>("management", "Perennial crop");
      change2.insert<std::string>("input", "Full");

      events.emplace_back(change1);
      events.emplace_back(change2);
      _cachedValue = events;
   }
   
   return _cachedValue;
}

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja