#ifndef BOX_H
#define BOX_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../physic/physic.h"

typedef struct Box {
    Physic p;
    Color color;
} Box;

void InitBox(void);
void UpdateBox(Box *box);
void DrawBox(Box box);

#ifdef __cplusplus
}
#endif

#endif // BOX_H
