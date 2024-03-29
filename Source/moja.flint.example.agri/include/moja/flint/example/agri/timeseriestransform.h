#ifndef MOJA_FLINT_EXAMPLE_AGRI_COMPOSITETRANSFORM_H_
#define MOJA_FLINT_EXAMPLE_AGRI_COMPOSITETRANSFORM_H_

#include "moja/flint/example/agri/_modules.agri_exports.h"

#include <moja/flint/itransform.h>
#include <moja/dynamic.h>

namespace moja {
    namespace flint {
        class IVariable;
    }
}

namespace moja {
namespace flint {
namespace example {
namespace agri {

class CompositeTimeSeriesTransform : public flint::ITransform {
  public:
   enum Format { Wide, Long };

   void configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                  moja::datarepository::DataRepository& dataRepository) override;

   void controllerChanged(const flint::ILandUnitController& controller) override;
   const DynamicVar& value() const override;

  private:
   const flint::ILandUnitController* _landUnitController;
   datarepository::DataRepository* _dataRepository;

   mutable std::vector<const IVariable*> _variables;
   std::vector<std::string> _varNames;
   mutable DynamicVar _currentValue;
   mutable std::string _dataPropertyName;
};

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja

#endif // MOJA_FLINT_EXAMPLE_AGRI_COMPOSITETRANSFORM_H_
