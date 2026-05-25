#include "MissionPlan.h"

namespace gcs::mission {

void MissionPlan::renumberSequencesInPlace()
{
    for (int i = 0; i < items.size(); ++i) {
        items[i].seq = i;
    }
}

} // namespace gcs::mission
