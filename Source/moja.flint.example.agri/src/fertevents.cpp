#include "moja/flint/example/agri/fertevents.h"

#include "moja/flint/example/agri/disturbanceevents.h"

#include <moja/flint/ilandunitcontroller.h>
#include <moja/flint/ivariable.h>

namespace moja {
namespace flint {
namespace example {
namespace agri {

// Flint Data Factory
std::shared_ptr<flint::IFlintData> createFertEventsFactory(const std::string& eventTypeStr, int id,
                                                             const std::string& name, const DynamicObject& other) {
   if (eventTypeStr == "agri.NFertEvent") return std::make_shared<NFertEvent>(id, name);
   if (eventTypeStr == "agri.EmissionEvent") return std::make_shared<EmissionEvent>(id, name);
   if (eventTypeStr == "agri.HarvestEvent") return std::make_shared<HarvestEvent>(id, name);
   if (eventTypeStr == "agri.ManureManagementEvent") return std::make_shared<ManureManagementEvent>(id, name);
   if (eventTypeStr == "agri.PlantEvent") return std::make_shared<PlantEvent>(id, name);
   return nullptr;
}

void NFertEvent::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   DisturbanceEventBase::configure(config, landUnitController, dataRepository);
   quantity = config["quantity"];
   runtime = config["runtime"];
}

DynamicObject NFertEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   object["quantity"] = quantity;
   object["runtime"] = runtime;
   return object;
}

void NFertEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }

void EmissionEvent::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   DisturbanceEventBase::configure(config, landUnitController, dataRepository);
   quantity = config["quantity"];
   runtime = config["runtime"];
}

DynamicObject EmissionEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   object["quantity"] = quantity;
   object["runtime"] = runtime;
   return object;
}

void EmissionEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }


void HarvestEvent::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   DisturbanceEventBase::configure(config, landUnitController, dataRepository);
   frac_renew = config["frac_renew"];
   frac_remove = config["frac_remove"];
   frac_burnt = config["frac_burnt"];
}

DynamicObject HarvestEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   object["frac_renew"] = frac_renew;
   object["frac_remove"] = frac_remove;
   object["frac_burnt"] = frac_burnt;
   return object;
}

void HarvestEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }


void ManureManagementEvent::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   DisturbanceEventBase::configure(config, landUnitController, dataRepository);
   no_livestock = config["no_livestock"].extract<const std::vector<DynamicVar>>();
   animal_type = config["animal_type"].extract<const std::vector<DynamicVar>>();
   productivity_class = config["productivity_class"].extract<const std::vector<DynamicVar>>();
   use = config["use"].extract<const std::vector<DynamicVar>>();
   N_cdg = config["N_cdg"];
   N_bedding = config["N_bedding"];
   frac_feed = config["frac_feed"];
   frac_fuel = config["frac_fuel"];
   frac_cnst = config["frac_cnst"];
}

DynamicObject ManureManagementEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   object["no_livestock"] = no_livestock;
   object["animal_type"] = animal_type;
   object["productivity_class"] = productivity_class;
   object["use"] = use;
   object["N_cdg"] = N_cdg;
   object["N_bedding"] = N_bedding;
   object["frac_feed"] = frac_feed;
   object["frac_fuel"] = frac_fuel;
   object["frac_cnst"] = frac_cnst;
   return object;
}

void ManureManagementEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }

void PlantEvent::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   DisturbanceEventBase::configure(config, landUnitController, dataRepository);
}

DynamicObject PlantEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   return object;
}

void PlantEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }
}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja