#pragma once

#include <moja/flint/modulebase.h>

#include <boost/optional/optional_fwd.hpp>

namespace moja {
namespace flint {
class EventQueue;
}
}  // namespace moja

namespace moja {
namespace flint {
namespace example {
namespace agri {

class BuildLandUnitModule : public flint::ModuleBase {
   public:
      BuildLandUnitModule() = default;
      virtual ~BuildLandUnitModule() = default;

      void configure(const DynamicObject&) override;
      void subscribe(NotificationCenter& notificationCenter) override;

      // Notification handlers

      void onLocalDomainInit() override;
      void onPreTimingSequence() override;

   private:
      flint::IVariable* landuse_;
      flint::IVariable* climate_;
      const flint::IVariable* zones_;
};

}  // namespace chapman_richards
}  // namespace modules
}  // namespace moja
}