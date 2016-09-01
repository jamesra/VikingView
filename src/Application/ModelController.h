#pragma once

#include <QString>
#include <QList>
#include <Data/ColorMapper.h>
#include <ProgressReporter.h>
#include <Data/Structure.h>


void GenerateMeshes(QList<QSharedPointer<Structure> > root_structures);
QList<QSharedPointer<Structure> > LoadStructures(QList<long> IDs, QString end_point, ColorMapper cmap, ProgressReporter &report_progress);
int ExportStructures(QList<long> IDs, QString end_point, QString export_dir, ColorMapper cmap, ProgressReporter &report_progress);

