# Main python file for the BRep Exporter

#ASSEMBLY_PATH

clr.AddReferenceToFileAndPath(assemblyPath)
from PADT import *

def ExportBRepData(ext):
	BRepExport.Exporter.Export(ExtAPI)