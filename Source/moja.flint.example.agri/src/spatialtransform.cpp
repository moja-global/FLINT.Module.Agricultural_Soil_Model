#include "moja/flint/example/agri/spatialtransform.h"

#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/ivariable.h"
#include "moja/flint/spatiallocationinfo.h"
#include "moja/flint/flintexceptions.h"

#include <moja/datarepository/datarepository.h>
#include <moja/datarepository/iproviderspatialrasterinterface.h>
#include <moja/logging.h>

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
      DynamicObject change;
      change.insert<std::string>("type", "landuse");
      change.insert<std::string>("data_property", "data_yearly");
      std::vector<DynamicVar> landtype{"F", "C", "C", "G", "G", "G"};
      change.insert<std::vector<DynamicVar>>("landtype", landtype);
      std::vector<DynamicVar> landuse{"", "Long term cultivated", "Perennial crop", "", "", ""};
      change.insert<std::vector<DynamicVar>>("landuse", landuse);
      std::vector<DynamicVar> management{
          "", "Reduced", "Full", "High intensity grazing", "High intensity grazing", "High intensity grazing"};
      change.insert<std::vector<DynamicVar>>("management", management);
      std::vector<DynamicVar> input{"", "Medium", "Medium", "High", "High", "High"};
      change.insert<std::vector<DynamicVar>>("input", input);

      _cachedValue = change;
   } else if (_dataPropertyName == "prp") {
      DynamicObject animals;
      animals.insert<DateTime>("date", DateTime(2003, 8, 1));
      animals.insert<std::string>("type", "agri.PRPEvent");
      animals.insert<std::string>("animal", "Buffalo");
      animals.insert<int>("number", 3);
      animals.insert<std::string>("productivity_class", "High");
      animals.insert<std::string>("use", "Other");
      
      events.emplace_back(animals);
      _cachedValue = events;
   }

   return _cachedValue;
}

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja