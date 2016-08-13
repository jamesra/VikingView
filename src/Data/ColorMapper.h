#pragma once

#include <QHash>
#include <QColor>
#include <QString>
#include <QSharedPointer>

typedef QHash<long, QSharedPointer<QColor> > ColorMap;

ColorMap LoadIDColorMapFromFile(QString filepath); 

class ColorMapper
{
protected:
	ColorMap StructureColors;
	ColorMap StructureTypeColors;

public:
	ColorMapper::ColorMapper()
	{
		this->StructureColors = ColorMap();
		this->StructureTypeColors = ColorMap();
	}

	ColorMapper::ColorMapper(QString StructureTypesMapFilename, QString StructureMapFilename)
	{
		this->StructureColors = LoadIDColorMapFromFile(StructureMapFilename);
		this->StructureTypeColors = LoadIDColorMapFromFile(StructureTypesMapFilename);
	}

	QSharedPointer<QColor> ColorForStructure(long ID, long TypeID, bool & Found);
};