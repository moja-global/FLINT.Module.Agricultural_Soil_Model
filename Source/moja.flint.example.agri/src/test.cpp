#include "moja/flint/example/agri/test.h"

#include "moja/flint/flintexceptions.h"
#include "moja/flint/ivariable.h"
#include "moja/flint/ioperation.h"

#include "moja/mathex.h"
#include "moja/notificationcenter.h"
#include "moja/signals.h"

#include <boost/format.hpp>

namespace moja {
namespace flint {
namespace example {
namespace agri {

void AgriModule::configure(const DynamicObject& config) { }

void AgriModule::subscribe(NotificationCenter& notificationCenter) {
	notificationCenter.subscribe(signals::LocalDomainInit	, &AgriModule::onLocalDomainInit	, *this);
	notificationCenter.subscribe(signals::TimingInit		, &AgriModule::onTimingInit		, *this);
	notificationCenter.subscribe(signals::TimingStep		, &AgriModule::onTimingStep		, *this);
}

/// <summary>
/// Soil moisture deficit (SMD)
/// </summary>
/// <param name="clayFrac"></param>
/// <param name="sampleDepth"></param>
/// <param name="bareToCvrdRatio"></param>
void AgriModule::Compute(double random) {
	
}

void AgriModule::InitializeForASimulation() {

}

void AgriModule::UpdateRateProperties() {
	
}

void AgriModule::SubmitMoves() {
	const auto fractions = _fractions->value();

	auto operation = _landUnitData->createProportionalOperation();
	operation
		->addTransfer(_Pool1, _Pool2, fractions["frac12"])
		->addTransfer(_Pool3, _Pool4, fractions["frac34"]);

	_landUnitData->submitOperation(operation);
}

void AgriModule::onLocalDomainInit() {
	// Pools
	_Pool1 = _landUnitData->getPool("Pool1");
    _Pool2 = _landUnitData->getPool("Pool2");
    _Pool3 = _landUnitData->getPool("Pool3");
    _Pool4 = _landUnitData->getPool("Pool4");

	// Variables
	_fractions = _landUnitData->getVariable("fractions");
}

void AgriModule::onTimingInit() {
	InitializeForASimulation();
}

void AgriModule::onTimingStep() {
	UpdateRateProperties();
	SubmitMoves();
}

}
}
}
} // namespace moja::flint::example::rothc




