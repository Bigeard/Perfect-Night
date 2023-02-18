#ifndef BOX_H
#define BOX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../physic/physic.h"

	typedef struct Box
	{
		int id;
		Physic p;
		Color color;
		bool displayQrCode;
	} Box;

	void InitBox(void);
	void UpdateBox(Box *box);
	void DrawBox(Box box);

#ifdef __cplusplus
}
#endif

#endif // BOX_H
