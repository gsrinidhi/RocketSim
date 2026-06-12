#ifndef GUIDANCE_H
#define GUIDANCE_H

#include "phyVector.h"

int guid_PEG(phyVector v,phyVector s,double commanded_pitch, double thrustMag, double tgo, double dt, double mass, double g, phyVector *pred_s, phyVector *pred_vtr);

#endif // GUIDANCE_H