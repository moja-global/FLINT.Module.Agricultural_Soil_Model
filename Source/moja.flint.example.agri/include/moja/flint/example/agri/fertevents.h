#pragma once
#include "moja/flint/example/agri/disturbanceevents.h"

#include <moja/flint/iflintdata.h>

#include <moja/dynamic.h>

namespace moja {
namespace flint {
namespace example {
namespace agri {

std::shared_ptr<flint::IFlintData> createFertEventsFactory(const std::string& eventTypeStr, int id,
                                                             const std::string& name, const DynamicObject& other);

class NFertEvent final : public DisturbanceEventBase {
  public:
   NFertEvent(int id, const std::string& name) : DisturbanceEventBase(id, "agri.NFertEvent", name) {}
   virtual ~NFertEvent() = default;

   void configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                  datarepository::DataRepository& dataRepository) override;
   DynamicObject exportObject() const override;
   void simulate(DisturbanceEventHandler& event_handler) const override;
   double quantity;
   int runtime;
};

class EmissionEvent final : public DisturbanceEventBase {
  public:
   EmissionEvent(int id, const std::string& name) : DisturbanceEventBase(id, "agri.EmissionEvent", name) {}
   virtual ~EmissionEvent() = default;

   void configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                  datarepository::DataRepository& dataRepository) override;
   DynamicObject exportObject() const override;
   void simulate(DisturbanceEventHandler& event_handler) const override;
   double quantity;
   int runtime;
};

}  // namespace chapman_richards
}  // namespace modules
}  // namespace moja
}