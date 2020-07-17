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


void DisturbanceEventModule::simulate(const PRPEvent& prp) {
   MOJA_LOG_DEBUG << "PRPEvent occured";
   auto EF_3 = _landUnitData->getVariable("EF_3")->value().extract<DynamicObject>();
   double EF_3_value;
   if (prp.animal_type == "Dairy cattle" || prp.animal_type == "Other cattle") {
      if (climate == "default") {
         EF_3_value = EF_3["cattle_default"];
      } else if (climate == "dry") {
         EF_3_value = EF_3["cattle_dry"];
      } else {
         EF_3_value = EF_3["cattle_wet"];
      }
   }
   else {
      EF_3_value = EF_3["other"];
   }
   const auto AWMS = _landUnitData->getVariable("AWMS")->value().extract<const std::vector<DynamicObject>>();
   const auto Animal_weights = _landUnitData->getVariable("Animal_weights")->value().extract<const std::vector<DynamicObject>>();
   const auto ex_rate = _landUnitData->getVariable("N_Excretion_rate")->value().extract<const std::vector<DynamicObject>>();

   const auto region = _landUnitData->getVariable("region")->value().convert<std::string>();
   std::string region_1, region_2;

   if (region == "North America") {
      region_1 = "North_America";
      region_2 = region_1;
   }
   else if (region == "Western_Europe") {
      region_1 = "Western_Europe";
      region_2 = region_1;   
   }
   else if (region == "Eastern Europe") {
      region_1 = "Eastern_Europe";
      region_2 = region_1;
   }
   else if (region == "Russia") {
      region_1 = "Eastern Europe";
      region_2 = "Russia";     
   }
   else if (region == "Oceania") {
      region_1 = "Oceania";
      region_2 = region_1;
   }
   else if (region == "Latin America") {
      if (prp.productivity_class == "high")
         region_1 = "Latin_America_High";
      else if(prp.productivity_class == "low")
         region_1 = "Latin_America_Low";
      else region_1 = "Latin_America_Mean";
      region_2 = "Latin_America";
   }
   else if (region == "Africa") {
      if (prp.productivity_class == "high")
         region_1 = "Africa_High";
      else if(prp.productivity_class == "low")
         region_1 = "Africa_Low";
      else region_1 = "Africa_Mean";
      region_2 = "Africa";
   }
   else if (region == "Middle East") {
      if (prp.productivity_class == "high")
         region_1 = "Middle_East_High";
      else if(prp.productivity_class == "low")
         region_1 = "Middle_East_Low";
      else region_1 = "Middle_East_Mean";
      region_2 = "Middle_East";
   }
   else if (region == "Asia") {
      if (prp.productivity_class == "high")
         region_1 = "Asia_High";
      else if(prp.productivity_class == "low")
         region_1 = "Asia_Low";
      else region_1 = "Asia_Mean";
      region_2 = "Asia";
   }
   else if (region == "India") {
      if (prp.productivity_class == "high")
         region_1 = "India_High";
      else if(prp.productivity_class == "low")
         region_1 = "India_Low";
      else region_1 = "India_Mean";
      region_2 = "India";
   }

   int temp = -1;
   for (auto i = 0; i < ex_rate.size(); i++){
      if (ex_rate[i]["Animal"].convert<std::string>() == prp.animal_type) {
         temp = i;
         break;
      }
   }
   if (temp == -1) {
      std::string str = "Animal type: " + prp.animal_type + " not present in FLINTagri.db";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }

   double N_rate = ex_rate[temp][region_1].convert<double>();

   temp = -1;
   for (auto i = 0; i < ex_rate.size(); i++){
      if (Animal_weights[i]["Animal"].convert<std::string>() == prp.animal_type) {
         temp = i;
         break;
      }
   }
   if (temp == -1) {
      std::string str = "Animal type: " + prp.animal_type + " not present in FLINTagri.db";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }

   double weight = Animal_weights[temp][region_1];
   double N_ex = N_rate * weight / 1000;

   temp = -1;

   std::string animal = prp.animal_type;

   if (prp.animal_type == "Horses" || prp.animal_type == "Mules" || prp.animal_type == "Camels" || prp.animal_type == "Asses") {
      animal = "Goat";
   }
   else if (prp.animal_type == "Swine Finishing" || prp.animal_type == "Swine Breeding") {
      if (prp.productivity_class == "high") 
         animal = prp.animal_type + " High";
      else if(prp.productivity_class == "low")
         animal = prp.animal_type + " Low";
   }
   else if (prp.animal_type == "Buffalo") {
      animal = prp.animal_type + " Other";
      if (prp.use == "Dairy")
         animal = prp.animal_type + " Dairy"; 
   }
   else if (prp.animal_type == "Sheep") {
      if (prp.use == "Dairy")
         animal = prp.animal_type + " Dairy";
      else if (prp.use == "Meat")
         animal = prp.animal_type + " Meat";
   }
   for (auto i = 0; i < ex_rate.size(); i++){
      if (AWMS[i]["Animal"].convert<std::string>() == animal) {
         temp = i;
         break;
      }
   }

   if (temp == -1) {
      std::string str = "Animal type: " + animal + " not present in FLINTagri.db";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }

   double MS = AWMS[temp][region_2].convert<double>();
   double Fprp = prp.no_livestock * N_ex * MS;

   auto operation = _landUnitData->createStockOperation();
   operation->addTransfer(soil_, atmosphere_, Fprp * EF_3_value);
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