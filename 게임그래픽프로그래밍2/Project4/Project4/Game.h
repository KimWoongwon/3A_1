#pragma once
#include "CD3DApp.h"

class CGame : public CD3DApp
{
	virtual void OnInit() override;
	virtual void OnRender() override;
	virtual void OnUpdate() override;
	virtual void OnRelease() override;

public:
	CGame();
	~CGame();
};

