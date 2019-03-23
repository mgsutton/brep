using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Ansys.ACT.Interfaces.Geometry;
using Ansys.ACT.Interfaces.Mechanical;
using Ansys.UI.Toolkit;
using Google.Protobuf;
using Padt.Brep.Proto;
using PADT.BRepExport.Core;

namespace PADT.BRepExport
{
  public static class Exporter
  {
    private class Implemenation
    {
      public IMechanicalExtAPI ExtAPI { get; set; }
      private MemoryStream buffer { get; set; }
      public void Write(string outputPath)
      {
        var gd = ExtAPI.DataModel.GeoData;
        using (buffer = new MemoryStream())
        {
          int assemblyId = 1;
          foreach (var assembly in gd.Assemblies)
          {
            WriteAssembly(assembly, assemblyId++);
          }
        }
      }

      private void WriteAssembly(IGeoAssembly assembly, int id)
      {
        var protoAssembly = new Assembly
        {
          Id = id,
          Source = ""
        };
        var partIds = (from p in assembly.Parts
                      select (long)p.Id).ToList();
        protoAssembly.Parts.AddRange(partIds);
        protoAssembly.WriteDelimitedTo(buffer);
        foreach (var part in assembly.Parts)
        {
          WritePart(part);
        }
      }

      private void WritePart(IGeoPart part)
      {
        var protoPart = new Part
        {
          Id = part.Id
        };
        var bodyIds = (from b in part.Bodies
                       select (long)b.Id).ToList();
        protoPart.Bodies.AddRange(bodyIds);
        protoPart.WriteDelimitedTo(buffer);
        foreach(var body in part.Bodies)
        {
          WriteBody(body as IGeoBody);
        }
      }

      private void WriteBody(IGeoBody body)
      {
        var protoBody = new Body
        {
          Id = body.Id
        };
        var faceIds = (from f in body.Faces
                       select (long)f.Id).ToList();
        foreach(var s in body.Shells)
        {
          var protoShell = new Shell();
          var shellFaceIds = (from f in s.Faces
                              select (long)f.Id).ToList();
          protoShell.Faces.AddRange(shellFaceIds);
          protoBody.Shells.Add(protoShell);
        }
        protoBody.WriteDelimitedTo(buffer);
        foreach(var f in body.Faces)
        {
          WriteFace(f as IGeoFace);
        }
        foreach(var e in body.Edges)
        {
          WriteEdge(e as IGeoEdge);
        }
        foreach(var v in body.Vertices)
        {
          WriteVertex(v as IGeoVertex);
        }
      }

      private void WriteFace(IGeoFace face)
      {
        

        var protoSurface = new Surface();
        foreach(var triple in face.Points.SplitToArray(3))
        {
          var protoPoint = new Vector3
          {
            X = triple[0],
            Y = triple[1],
            Z = triple[2]
          };
          var uv = face.ParamAtPoint(triple);
          var protoUV = new Vector2
          {
            U = uv[0],
            V = uv[1]
          };
          protoSurface.Points.Add(protoPoint);
          protoSurface.Parameters.Add(protoUV);
        }
        foreach (var triple in face.Normals.SplitToArray(3))
        {
          var protoNormal = new Vector3
          {
            X = triple[0],
            Y = triple[1],
            Z = triple[2]
          };
          protoSurface.Normals.Add(protoNormal);
        }
        // Get the indices into the proper form
        var indices = 
          Utilities.ConvertANSYSFacetListToDMesh(
            face.Points, 
            face.Indices, 
            face.Normals);
        foreach(var triple in indices.Chunk(3))
        {
          var facetIndices = triple.ToList();
          var protoFacet = new Facet
          {
            I = facetIndices[0],
            J = facetIndices[1],
            K = facetIndices[2]
          };
          protoSurface.Triangles.Add(protoFacet);
        }

        var protoFace = new Face
        {
          Id = face.Id,
          Surface = protoSurface
        };

        var edges = (from e in face.Edges
                     select (long)e.Id).ToList();
        protoFace.Edges.AddRange(edges);

        foreach(var l in face.Loops)
        {
          var protoLoop = new Loop();
          var loopEdges = (from e in l.Edges
                           select (long)e.Id).ToList();
          protoLoop.Edges.AddRange(loopEdges);
          protoFace.Loops.Add(protoLoop);
        }
        protoFace.WriteDelimitedTo(buffer);
      }


      private void WriteEdge(IGeoEdge edge)
      {

      }

      private void WriteVertex(IGeoVertex vertex)
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

