#include "moja/flint/example/agri/buildlandunitmodule.h"

#include "moja/flint/example/agri/fertevents.h"

#include <moja/flint/eventqueue.h>
#include <moja/flint/flintexceptions.h>
#include <moja/flint/itiming.h>
#include <moja/flint/ivariable.h>
#include <moja/flint/spatiallocationinfo.h>

#include <moja/logging.h>
#include <moja/notificationcenter.h>
#include <moja/signals.h>

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
   
}

void BuildLandUnitModule::onPreTimingSequence() {
   /*zones_ = _landUnitData->getVariable("zones");
   DynamicObject table;
   try {
      table = _landUnitData->getVariable("Wet_Dry_Zone")->value().extract<DynamicObject>();
   } catch (const std::exception& e) {
      if (zones_->value() == 0) {
         climate_->set_value("default");
      } else {
         std::string str = "Zone Id: " + zones_->value() + " is not an IPCC Climate Zone";
         BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                               << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                               << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   
      }
   }
   climate_->set_value(table["Climate_Zone"]);
   
   const auto timing = _landUnitData->timing();
   const auto startDate = timing->startDate();
   const auto endDate = timing->endDate();

   auto temp = startDate;
   */
   auto event_queue = std::static_pointer_cast<flint::EventQueue>(
       _landUnitData->getVariable("eventqueue")->value<std::shared_ptr<flint::IFlintData>>());
   event_queue->clear();
   int eventId = 0;

   const auto timing = _landUnitData->timing();
   const auto startDate = timing->startDate();
   const auto endDate = timing->endDate();

   auto prev_date = startDate; 
  
   auto events = _landUnitData->getVariable("spatialevents")->value().extract<const DynamicVector>();

   for (auto i = 0; i < events.size(); i++) {
      auto temp = events[i].extract<DynamicObject>();


      if (temp["type"] == "agri.PlantEvent") {
         auto plant = std::make_shared<PlantEvent>(eventId++, temp["name"].extract<std::string>());
         auto date = temp["date"].extract<const DateTime>();
         event_queue->emplace_back(date, plant);

      } else if (temp["type"] == "agri.HarvestEvent") {
         auto harvest = std::make_shared<HarvestEvent>(eventId++, "Harvest Event");
         harvest->frac_burnt = temp["frac_burnt"];
         harvest->frac_remove = temp["frac_remove"];
         harvest->frac_renew = temp["frac_renew"];
         auto date = temp["date"].extract<const DateTime>();
         event_queue->emplace_back(date, harvest);

      } else if (temp["type"] == "agri.NFertEvent" && temp["name"] == "Synthetic fertilizer") {
         auto synth_ev = std::make_shared<NFertEvent>(eventId++, temp["name"]);
         auto date = temp["date"].extract<const DateTime>();
         synth_ev->quantity = temp["quantity"];
         synth_ev->runtime = temp["runtime"];
         event_queue->emplace_back(date, synth_ev);

      } else if (temp["type"] == "agri.NFertEvent" && temp["name"] == "Organic fertilizer") {
         auto org_ev = std::make_shared<NFertEvent>(eventId++, temp["name"]);
         auto date = temp["date"].extract<const DateTime>();
         org_ev->quantity = temp["quantity"];
         org_ev->runtime = temp["runtime"];
         event_queue->emplace_back(date, org_ev);

      }
   }

   std::sort(event_queue->begin(), event_queue->end(),
             [](const flint::EventQueueItem& a, const flint::EventQueueItem& b) -> bool { return a._date < b._date; });
}

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja