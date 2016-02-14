#include "delta.h"

namespace sky {

void PlaneDelta::apply(Plane &plane) {
  if (tuningDelta) plane.tuning = *tuningDelta;

  // reset or update plane.vital
  if (!vitalDelta) {
    plane.vital.reset();
  } else {
    if (tuningDelta or !plane.vital)
      plane.vital.emplace(PlaneVital(plane.tuning, {0, 0}, 0));

    plane.vital = *vitalDelta;
  }
}

#define member(TYPE, PTR, RULE) \
  tg::MemberRule<PlaneDelta, TYPE>(RULE, &PlaneDelta::PTR)
const tg::Pack<PlaneDelta> planeDeltaPack =
    tg::ClassPack<PlaneDelta>(
        member(optional<PlaneTuning>, tuningDelta,
               tg::OptionalPack<PlaneTuning>(planeTuningPack)),
        member(optional<PlaneVital>, vitalDelta,
               tg::OptionalPack<PlaneVital>(planeVitalPack))
    );
#undef member

}
