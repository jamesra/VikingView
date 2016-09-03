#include <vtkSTLWriter.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <QList>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>

#include <Application/ModelController.h>
#include <Data/Downloader.h>
#include <Data/Structure.h>

#include <Application/Preferences.h>

QList<long> ParseInputFile(QString filepath);

QList<long> FetchIDsFromInputString(QString str)
{
	//Parses an input string, which can be any of the following
	//1. Numbers
	//2. A filename
	//3. An OData query
	QList<long> ids;

	bool isNumber = false;
	long ID = str.toInt(&isNumber);
	if (isNumber)
	{
		ids.append(ID);
		return ids;
	}
	else
	{
		//Must be filename or OData query
		if (QFile::exists(str))
		{
			//A file
			ids.append(ParseInputFile(str));
		}
		else
		{
			//OData query
			QString endpoint = Preferences::Instance().active_endpoint();
			ids.append(Downloader::FetchStructureIDsFromODataQuery(Preferences::Instance().active_endpoint(), str));
		}
	} 

	return ids;
}

QList<long> ParseInputFile(QString filepath)
{
	QList<long> ids;
	QFile inputFile(filepath);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			ids.append(FetchIDsFromInputString(line));
		}
	}

	return ids;
}

QList<long> ParseInputArray(QString input)
{
	//Parses an input string, which can be any of the following
	//1. Numbers
	//2. A filename
	//3. An OData query

	QStringList pieces = input.split(" ");
	QList<long> ids;
	foreach(QString str, pieces) {
		QList<long> new_ids = FetchIDsFromInputString(str);
		ids.append(new_ids);
	}

	//Convert to set and back to remove duplicates
	return ids.toSet().toList();
}

QList<QSharedPointer<Structure> > LoadStructures(QList<long> IDs, QString end_point, ColorMapper cmap, ProgressReporter &report_progress)
{
	double num_reports = 4 + IDs.count();
	int report_index = 0;
	report_progress(report_index / num_reports, "Downloading Scale...");
	Downloader downloader;
	QSharedPointer<ScaleObject> scale = downloader.download_scale(end_point);

	QList<QSharedPointer<Structure> > listCells;

	DownloadObject download_object;

	downloader.download_cells(end_point, IDs, download_object, report_progress);

	QSharedPointer<StructureHash> structures = Structure::create_structures(download_object.structure_list,
			download_object.location_list,
			download_object.link_list,
			scale,
			cmap);

	QList<QSharedPointer<Structure>> output;
	foreach(long ID, IDs)
	{
		if (!structures->contains(ID))
		{
			std::cerr << "Could not download structure #" << ID;
			continue;
		}
		else
		{
			output.append(structures->value(ID));
		}
	}

	return output;
}

void GenerateMesh(QSharedPointer<Structure> structure)
{
	structure->get_mesh();
}

void GenerateMeshes(QList<QSharedPointer<Structure> > root_structures)
{
#ifndef QT_NO_CONCURRENT
	QList<QSharedPointer<Structure>> allStructures = GatherStructures(root_structures);
	QtConcurrent::blockingMap(allStructures, GenerateMesh);
#else
	
	foreach(QSharedPointer<Structure> root_structure, root_structures)
	{
		root_structure->get_mesh();

		GenerateMeshes(root_structure->structures.values());
	}
#endif
}

vtkSmartPointer< vtkActor >  CreateActorForStructure(QSharedPointer< Structure > s)
{
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyData> mesh = s->get_mesh();

	if (mesh == NULL)
		return NULL;

	mapper->SetInputData(mesh);
	actor->SetMapper(mapper);

	QColor color = s->get_color();

	actor->GetProperty()->SetDiffuseColor(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0);
	actor->GetProperty()->SetOpacity(color.alpha() / 255.0);
	actor->GetProperty()->SetSpecular(0.2);
	actor->GetProperty()->SetSpecularPower(15);
	actor->GetProperty()->BackfaceCullingOn();

	mapper->ScalarVisibilityOff();

	return actor;
}

void PopulateRenderer(vtkSmartPointer<vtkRenderer> renderer, 
										      QList < QSharedPointer<Structure> > structures)
{
	foreach(QSharedPointer<Structure> s, structures)
	{
		vtkSmartPointer<vtkActor> actor = CreateActorForStructure(s);
		if (actor != NULL)
			renderer->AddActor(actor);
	}

	return;
}

void export_stl(QList < QSharedPointer<Structure> > structures, QString filename)
{
	// Create an OBJ file exporter
	vtkSmartPointer<vtkSTLWriter> vtkExporter = vtkSmartPointer<vtkSTLWriter>::New();
	/*
	vtkActorCollection *actors = renderer->GetActors();
	for (vtkIdType i = 0; i < actors->GetNumberOfItems(); i++)
	{
		vtkActor* nextActor = actors->GetNextActor();
		 

	}
	*/

	for(int i = 0; i < structures.count(); i++)
	{
		QSharedPointer<Structure> s = structures[i];
		vtkSmartPointer<vtkPolyData> mesh = s->get_mesh();
		vtkExporter->SetInputData(0, mesh);
	}

	vtkExporter->SetFileName(filename.toStdString().c_str());
	vtkExporter->Write();
}

int ExportStructures(QList<long> IDs, QString end_point, QString export_dir, ColorMapper cmap, ProgressReporter &report_progress)
{
	QList<QSharedPointer<Structure> > structures = LoadStructures(IDs, end_point, cmap, report_progress);
	 
	GenerateMeshes(structures);

	export_stl(structures, export_dir + "data.stl");

	//vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	//PopulateRenderer(renderer, structures);


	return 0;
}
