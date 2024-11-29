#pragma once

#include "remote.h"

typedef unsigned long long POPUPSTREAMSESSION, *PPOPUPSTREAMSESSION;

BOOLEAN PopupStreamInitialize();
BOOLEAN PopupStreamSessionInitialize(const PPOPUPSTREAMSESSION session);
void PopupStreamSessionCleanup(const PPOPUPSTREAMSESSION session);
void PopupStreamCleanup();
