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
   if (eventTypeStr == "agri.PRPEvent") return std::make_shared<PRPEvent>(id, name);
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
   yield_fresh = config["yield_fresh"];
   frac_renew = config["frac_renew"];
   frac_remove = config["frac_remove"];
   frac_burnt = config["frac_burnt"];
   above_ground_residue = config["above_ground_residue"];
}

DynamicObject HarvestEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   object["yield_fresh"] = yield_fresh;
   object["frac_renew"] = frac_renew;
   object["frac_remove"] = frac_remove;
   object["frac_burnt"] = frac_burnt;
   object["above_ground_residue"] = above_ground_residue;
   return object;
}

void HarvestEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }


void PRPEvent::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                 datarepository::DataRepository& dataRepository) {
   DisturbanceEventBase::configure(config, landUnitController, dataRepository);
   no_livestock = config["no_livestock"];
   animal_type = config["animal_type"].extract<const std::string>();
   productivity_class = config["productivity_class"].extract<const std::string>();
   use = config["use"].extract<const std::string>();
}

DynamicObject PRPEvent::exportObject() const {
   auto object = DisturbanceEventBase::exportObject();
   object["no_livestock"] = no_livestock;
   object["animal_type"] = animal_type;
   object["productivity_class"] = productivity_class;
   object["use"] = use;
   return object;
}

void PRPEvent::simulate(DisturbanceEventHandler& event_handler) const { event_handler.simulate(*this); }
}  // namespace chapman_richards
}  // namespace modules
}  // namespace moja
}