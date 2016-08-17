#include <Application/ModelController.h>
#include <Data/Downloader.h>
#include <Data/Structure.h>

QSharedPointer<Cell> LoadRootStructure(long ID, QString end_point, ScaleObject scale, ColorMapper cmap, ProgressReporter &report_progress = NoProgressReporter())
{
	Downloader downloader;
	DownloadObject download_object;

	if (!downloader.download_cell(end_point, ID, download_object, report_progress))
	{
		return QSharedPointer<Cell>();
	}

	QSharedPointer<StructureHash> structures = Structure::create_structures(download_object.structure_list,
		download_object.location_list,
		download_object.link_list,
		scale,
		cmap);
	
	QSharedPointer<Cell> cell = QSharedPointer<Cell>(new Cell());
	cell->id = ID;
	cell->structures = structures;

	return cell;
}

QList<QSharedPointer<Cell>> LoadStructures(QList<long> IDs, QString end_point, ColorMapper cmap, ProgressReporter &report_progress = NoProgressReporter())
{
	double num_reports = 4 + IDs.count();
	int report_index = 0;
	report_progress(report_index / num_reports, "Downloading Scale...");
	Downloader downloader;
	ScaleObject scale = downloader.download_scale(end_point);

	QList<QSharedPointer<Cell>> listCells;

	for (int i = 0; i < IDs.count(); i++)
	{
		long ID = IDs[i]; 
		report_progress(report_index / num_reports, QString("Downloading Annotations for %1...").arg(ID));
		QSharedPointer<Cell> structure = LoadRootStructure(ID, end_point, scale, cmap, report_progress);
		if (structure.isNull())
		{
			std::cerr << "Could not download structure #" << ID;
			continue;
		}
		else
		{
			listCells.append(structure);
		}
	}

	return listCells;
}

void GenerateMeshes(QList<QSharedPointer<Structure>> root_structures)
{
	
}

int ExportStructures(QList<long> IDs, QString end_point, QString export_dir, ColorMapper cmap, ProgressReporter &report_progress = NoProgressReporter())
{
	QList<QSharedPointer<Cell>> structures = LoadStructures(IDs, end_point, cmap, report_progress);
	 
	foreach(QSharedPointer<Cell> root_structure, structures)
	{
		foreach(QSharedPointer<Structure> child_structure, root_structure->structures->values())
		{
			child_structure->get_mesh();
		}
	}

	return 0;
}
