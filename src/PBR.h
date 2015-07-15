#ifndef CAM_N_PROJ_H_
#define CAM_N_PROJ_H_
#include "Application.h"
#include "Camera.h"

class PBR : public Application
{
	FlyCamera m_oCamera;
public:
	PBR();
	virtual ~PBR();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//CAM_N_PROJ_H_