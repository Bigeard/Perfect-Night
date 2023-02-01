#ifndef TOOL_H
#define TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../lib/raylib/src/raylib.h"

Color LightenColor(Color color, float percentage);
Color ReverseColor(Color color, float percentage);

#ifdef __cplusplus
}
#endif

#endif // TOOL_H
