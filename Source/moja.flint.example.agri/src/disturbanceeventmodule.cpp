#include "moja/flint/example/agri/disturbanceeventmodule.h"

#include "moja/flint/example/agri/disturbanceevents.h"
#include "moja/flint/example/agri/fertevents.h"

#include <moja/flint/eventqueue.h>
#include <moja/flint/flintexceptions.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/itiming.h>
#include <moja/flint/ivariable.h>

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
   notificationCenter.subscribe(signals::PreTimingSequence, &DisturbanceEventModule::onPreTimingSequence, *this);
}

void DisturbanceEventModule::onPreTimingSequence() {
   std::vector<std::pair<DateTime, std::shared_ptr<EmissionEvent>>> temp;
   auto& eventQueue = std::static_pointer_cast<flint::EventQueue>(
       _landUnitData->getVariable("eventqueue")->value<std::shared_ptr<flint::IFlintData>>());
   auto nextEvent = eventQueue->begin();
   while (nextEvent != eventQueue->end()) {
      const auto& curEvent = (*(nextEvent));
      if (curEvent._event->type != "agri.NFertEvent") {
         nextEvent++;
         continue;
      } else {
         const auto disturbance_event = std::static_pointer_cast<const NFertEvent>(curEvent._event);
         auto evDate = curEvent._date;
         std::shared_ptr<EmissionEvent> ev;
         if (curEvent._event->name == "Synthetic fertilizer")
            ev = std::make_shared<EmissionEvent>(1, "Synthetic emission");
         else
            ev = std::make_shared<EmissionEvent>(2, "Organic emission");
         ev->quantity = disturbance_event->quantity;
         ev->runtime = disturbance_event->runtime;
         for (auto i = 1; i < disturbance_event->runtime; i++) {
            evDate.addDays(1);
            temp.push_back(std::make_pair(evDate, ev));
         }
         nextEvent++;
      }
   }
   for (int i = 0; i < temp.size(); i++) {
      eventQueue->emplace_back(temp[i].first, temp[i].second);
   }
   std::sort(eventQueue->begin(), eventQueue->end(),
             [](const flint::EventQueueItem& a, const flint::EventQueueItem& b) -> bool { return a._date < b._date; });
}

void DisturbanceEventModule::onTimingInit() {
   atmosphere_ = _landUnitData->getPool("atmosphere");
   soil_ = _landUnitData->getPool("soil");
   try {
      climate = _landUnitData->getVariable("climate")->value().convert<std::string>();
   } catch (const std::exception&) {
      climate = "default";
   }
}

void DisturbanceEventModule::simulate(const NFertEvent& fert) {
   auto EF_1 = _landUnitData->getVariable("EF_1")->value().extract<DynamicObject>();
   double EF_1_value;
   if (climate == "default") {
      EF_1_value = EF_1["default"];
   } else if (climate == "dry") {
      EF_1_value = EF_1["dry"];
   } else if (climate == "wet" && fert.name == "Synthetic fertilizer") {
      EF_1_value = EF_1["synth_wet"];
   } else {
      EF_1_value = EF_1["wet"];
   }
   MOJA_LOG_DEBUG << fert.name + " Event Occured";
   auto operation = _landUnitData->createStockOperation();
   operation->addTransfer(soil_, atmosphere_, (fert.quantity * EF_1_value) / fert.runtime);
   _landUnitData->submitOperation(operation);
}

void DisturbanceEventModule::simulate(const EmissionEvent& fert) {
   auto EF_1 = _landUnitData->getVariable("EF_1")->value().extract<DynamicObject>();
   double EF_1_value;
   if (climate == "default") {
      EF_1_value = EF_1["default"];
   } else if (climate == "dry") {
      EF_1_value = EF_1["dry"];
   } else if (climate == "wet" && fert.name == "Synthetic emission") {
      EF_1_value = EF_1["synth_wet"];
   } else {
      EF_1_value = EF_1["wet"];
   }
   auto operation = _landUnitData->createStockOperation();
   operation->addTransfer(soil_, atmosphere_, (fert.quantity * EF_1_value) / fert.runtime);
   _landUnitData->submitOperation(operation);
}

void DisturbanceEventModule::simulate(const HarvestEvent& harvest) {
   MOJA_LOG_DEBUG << "Harvest Event Occured";
   auto EF_1 = _landUnitData->getVariable("EF_1")->value().extract<DynamicObject>();
   double EF_1_value;
   if (climate == "default") {
      EF_1_value = EF_1["default"];
   } else if (climate == "dry") {
      EF_1_value = EF_1["dry"];
   } else {
      EF_1_value = EF_1["wet"];
   }
   const auto FCR_table = _landUnitData->getVariable("FCR_table")->value().extract<const std::vector<DynamicObject>>();
   const auto Cf_table = _landUnitData->getVariable("Cf_table")->value().extract<const std::vector<DynamicObject>>();
   int temp = 0;
   for (int i = 0; i < FCR_table.size(); i++) {
      if (FCR_table[i]["Crops"].convert<std::string>() == harvest.name) {
         temp = i;
         break;
      }
   }
   std::string crop_type = FCR_table[temp]["Crops"].convert<std::string>();
   double DRY = FCR_table[temp]["DRY"].convert<double>();
   double R_AG = FCR_table[temp]["R_AG"].convert<double>();
   double R_S = FCR_table[temp]["R_S"].convert<double>();
   double N_AG = FCR_table[temp]["N_AG"].convert<double>();
   double N_BG = FCR_table[temp]["N_BG"].convert<double>();
   temp = 4;
   if (crop_type == "Winter Wheat" || crop_type == "Spring Wheat") {
      crop_type = "Wheat";
   }
   for (int i = 0; i < Cf_table.size(); i++) {
      if (Cf_table[i]["Crops"].convert<std::string>() == crop_type) {
         temp = i;
         break;
      }
   }
   double cf = Cf_table[temp]["Cf"].convert<double>();
   double area = 1;
   double dry_crop_yield = harvest.yield_fresh * DRY;
   double above_ground_dry_residue = dry_crop_yield * R_AG;
   double below_ground_residue = (dry_crop_yield + above_ground_dry_residue) * R_S * area * harvest.frac_renew;
   double F_CR = harvest.above_ground_residue * N_AG * (1.0 - harvest.frac_renew - (harvest.frac_burnt * cf)) +
                 below_ground_residue * N_BG;
   auto operation = _landUnitData->createStockOperation();
   operation->addTransfer(soil_, atmosphere_, F_CR * EF_1_value);
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