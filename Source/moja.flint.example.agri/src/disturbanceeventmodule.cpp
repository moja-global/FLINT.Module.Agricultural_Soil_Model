#include "moja/flint/example/agri/disturbanceeventmodule.h"

#include "moja/flint/example/agri/disturbanceevents.h"
#include "moja/flint/example/agri/fertevents.h"

#include <moja/flint/eventqueue.h>
#include <moja/flint/flintexceptions.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ivariable.h>
#include <moja/flint/itiming.h>

#include <moja/logging.h>
#include <moja/notificationcenter.h>
#include <moja/signals.h>

namespace moja {
namespace flint {
namespace example {
namespace agri {
void DisturbanceEventModule::configure(const DynamicObject& config) {}

void DisturbanceEventModule::subscribe(NotificationCenter& notificationCenter) {
   notificationCenter.subscribe(signals::TimingInit, &DisturbanceEventModule::onTimingInit, *this);
   notificationCenter.subscribe(signals::DisturbanceEvent, &DisturbanceEventModule::disturbanceEventHandler, *this); 
}

void DisturbanceEventModule::onTimingInit() {
   atmosphere_ = _landUnitData->getPool("atmosphere");
   soil_ = _landUnitData->getPool("soil");
   try{
      climate = _landUnitData->getVariable("climate")->value().convert<std::string>();
   }
   catch(const std::exception&){
      climate = "default";
   }
}

void DisturbanceEventModule::simulate(const NFertEvent& fert) {
   auto EF_1 = _landUnitData->getVariable("EF_1")->value().extract<DynamicObject>();
   double EF_1_value;
   if(climate == "default"){
      EF_1_value = EF_1["default"];
   }
   else if(climate == "dry"){
      EF_1_value = EF_1["dry"];
   }
   else if(climate == "wet" && fert.name == "Synthetic emission"){
      EF_1_value = EF_1["synth_wet"];
   }
   else{
      EF_1_value = EF_1["wet"];
   }
   MOJA_LOG_DEBUG << fert.name + " Event Occured";
   auto operation = _landUnitData->createStockOperation();
   operation->addTransfer(soil_, atmosphere_, (fert.quantity * EF_1_value) / fert.runtime);
   _landUnitData->submitOperation(operation);
}

void DisturbanceEventModule::disturbanceEventHandler(const flint::EventQueueItem* event) {
   const auto disturbance_event = std::static_pointer_cast<const DisturbanceEventBase>(event->_event);
   disturbance_event->simulate(*this);
}

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja