#pragma once

#include <QString>

struct AxisScale
{
public:
	AxisScale() {}
	AxisScale(QString Units, double Scale)
	{
		this->scale = Scale;
		this->units = Units;
	}

	QString units;
	double scale;
};

struct ScaleObject
{
public:
	ScaleObject() {}
	ScaleObject(AxisScale x, AxisScale y, AxisScale z)
	{
		this->X = x;
		this->Y = y;
		this->Z = z;
	}

	AxisScale X;
	AxisScale Y;
	AxisScale Z;
};