#pragma once
#include "Vector.h"

Vector4 ConvertColor(uint32_t color);

uint32_t ConvertColor(const Vector4& color);

Vector4 ConvertColorForImGui(uint32_t color);

uint32_t ConvertColorForImGui(const Vector4& color);
