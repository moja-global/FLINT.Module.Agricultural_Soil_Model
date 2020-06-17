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

}  // namespace chapman_richards
}  // namespace modules
}  // namespace moja
}