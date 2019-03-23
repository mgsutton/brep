using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Ansys.ACT.Interfaces.Mechanical;
using Ansys.UI.Toolkit;
using Padt.Brep.Proto;

namespace PADT.BRepExport
{
  public static class Exporter
  {
    private class Implemenation
    {
      public IMechanicalExtAPI ExtAPI { get; set; }

      public void Write(string outputPath)
      {

      }
    }
    public static void Export(IMechanicalExtAPI ExtAPI)
    {
      string directory = "";
      string name = "";
      string selectedFile;
      int filterIndex;
      Window mainWin = (ExtAPI.UserInterface.MainWindow as Window);
      var result = FileDialog.ShowFileDialog(
          mainWin,
          true,
          directory,
          "BRep files (*.brep)|*.brep",
          0,
          out selectedFile,
          out filterIndex,
          "Export BRep File", name,
          true);
      if (result == DialogResult.OK)
      {
        // Write the data
        Implemenation writer = new Implemenation { ExtAPI = ExtAPI };
        writer.Write(selectedFile);
        Core.RecentFolderDataStore.StoreRecentFolder(
            "BRepExporter",
            "ExportFileLocation",
            Directory.GetParent(selectedFile).FullName);

      }

     
    }
  }
}

