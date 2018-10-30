#ifndef SPKUTILS_H
#define SPKUTILS_H

#include "spark/SPARK_Core.h"
#include "../math/Vector3.h"
#include "../../node-editor/common/Types.h"
#include "../../node-editor/common/Color.h"

//----------------------------------------------------------------------------------------------
// Conversion functions for spark
//----------------------------------------------------------------------------------------------

SPK::Vector3D ToSpkVector3D(const eFXYZ& v);
gplay::Vector3 ToGplayVector3(const SPK::Vector3D& v);
SPK::Color ToSpkColor(const eColor& c);

#endif // SPKUTILS_H
