#include <stdio.h>
#include <d2d1.h>
#include "header.h"

static ID2D1HwndRenderTarget* target;
static ID2D1SolidColorBrush* brush;

BOOLEAN initializeDirect2D(const HWND window) {
	ID2D1Factory* factory;
	HRESULT result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

	if(FAILED(result)) {
		printf("D2D1CreateFactory() failed\n");
		return FALSE;
	}

	RECT area;
	GetClientRect(window, &area);
	result = factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(area.right - area.left, area.bottom - area.top)), &target);

	if(FAILED(result)) {
		printf("ID2D1Factory::CreateHwndRenderTarget() failed\n");
		return FALSE;
	}

	result = target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &brush);

	if(FAILED(result)) {
		printf("ID2D1HwndRenderTarget::CreateSolidColorBrush() failed\n");
		return FALSE;
	}

	return TRUE;
}

void drawDirect2D() {
	target->BeginDraw();
	target->FillRectangle(D2D1::RectF(100.0f, 100.0f, 200.0f, 200.0f), brush);

	if(FAILED(target->EndDraw())) {
		printf("ID2D1HwndRenderTarget::EndDraw() failed\n");
	}
}
