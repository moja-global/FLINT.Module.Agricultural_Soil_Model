#ifndef MOJA_FLINT_LOCATIONIDXFROMFLINTDATATRANSFORM_H_
#define MOJA_FLINT_LOCATIONIDXFROMFLINTDATATRANSFORM_H_

#include "moja/flint/itransform.h"
#include "moja/flint/example/agri/_modules.agri_exports.h"
#include <moja/flint/itiming.h>
#include <moja/dynamic.h>

namespace moja {
namespace flint {
class SpatialLocationInfo;
namespace example {
namespace agri {

class AGRI_API SpatialTransform : public ITransform {
  public:
   SpatialTransform() : _spatialLocationInfo(nullptr) {}

   void configure(DynamicObject config, const ILandUnitController& landUnitController,
                  datarepository::DataRepository& dataRepository) override;

   void controllerChanged(const ILandUnitController& controller) override;
   const DynamicVar& value() const override;

  private:
   const ILandUnitController* _landUnitController;

   mutable std::shared_ptr<const SpatialLocationInfo> _spatialLocationInfo;

   mutable DynamicVar _cachedValue;
   mutable std::string _dataPropertyName;
};

}  // namespace flint
}  // namespace moja
}
}
#endif  // MOJA_FLINT_LOCATIONIDXFROMFLINTDATATRANSFORM_H_