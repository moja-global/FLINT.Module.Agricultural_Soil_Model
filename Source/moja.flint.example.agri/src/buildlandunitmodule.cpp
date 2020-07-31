#include "moja/flint/example/agri/buildlandunitmodule.h"

#include "moja/flint/example/agri/fertevents.h"

#include <moja/flint/eventqueue.h>
#include <moja/flint/flintexceptions.h>
#include <moja/flint/itiming.h>
#include <moja/flint/ivariable.h>
#include <moja/flint/spatiallocationinfo.h>

#include <moja/notificationcenter.h>
#include <moja/signals.h>
#include <moja/logging.h>

#include <boost/format.hpp>

namespace moja {
namespace flint {
namespace example {
namespace agri {

void BuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
   notificationCenter.subscribe(signals::LocalDomainInit, &BuildLandUnitModule::onLocalDomainInit, *this);
   notificationCenter.subscribe(signals::PreTimingSequence, &BuildLandUnitModule::onPreTimingSequence, *this);
}

void BuildLandUnitModule::configure(const DynamicObject& config) {}

void BuildLandUnitModule::onLocalDomainInit() {
   landuse_ = _landUnitData->getVariable("landuse");
   climate_ = _landUnitData->getVariable("ipcc_climate_zone");
}

void BuildLandUnitModule::onPreTimingSequence() {
   zones_ = _landUnitData->getVariable("zones");
   DynamicObject table;
   try {
      table = _landUnitData->getVariable("Wet_Dry_Zone")->value().extract<DynamicObject>();
   } catch (const std::exception& e) {
      std::string str = "Zone Id: " + zones_->value() + " is not an IPCC Climate Zone";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }
   climate_->set_value(table["Climate_Zone"]);
  
   const auto timing = _landUnitData->timing();
   const auto startDate = timing->startDate();
   const auto endDate = timing->endDate();

   auto temp = startDate;
   auto event_queue = std::static_pointer_cast<flint::EventQueue>(
         _landUnitData->getVariable("eventqueue")->value<std::shared_ptr<flint::IFlintData>>());
   event_queue->clear();
   int eventId = 0;

   if (landuse_->value().convert<int>() == 15) {
		std::shared_ptr<NFertEvent> synth_ev, org_ev;
      while(temp < endDate) {
         auto date = temp;
		   synth_ev = std::make_shared<NFertEvent>(eventId++, "Synthetic emission");
			synth_ev->quantity = 100;
		   synth_ev->runtime = 5;
         org_ev = std::make_shared<NFertEvent>(eventId++, "Organic emission");
         org_ev->quantity = 100;
		   org_ev->runtime = 5;
         event_queue->emplace_back(date, synth_ev);
         date.addHours(6);
         event_queue->emplace_back(date, org_ev);
         temp.addMonths(3);
      }
   }
}

}  // namespace chapman_richards
}  // namespace modules
}  // namespace moja
}