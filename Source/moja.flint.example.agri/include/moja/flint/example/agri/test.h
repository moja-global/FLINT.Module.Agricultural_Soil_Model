#ifndef MOJA_FLINT_EXAMPLE_AGRI_AGRIMODULE_H_
#define MOJA_FLINT_EXAMPLE_AGRI_AGRIMODULE_H_

#include "moja/_core_exports.h"
#include "moja/flint/imodule.h"
#include "moja/flint/modulebase.h"
#include <moja/flint/itiming.h>
#include "moja/flint/example/agri/_modules.agri_exports.h"

namespace moja {
namespace flint {
namespace example {
namespace agri {

class AGRI_API AgriModule : public flint::ModuleBase
{
public:
	AgriModule() = default;
	~AgriModule() = default;

	void configure(const DynamicObject& config) override;
	void subscribe(NotificationCenter& notificationCenter) override;

	void onLocalDomainInit() override;
	void onTimingInit() override;
	void onTimingStep() override;

private:
	void Compute(double random);
	void InitializeForASimulation();
	void UpdateRateProperties();
	void SubmitMoves();

	/// <summary>
	///  Just summing up
	/// </summary>
	const flint::IPool* _Pool1; // count to store the sum
	const flint::IPool* _Pool2;
	const flint::IPool* _Pool3;
	const flint::IPool* _Pool4;

	const flint::IVariable* _fractions;
};

}
}
}
} // namespace moja::flint::example::sulay

#endif // MOJA_FLINT_EXAMPLE_AGRI_AGRIMODULE_H_
