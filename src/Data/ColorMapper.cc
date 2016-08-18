#include <Data/ColorMapper.h>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QString>
#include <QRegExp>

#ifdef _WIN32

	#include <stdio.h>
	#include <fcntl.h>
	#include <io.h>
	#include <iostream>
	#include <fstream>
#else
#include<iostream>
#endif


	//#include <Data/AlphaShape.h>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

bool IsComment(QString line)
{
	return line.startsWith('%');
}

/*Load a tab delimited file containing lines with ID R G B A entries.  % characters indicate a comment*/
ColorMap LoadIDColorMapFromFile(QString filepath)
{
	QFileInfo check_file(filepath);

	ColorMap cmap = ColorMap();
	
	if (!(check_file.exists() && check_file.isFile()))
	{
		QString absPath = check_file.absoluteFilePath();
		std::cout << "LoadStructureColorsFromFile: File not found " << check_file.absoluteFilePath().toStdString();
		return cmap;
	}
	
	QRegExp rx("(^\\s*%.*$)|^\\s*(\\d+|\\d+\.\\d+|\.\\d+)\\s+(\\d+|\\d+\.\\d+|\.\\d+)\\s+(\\d+|\\d+\.\\d+|\.\\d+)\\s+(\\d+|\\d+\.\\d+|\.\\d+)\\s+(\\d+|\\d+\.\\d+|\.\\d+).*");

	QFile inputFile(filepath);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();

			bool matched = rx.exactMatch(line);  
			if (matched)
			{ 
				if (IsComment(rx.cap(1)))
					continue; 

                std::cout << rx.cap(0).toStdString() << " " << rx.cap(1).toStdString() << " " << rx.cap(2).toStdString() << " " << rx.cap(3).toStdString() << " " << rx.cap(4).toStdString() << std::endl;
				long ID = rx.cap(2).toLongLong();
				double R = rx.cap(3).toDouble();
				double G = rx.cap(4).toDouble();
				double B = rx.cap(5).toDouble();
				double A = rx.cap(6).toDouble();
				  
				cmap[ID] = QSharedPointer<QColor>(new QColor((int)(R * 255.0), (int)(G * 255.0), (int)(B * 255.0), (int)(A * 255.0)));;
			}
		}
	} 

	return cmap; 
}

QSharedPointer<QColor> ColorMapper::ColorForStructure(long ID, long TypeID, bool &Found)
{
	Found = true;
	if (this->StructureColors.contains(ID))
		return this->StructureColors[ID];

	if (this->StructureTypeColors.contains(TypeID))
		return this->StructureTypeColors[TypeID];

	Found = false;
	return QSharedPointer<QColor>(new QColor(128 + (qrand() % 128), 128 + (qrand() % 128), 128 + (qrand() % 128)));
}