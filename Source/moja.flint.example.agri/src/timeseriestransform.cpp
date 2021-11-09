#include "moja/flint/example/agri/timeseriestransform.h"

#include "moja/flint/flintexceptions.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/ivariable.h"

#include <moja/flint/itiming.h>

#include <moja/logging.h>

#include <boost/algorithm/string.hpp>

namespace moja {
namespace flint {
namespace example {
namespace agri {

void CompositeTimeSeriesTransform::configure(DynamicObject config, const flint::ILandUnitController& landUnitController,
                                             moja::datarepository::DataRepository& dataRepository) {
   _landUnitController = &landUnitController;
   _dataRepository = &dataRepository;

   _dataPropertyName = "data";
   if (config.contains("data_property")) {
      _dataPropertyName = config["data_property"].extract<std::string>();
   }

   if (!config.contains("vars")) {
      throw std::runtime_error("vars");
   }

   auto varNames = config["vars"];
   if (varNames.size() < 1) {
      throw std::runtime_error("CompositeTimeSeriesTransform:: At least one variable name required for varName");
   }

   for (std::string varName : varNames) {
      if (std::find(_varNames.begin(), _varNames.end(), varName) != _varNames.end()) {
         throw std::runtime_error("CompositeTimeSeriesTransform:: Duplicate variable reference " + varName);
      }

      _varNames.push_back(varName);
   }

   if (_variables.empty()) {
      for (auto varName : _varNames) {
         auto var = _landUnitController->getVariable(varName);
         if (var == nullptr) {
            throw std::runtime_error("CompositeTimeSeriesTransform:: Variable not found" + varName);
         }

         _variables.push_back(var);
      }
   }
}

void CompositeTimeSeriesTransform::controllerChanged(const flint::ILandUnitController& controller) {
   // opportunity to change cache if there is any kept
   //_cachedValue = nullptr;
   _landUnitController = &controller;
};

// Step 0 will be the Init step, so I've added a blank value to the start of each dataset
const DynamicVar& CompositeTimeSeriesTransform::value() const {
   // simply look into the data array and get the value out for the current step.

   const auto timing = &_landUnitController->timing();
   DynamicObject result;
   
   for (auto var : _variables) {
      int curStep = timing->step();
      std::vector<DynamicVar> temp;
      const auto& value = var->value();
      if (value.isEmpty()) {
         throw std::runtime_error("CompositeTimeSeriesTransform::" + var->info().name + "is not a vector");
      } else if (value.isVector()) {;
         auto& valueVec = value.extract<std::vector<DynamicVar>>();
         for (auto& row : valueVec) {
            if (_dataPropertyName == "data_yearly") {
               for (auto i = 0; i < 12; i++) {
                  temp.push_back(row);
               }
            } else {
               temp.push_back(row);
            }
         }
         // If past the end of values given for timeseries, use last valid value
         // Brutal, but effective here
         if (curStep > (temp.size() - 1)) curStep = temp.size() - 1;
         result.insert<std::string>(var->info().name, temp[curStep]);
      }
   }
   _currentValue = result;
   return _currentValue;
}

}  // namespace agri
}  // namespace example
}  // namespace flint
}  // namespace moja
