#include "moja/flint/example/agri/landusemodule.h"

#include "moja/flint/flintexceptions.h"
#include "moja/flint/ivariable.h"
#include "moja/flint/ioperation.h"

#include "moja/mathex.h"
#include "moja/notificationcenter.h"
#include "moja/signals.h"
#include "moja/logging.h"

#include <boost/format.hpp>

namespace moja {
namespace flint {
namespace example {
namespace agri {

void LandUseModule::configure(const DynamicObject& config) { }

void LandUseModule::subscribe(NotificationCenter& notificationCenter) {
	notificationCenter.subscribe(signals::LocalDomainInit	, &LandUseModule::onLocalDomainInit	, *this);
	notificationCenter.subscribe(signals::TimingInit		, &LandUseModule::onTimingInit		, *this);
	notificationCenter.subscribe(signals::TimingStep		, &LandUseModule::onTimingStep		, *this);
}

void LandUseModule::InitializeForASimulation() {
    soilType = _soilType->value().extract<std::string>();
    climateZone = _climateZone->value().extract<std::string>();
    const auto stockRef = _stockRef->value().extract<const std::vector<DynamicObject>>();
    try {
        int temp = -1;
        for (auto i = 0; i < stockRef.size(); i++){
            if (stockRef[i]["Climate Zone"] == climateZone) {
                temp = i;
                break;
            }
        }
        if (temp == -1) {
            std::string str = "Climate zone: " + climateZone + " not present in SOC_REF FLINTagri.db";
            BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                                << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                                << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
        }
        SOC_REF = stockRef[temp][soilType].convert<double>();
    }
    catch(const std::exception&) {
        std::string str = "Soil type: " + soilType + " not present in SOC_REF FLINTagri.db";
        BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
    }
}

void LandUseModule::SubmitMoves() {
    const auto changeFactor = _changeFactor->value().extract<const std::vector<DynamicObject>>();
    const auto landType = _landType->value().extract<std::string>();
    const auto landUse = _landUse->value().extract<std::string>();
    const auto management = _management->value().extract<std::string>();
    const auto input = _input->value().extract<std::string>();

    std::string str;
    auto EF_1 = _landUnitData->getVariable("EF_1")->value().extract<DynamicObject>();
    double EF_1_value; 
    if (climate == "default") {
       EF_1_value = EF_1["default"];
    } else if (climate == "dry") {
       EF_1_value = EF_1["dry"];
    } else {
       EF_1_value = EF_1["wet"];
    }
    double area = 1;
    double F_LU = -1, F_MG = -1, F_I = -1, R = 10;
    if (landType == "F") {
        F_LU = 1; F_MG = 1; F_I = 1;
    }
    else {
        if (landType == "C") {
            str = "Cropland";
        }
        else if (landType == "G") {
            str = "Grassland";
        }
        else {
            std::string str = "Land type: " + landType + " not present in Soil_Stock_Change FLINTagri.db";
            BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                                << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                                << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
        }
        for (auto i = 0; i < changeFactor.size(); i++) {
            if (changeFactor[i]["Type"] == str
                && changeFactor[i]["Factor type"] == "F_LU"
                && (changeFactor[i]["Level"] == landUse || changeFactor[i]["Level"] == "All")   
                && (changeFactor[i]["Climate Zone"] == climateZone || changeFactor[i]["Climate Zone"] == "All")) {
                F_LU = changeFactor[i]["Value"];
            } else if (changeFactor[i]["Type"] == str
                && changeFactor[i]["Factor type"] == "F_MG"
                && (changeFactor[i]["Level"] == management || changeFactor[i]["Level"] == "All")
                && (changeFactor[i]["Climate Zone"] == climateZone || changeFactor[i]["Climate Zone"] == "All")) {
                F_MG = changeFactor[i]["Value"];
            } else if (changeFactor[i]["Type"] == str
                && changeFactor[i]["Factor type"] == "F_I"
                && (changeFactor[i]["Level"] == input || changeFactor[i]["Level"] == "All")
                && (changeFactor[i]["Climate Zone"] == climateZone || changeFactor[i]["Climate Zone"] == "All")) {
                F_I = changeFactor[i]["Value"];
            }
        }
        if (F_LU == -1 || F_MG == -1 || F_I == -1) {
            std::string str = "The entry: " + landType + " " + landUse + " " + management + " " + input + "is not present in Soil_Stock_Change FLINTagri.db";
            BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                                << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                                << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
        }
    }
    double SOC_REF_0 = SOC_REF * F_LU * F_MG * F_I * area;
    double delta_C =  (SOC_REF_0 - prevSOC) * 30 / (20 * 365);
    if (prevSOC != -1) {
        if (delta_C > 0) {
            auto operation = _landUnitData->createStockOperation();
            operation
                ->addTransfer(_soil, _atmosphere, delta_C * 1000 * EF_1_value / R);
            _landUnitData->submitOperation(operation);
        }
    }
    prevSOC = SOC_REF_0;
    prevLandType = landType;
}

void LandUseModule::onLocalDomainInit() {
	// Pools
	_soil = _landUnitData->getPool("soil");
	_atmosphere = _landUnitData->getPool("atmosphere");
	_initialValues = _landUnitData->getPool("initial_values");

	// Variables
	_landType = _landUnitData->getVariable("landtype");
	_landUse = _landUnitData->getVariable("landuse");
	_management = _landUnitData->getVariable("management");
	_input = _landUnitData->getVariable("input");
    _soilType = _landUnitData->getVariable("soil_type");
    _climateZone = _landUnitData->getVariable("ipcc_climate_zone");
    _stockRef = _landUnitData->getVariable("SOC_REF");
    _changeFactor = _landUnitData->getVariable("Soil_Stock_Change");
}

void LandUseModule::onTimingInit() {
    InitializeForASimulation();
    ClassifyClimate();
}

void LandUseModule::onTimingStep() {
	SubmitMoves();
}

void LandUseModule::ClassifyClimate() {
   const auto table = _landUnitData->getVariable("Wet_Dry_Climate")->value().extract<std::vector<DynamicObject>>();
   int temp = -1;
   for (auto i = 0; i < table.size(); i++) {
      if (table[i]["Climate Zone"] == climateZone) {
         temp = i;
         climate = table[i]["Wet/Dry"] ? "wet" : "dry";
         break;
      }
   }
   if (temp == -1) {
      std::string str = "Climate Zone: " + climateZone + " is not an IPCC Climate Zone";
      BOOST_THROW_EXCEPTION(flint::LocalDomainError()
                            << flint::Details(str) << flint::LibraryName("moja.flint.example.agri")
                            << flint::ModuleName(BOOST_CURRENT_FUNCTION) << flint::ErrorCode(1));
   }
}

}
}
}
} // namespace moja::flint::example::agri




