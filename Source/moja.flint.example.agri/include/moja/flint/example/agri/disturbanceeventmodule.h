#pragma once

#include "moja/flint/example/agri/_modules.agri_exports.h"
#include "moja/flint/example/agri/disturbanceevents.h"

#include <moja/flint/modulebase.h>

namespace moja {
namespace flint {
namespace example {
namespace agri {
class NFertEvent;
class EmissionEvent;
class HarvestEvent;
class PRPEvent;
class PlantEvent;

class AGRI_API DisturbanceEventModule : public flint::ModuleBase, DisturbanceEventHandler {
  public:
   DisturbanceEventModule() = default;
   virtual ~DisturbanceEventModule() = default;

   void configure(const DynamicObject& config) override;
   void subscribe(NotificationCenter& notificationCenter) override;

   void onPreTimingSequence() override; 
   void onTimingInit() override;
   void onTimingStep() override;

   void disturbanceEventHandler(const flint::EventQueueItem* event);  // special handler for events

  private:
   void simulate(const NFertEvent& fert) override;
   void simulate(const EmissionEvent& fert) override;
   void simulate(const HarvestEvent& harvest) override;
   void simulate(const PRPEvent& prp) override;
   void simulate(const PlantEvent &plant) override;

   const flint::IPool* atmosphere_;
   const flint::IPool* soil_;
   const flint::IPool* initial_values;
   const flint::IPool* debris_;
   std::string climate;
};

}  // namespace chapman_richards
}  // namespace modules
}  // namespace agri
}