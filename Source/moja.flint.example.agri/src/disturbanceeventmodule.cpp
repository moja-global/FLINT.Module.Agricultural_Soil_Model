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
   notificationCenter.subscribe(signals::TimingStep, &DisturbanceEventModule::onTimingStep, *this);
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
   debris_ = _landUnitData->getPool("debris");

   auto climate_ = _landUnitData->getVariable("ipcc_climate_zone");

   try {
      auto zones_ = _landUnitData->getVariable("zones");
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
   } catch (const std::exception& e) {
   }

   std::string climateZone = _landUnitData->getVariable("ipcc_climate_zone")->value().convert<std::string>();
   DynamicObject table;
   try {
      table = _landUnitData->getVariable("Wet_Dry_Climate")->value().extract<DynamicObject>();
   } catch (const std::exception& e) {
      std::string str = "Climate Zone: " + climateZone + " is not an IPCC Climate Zone";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }
   climate = table["Wet/Dry"] ? "wet" : "dry";
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
   if (planted == false) {
      std::string str = "Harvest Event cannot occur before plantation of crop";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }
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

   const auto cropType = _landUnitData->getVariable("crop_type");

   DynamicObject FCR_table;

   try {
      FCR_table = _landUnitData->getVariable("FCR_table")->value().extract<const DynamicObject>();
   } catch (const std::exception& e) {
      std::string str = "Crop Type: " + harvest.name + " not present in FLINTagri.db Cf_table";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }

   std::string crop_type = FCR_table["Crops"].convert<std::string>();
   double DRY = FCR_table["DRY"].convert<double>();
   double R_AG = FCR_table["R_AG"].convert<double>();
   double R_S = FCR_table["R_S"].convert<double>();
   double N_AG = FCR_table["N_AG"].convert<double>();
   double N_BG = FCR_table["N_BG"].convert<double>();
   double cf = FCR_table["Cf"].convert<double>();
   double area = 1;
   double dry_crop_yield = yield * DRY;
   double above_ground_dry_residue = dry_crop_yield * R_AG;
   double below_ground_residue = (dry_crop_yield + above_ground_dry_residue) * R_S * area * harvest.frac_renew;
   double F_CR = above_ground_dry_residue * N_AG * (1.0 - harvest.frac_remove - (harvest.frac_burnt * cf)) +
                 below_ground_residue * N_BG;
   auto operation = _landUnitData->createStockOperation();
   operation->addTransfer(soil_, debris_, F_CR * EF_1_value);
   _landUnitData->submitOperation(operation);

   harvested = true;
   cropType->set_value("default");
   yield = 0;
   planted = false;
}

void DisturbanceEventModule::simulate(const ManureManagementEvent& manure) {
   MOJA_LOG_DEBUG << "ManureManagementEvent occured";
   auto EF_3 = _landUnitData->getVariable("EF_3")->value().extract<DynamicObject>();
   double EF_3_value;

   auto AWMS_manure = _landUnitData->getVariable("AWMS_manure_management")->value().extract<DynamicObject>();
   auto temp = AWMS_manure.members();
   std::vector<std::string> systems(temp.begin(), temp.end());

   int size = manure.animal_type.size();

   if (size != manure.no_livestock.size() || size != manure.productivity_class.size() || size != manure.use.size()) {
      std::string str = "Size of the variable arrays is not same";
      BOOST_THROW_EXCEPTION(flint::IncompleteConfigurationException()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }

   for (auto i = 0; i < size; i++) {
      auto animal_type = manure.animal_type[i].extract<std::string>();
      int no_livestock = manure.no_livestock[i];
      auto productivity_class = manure.productivity_class[i].extract<std::string>();
      auto use = manure.use[i].extract<std::string>();

      if (animal_type == "Dairy cattle" || animal_type == "Other cattle") {
         if (climate == "default") {
            EF_3_value = EF_3["cattle_default"];
         } else if (climate == "dry") {
            EF_3_value = EF_3["cattle_dry"];
         } else {
            EF_3_value = EF_3["cattle_wet"];
         }
      } else {
         EF_3_value = EF_3["other"];
      }
      const auto region = _landUnitData->getVariable("region")->value().convert<std::string>();
      std::string region_1, region_2;

      if (region == "Latin America" || region == "India" || region == "Middle East" || region == "Africa" ||
          region == "Asia") {
         if (productivity_class == "High")
            region_1 = region + " High";
         else if (productivity_class == "Low")
            region_1 = region + " Low";
         else
            region_1 = region + " Mean";
         region_2 = region;
      }

      const auto animalType = _landUnitData->getVariable("animal_type");
      animalType->set_value(animal_type);
      DynamicObject Animal_weights;

      try {
         Animal_weights = _landUnitData->getVariable("Animal_weights")->value().extract<const DynamicObject>();
      } catch (const std::exception& e) {
         std::string str = "Animal type: " + animal_type + " not present in FLINTagri.db Animal_weights";
         BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                               << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                               << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
      }

      DynamicObject ex_rate;

      try {
         ex_rate = _landUnitData->getVariable("N_Excretion_rate")->value().extract<const DynamicObject>();
      } catch (const std::exception& e) {
         std::string str = "Animal type: " + animal_type + " not present in FLINTagri.db N_excretion_rate";
         BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                               << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                               << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
      }

      double N_rate = ex_rate[region_1];
      double weight = Animal_weights[region_1];
      double N_ex = N_rate * weight / 1000;

      std::string animal = animal_type;

      if (animal_type == "Horses" || animal_type == "Mules" || animal_type == "Camels" || animal_type == "Asses") {
         animal = "Goat";
      } else if (animal_type == "Swine Finishing" || animal_type == "Swine Breeding") {
         animal = animal_type + " " + productivity_class;
      } else if (animal_type == "Buffalo") {
         animal = animal_type + " Other";
         if (use == "Dairy") animal = animal_type + " Dairy";
      } else if (animal_type == "Sheep") {
         if (use == "Dairy")
            animal = animal_type + " Dairy";
         else if (use == "Meat")
            animal = animal_type + " Meat";
      }

      animalType->set_value(animal);
      DynamicObject AWMS;

      try {
         AWMS = _landUnitData->getVariable("AWMS")->value().extract<const DynamicObject>();
      } catch (const std::exception& e) {
         std::string str = "Animal type: " + animal + " not present in FLINTagri.db AWMS";
         BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                               << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                               << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
      }

      double MS = AWMS[region_2].convert<double>();
      double Fprp = no_livestock * N_ex * MS;

      auto operation = _landUnitData->createStockOperation();
      operation->addTransfer(soil_, atmosphere_, Fprp * EF_3_value);
      _landUnitData->submitOperation(operation);

      

      auto EF_1 = _landUnitData->getVariable("EF_1")->value().extract<DynamicObject>();
      double EF_1_value;
      if (climate == "default") {
         EF_1_value = EF_1["default"];
      } else if (climate == "dry") {
         EF_1_value = EF_1["dry"];
      } else {
         EF_1_value = EF_1["wet"];
      }

      double NMMS = 0;

      for (auto i = 0; i < systems.size(); i++) {
         auto temp = AWMS_manure[systems[i]].extract<DynamicObject>();
         double AWMS_system;
         if (temp.find(animal_type) != temp.end()) {
            AWMS_system = temp[animal_type];
         } else {
            std::string str = animal_type + " not present in AWMS_manure_management under system " + systems[i];
            BOOST_THROW_EXCEPTION(flint::IncompleteConfigurationException()
                                  << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                                  << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
         }

         double EF_3_system, frac_gas, frac_leach;

         std::string temp_animal = animal_type;

         if (animal_type != "Swine" && animal_type != "Dairy cattle" && animal_type != "Other cattle" &&
             animal_type != "Poultry")
            temp_animal = "Other";

         auto random = _landUnitData->getVariable("systems");
         random->set_value(systems[i]);
         auto nitrogenLoss =
             _landUnitData->getVariable("Nitrogen_Loss")->value().extract<const DynamicObject>();
         EF_3_system = nitrogenLoss["EF_3"];
         frac_gas = nitrogenLoss[temp_animal + " Gas"];
         frac_leach = nitrogenLoss[temp_animal + " Leach"];

         double R_N2 = 3;
         double frac_N2MS = R_N2 * EF_3_system;

         double frac_loss = frac_gas + frac_leach + frac_N2MS + EF_3_system;

         auto N_cdg = manure.N_cdg;
         auto N_bedding = manure.N_bedding;

         NMMS += (no_livestock * N_ex * AWMS_system + N_cdg) * (1 - frac_loss) + no_livestock * AWMS_system * N_bedding;
      }

      auto frac_feed = manure.frac_feed;
      auto frac_cnst = manure.frac_cnst;
      auto frac_fuel = manure.frac_fuel;

      operation = _landUnitData->createStockOperation();
      operation->addTransfer(soil_, atmosphere_, NMMS * (1 - (frac_feed + frac_cnst + frac_fuel)) * EF_1_value);
      _landUnitData->submitOperation(operation);
   }
}

void DisturbanceEventModule::simulate(const PlantEvent& plant) {
   auto crop_type = _landUnitData->getVariable("crop_type");
   if (planted) {
      std::string str = "Plant event has occured when the previous yield has not been harvested";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }
   crop_type->set_value(plant.name);
   planted = true;
}

void DisturbanceEventModule::disturbanceEventHandler(const flint::EventQueueItem* event) {
   const auto disturbance_event = std::static_pointer_cast<const DisturbanceEventBase>(event->_event);
   disturbance_event->simulate(*this);
}

void DisturbanceEventModule::onTimingStep() {
   auto crop_type = _landUnitData->getVariable("crop_type")->value().extract<std::string>();
   double decayRate;

   if (planted) {
      DynamicObject FCR_table;

      try {
         FCR_table = _landUnitData->getVariable("FCR_table")->value().extract<const DynamicObject>();
      } catch (const std::exception& e) {
         std::string str = "Crop Type: " + crop_type + " not present in FLINTagri.db Cf_table";
         BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                               << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                               << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
      }

      yield += FCR_table["Growth_Rate"]; 
      decayRate = FCR_table["Decay_Rate"];
   }

   if (harvested) {
      auto operation = _landUnitData->createProportionalOperation();
      operation->addTransfer(debris_, atmosphere_, 0.5);
      _landUnitData->submitOperation(operation);
   }
}

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja